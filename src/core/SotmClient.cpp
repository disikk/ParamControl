#include "SotmClient.h"

#include <QHostAddress>
#include <QDataStream>
#include <QElapsedTimer>
#include <QDebug>

// Константы протокола СОТМ
constexpr quint8 DIRECTORY_NUMBER = 158;
constexpr quint16 INFORMATION_TYPE = 1888;
constexpr int HEADER_LENGTH = 25;
constexpr int DEFAULT_TIMEOUT_MS = 5000;
constexpr int DEFAULT_CONNECT_TIMEOUT_MS = 10000;
constexpr int RECONNECT_DELAY_MS = 5000;

namespace ParamControl {

SotmClient::SotmClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket())
    , m_connectionTimeoutTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
{
    // Настройка таймера таймаута подключения
    m_connectionTimeoutTimer->setSingleShot(true);
    m_settings.responseTimeoutMs = DEFAULT_TIMEOUT_MS;
    
    // Настройка таймера переподключения
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->setInterval(RECONNECT_DELAY_MS);
    
    // Подключение сигналов сокета
    connect(m_socket.get(), &QTcpSocket::stateChanged,
            this, &SotmClient::onSocketStateChanged);
    connect(m_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &SotmClient::onSocketError);
    connect(m_connectionTimeoutTimer, &QTimer::timeout,
            this, &SotmClient::onConnectionTimeout);
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &SotmClient::onReconnectTimerTimeout);
}

SotmClient::~SotmClient() {
    disconnect();
}

bool SotmClient::connect(const SotmSettings& settings) {
    // Если уже подключены, сначала отключаемся
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        disconnect();
    }
    
    // Сохраняем настройки
    m_settings = settings;
    
    // Запускаем таймер таймаута
    m_connectionTimeoutTimer->start(DEFAULT_CONNECT_TIMEOUT_MS);
    
    // Останавливаем таймер переподключения, если он активен
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
    }
    
    // Пытаемся подключиться
    m_socket->connectToHost(QHostAddress(settings.ipAddress), settings.port);
    
    // Ждем подключения
    if (!m_socket->waitForConnected(DEFAULT_CONNECT_TIMEOUT_MS)) {
        QString errorMsg = QString("Не удалось подключиться к СОТМ: %1").arg(m_socket->errorString());
        emit errorOccurred(errorMsg);
        
        // Запускаем таймер переподключения
        m_reconnectTimer->start();
        
        return false;
    }
    
    // Останавливаем таймер таймаута
    m_connectionTimeoutTimer->stop();
    
    emit connectionStatusChanged(true);
    return true;
}

void SotmClient::disconnect() {
    // Останавливаем таймер переподключения
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
    }
    
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        
        // Если сокет не закрылся сразу, ждем закрытия
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
        
        emit connectionStatusChanged(false);
    }
}

bool SotmClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

std::optional<QByteArray> SotmClient::sendRequest(const QByteArray& requestData) {
    // Проверяем, подключены ли мы
    if (!isConnected()) {
        emit errorOccurred("Нет подключения к СОТМ");
        
        // Пытаемся восстановить соединение
        m_reconnectTimer->start();
        
        return std::nullopt;
    }
    
    // Формируем заголовок и полный пакет
    QByteArray headerPacket = createHeaderPacket(requestData.size());
    QByteArray fullPacket = headerPacket + requestData;
    
    // Очищаем буфер перед отправкой
    m_socket->readAll();
    
    // Отправляем запрос
    qint64 bytesWritten = m_socket->write(fullPacket);
    if (bytesWritten != fullPacket.size()) {
        emit errorOccurred(QString("Ошибка отправки данных: отправлено %1 из %2 байт")
                         .arg(bytesWritten)
                         .arg(fullPacket.size()));
        return std::nullopt;
    }
    
    if (!m_socket->waitForBytesWritten(m_settings.responseTimeoutMs)) {
        emit errorOccurred("Таймаут отправки данных");
        
        // Проверяем состояние соединения
        checkConnection();
        
        return std::nullopt;
    }
    
    // Ждем ответа
    if (!waitForReadyRead(m_settings.responseTimeoutMs)) {
        emit errorOccurred("Таймаут ожидания ответа");
        
        // Проверяем состояние соединения
        checkConnection();
        
        return std::nullopt;
    }
    
    // Читаем заголовок ответа
    QByteArray headerBuffer;
    while (headerBuffer.size() < HEADER_LENGTH) {
        if (!m_socket->bytesAvailable() && !waitForReadyRead(m_settings.responseTimeoutMs)) {
            emit errorOccurred("Таймаут чтения заголовка ответа");
            
            // Проверяем состояние соединения
            checkConnection();
            
            return std::nullopt;
        }
        
        QByteArray chunk = m_socket->read(HEADER_LENGTH - headerBuffer.size());
        if (chunk.isEmpty()) {
            emit errorOccurred("Ошибка чтения заголовка ответа");
            
            // Проверяем состояние соединения
            checkConnection();
            
            return std::nullopt;
        }
        
        headerBuffer.append(chunk);
    }
    
    // Проверяем директиву и код квитанции
    if (headerBuffer.at(0) != DIRECTORY_NUMBER || headerBuffer.at(24) != 2) {
        emit errorOccurred(QString("Неверный заголовок ответа: директива %1, код квитанции %2")
                         .arg(static_cast<int>(headerBuffer.at(0)))
                         .arg(static_cast<int>(headerBuffer.at(24))));
        return std::nullopt;
    }
    
    // Получаем длину прикладного пакета
    quint16 appPacketLength = 0;
    QDataStream ds(headerBuffer.mid(22, 2));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> appPacketLength;
    
    // Читаем прикладной пакет
    QByteArray appPacket;
    while (appPacket.size() < appPacketLength) {
        if (!m_socket->bytesAvailable() && !waitForReadyRead(m_settings.responseTimeoutMs)) {
            emit errorOccurred("Таймаут чтения данных ответа");
            
            // Проверяем состояние соединения
            checkConnection();
            
            return std::nullopt;
        }
        
        QByteArray chunk = m_socket->read(appPacketLength - appPacket.size());
        if (chunk.isEmpty()) {
            emit errorOccurred("Ошибка чтения данных ответа");
            
            // Проверяем состояние соединения
            checkConnection();
            
            return std::nullopt;
        }
        
        appPacket.append(chunk);
    }
    
    return appPacket;
}

const SotmSettings& SotmClient::getSettings() const {
    return m_settings;
}

void SotmClient::setSettings(const SotmSettings& settings) {
    m_settings = settings;
}

void SotmClient::onSocketStateChanged(QAbstractSocket::SocketState state) {
    if (state == QAbstractSocket::ConnectedState) {
        emit connectionStatusChanged(true);
        qDebug() << "СОТМ: Соединение установлено";
    } else if (state == QAbstractSocket::UnconnectedState) {
        emit connectionStatusChanged(false);
        qDebug() << "СОТМ: Соединение закрыто";
        
        // Запускаем автоматическое переподключение, если таймер не активен
        if (!m_reconnectTimer->isActive()) {
            m_reconnectTimer->start();
        }
    }
}

void SotmClient::onSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    QString errorMsg = QString("Ошибка сокета: %1").arg(m_socket->errorString());
    emit errorOccurred(errorMsg);
    emit connectionStatusChanged(false);
    
    // Запускаем автоматическое переподключение
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
}

void SotmClient::onConnectionTimeout() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->abort();
        emit errorOccurred("Таймаут подключения к СОТМ");
        emit connectionStatusChanged(false);
        
        // Запускаем автоматическое переподключение
        if (!m_reconnectTimer->isActive()) {
            m_reconnectTimer->start();
        }
    }
}

void SotmClient::onReconnectTimerTimeout() {
    // Пытаемся переподключиться
    qDebug() << "СОТМ: Попытка переподключения...";
    connect(m_settings);
}

QByteArray SotmClient::createHeaderPacket(quint16 appPacketLength) const {
    QByteArray header(HEADER_LENGTH, 0);
    
    // Директива
    header[0] = DIRECTORY_NUMBER;
    
    // Номер КА (2 байта, little-endian)
    QDataStream dsKa(&header[4], QIODevice::WriteOnly);
    dsKa.setByteOrder(QDataStream::LittleEndian);
    dsKa << m_settings.kaNumber;
    
    // Вид информации (2 байта, little-endian)
    QDataStream dsInfo(&header[8], QIODevice::WriteOnly);
    dsInfo.setByteOrder(QDataStream::LittleEndian);
    dsInfo << INFORMATION_TYPE;
    
    // Длина прикладного пакета (2 байта, little-endian)
    QDataStream dsLength(&header[22], QIODevice::WriteOnly);
    dsLength.setByteOrder(QDataStream::LittleEndian);
    dsLength << appPacketLength;
    
    return header;
}

bool SotmClient::waitForReadyRead(int timeout) {
    QElapsedTimer timer;
    timer.start();
    
    while (!m_socket->bytesAvailable()) {
        if (timer.elapsed() > timeout) {
            return false;
        }
        
        // Если отсоединились во время ожидания
        if (m_socket->state() != QAbstractSocket::ConnectedState) {
            return false;
        }
        
        // Ждем данные с периодической проверкой
        m_socket->waitForReadyRead(100);
    }
    
    return true;
}

void SotmClient::checkConnection() {
    // Проверяем состояние сокета
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit connectionStatusChanged(false);
        
        // Запускаем автоматическое переподключение
        if (!m_reconnectTimer->isActive()) {
            m_reconnectTimer->start();
        }
    } else {
        // Проверка соединения с помощью Poll
        bool part1 = m_socket->waitForReadyRead(0);
        bool part2 = (m_socket->bytesAvailable() == 0);
        
        if (!part1 && part2) {
            // Соединение разорвано
            m_socket->abort();
            emit connectionStatusChanged(false);
            emit errorOccurred("Соединение с СОТМ разорвано");
            
            // Запускаем автоматическое переподключение
            if (!m_reconnectTimer->isActive()) {
                m_reconnectTimer->start();
            }
        }
    }
}

} // namespace ParamControl