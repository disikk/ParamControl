#include "AlertManager.h"
#include <QDebug>
#include <QUrl>
#include <QFileInfo>

namespace ParamControl {

AlertManager::AlertManager(QObject* parent)
    : QObject(parent)
{
    // Инициализация оповещений по умолчанию
    initAlert(AlertType::NoTmi, "./sounds/notmi.wav");
    initAlert(AlertType::ParameterChange, "./sounds/beep.wav");
    initAlert(AlertType::ParameterLimit, "./sounds/alert.wav");
    initAlert(AlertType::Default, "./sounds/warning.wav");
}

AlertManager::~AlertManager() {
    // Останавливаем все оповещения перед уничтожением
    stopAllAlerts();
}

void AlertManager::playAlert(AlertType type, bool looping) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, существует ли такое оповещение
    if (!m_alerts.contains(type)) {
        qWarning() << "Alert type not found:" << static_cast<int>(type);
        return;
    }
    
    // Проверяем, включено ли оповещение
    if (!m_alerts[type].enabled) {
        qDebug() << "Alert disabled:" << static_cast<int>(type);
        return;
    }
    
    // Проверяем, существует ли звуковой файл
    QFileInfo fileInfo(m_alerts[type].soundFile);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qWarning() << "Alert sound file not found:" << m_alerts[type].soundFile;
        return;
    }
    
    // Устанавливаем флаг циклического воспроизведения
    m_alerts[type].looping = looping;
    
    // Останавливаем текущее воспроизведение
    if (m_alerts[type].player->state() == QMediaPlayer::PlayingState) {
        m_alerts[type].player->stop();
    }
    
    // Устанавливаем источник
    m_alerts[type].player->setMedia(QUrl::fromLocalFile(m_alerts[type].soundFile));
    
    // Запускаем воспроизведение
    m_alerts[type].player->play();
    
    // Эмитируем сигнал начала воспроизведения
    emit alertStarted(type);
}

void AlertManager::stopAlert(AlertType type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (type == AlertType::Default) {
        // Останавливаем все оповещения
        for (auto it = m_alerts.begin(); it != m_alerts.end(); ++it) {
            if (it.value().player->state() == QMediaPlayer::PlayingState) {
                it.value().player->stop();
                
                // Эмитируем сигнал окончания воспроизведения
                emit alertStopped(it.key());
            }
        }
    } else {
        // Останавливаем конкретное оповещение
        if (m_alerts.contains(type)) {
            if (m_alerts[type].player->state() == QMediaPlayer::PlayingState) {
                m_alerts[type].player->stop();
                
                // Эмитируем сигнал окончания воспроизведения
                emit alertStopped(type);
            }
        }
    }
}

void AlertManager::stopAllAlerts() {
    // Останавливаем все системные оповещения
    stopAlert(AlertType::Default);
    
    // Останавливаем все оповещения для параметров
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto it = m_parameterAlerts.begin(); it != m_parameterAlerts.end(); ++it) {
        if (it.value().player->state() == QMediaPlayer::PlayingState) {
            it.value().player->stop();
            
            // Эмитируем сигнал окончания воспроизведения
            emit parameterAlertStopped(it.key());
        }
    }
}

void AlertManager::setAlertSound(AlertType type, const QString& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, существует ли такое оповещение
    if (!m_alerts.contains(type)) {
        // Инициализируем новое оповещение
        initAlert(type, filePath);
    } else {
        // Обновляем звуковой файл
        m_alerts[type].soundFile = filePath;
    }
}

QString AlertManager::getAlertSound(AlertType type) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, существует ли такое оповещение
    if (!m_alerts.contains(type)) {
        qWarning() << "Alert type not found:" << static_cast<int>(type);
        return QString();
    }
    
    return m_alerts[type].soundFile;
}

void AlertManager::setAlertEnabled(AlertType type, bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, существует ли такое оповещение
    if (!m_alerts.contains(type)) {
        qWarning() << "Alert type not found:" << static_cast<int>(type);
        return;
    }
    
    // Устанавливаем флаг включения
    m_alerts[type].enabled = enabled;
    
    // Если оповещение выключено, останавливаем его
    if (!enabled) {
        stopAlert(type);
    }
}

bool AlertManager::isAlertEnabled(AlertType type) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, существует ли такое оповещение
    if (!m_alerts.contains(type)) {
        qWarning() << "Alert type not found:" << static_cast<int>(type);
        return false;
    }
    
    return m_alerts[type].enabled;
}

void AlertManager::playParameterAlert(const QString& paramName, const QString& soundFile, bool looping) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, существует ли оповещение для параметра
    if (!m_parameterAlerts.contains(paramName)) {
        // Инициализируем новое оповещение
        initParameterAlert(paramName, soundFile);
    } else {
        // Обновляем звуковой файл
        m_parameterAlerts[paramName].soundFile = soundFile;
    }
    
    // Проверяем, включено ли оповещение
    if (!m_parameterAlerts[paramName].enabled) {
        qDebug() << "Parameter alert disabled:" << paramName;
        return;
    }
    
    // Проверяем, существует ли звуковой файл
    QFileInfo fileInfo(m_parameterAlerts[paramName].soundFile);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qWarning() << "Parameter alert sound file not found:" << m_parameterAlerts[paramName].soundFile;
        return;
    }
    
    // Устанавливаем флаг циклического воспроизведения
    m_parameterAlerts[paramName].looping = looping;
    
    // Останавливаем текущее воспроизведение
    if (m_parameterAlerts[paramName].player->state() == QMediaPlayer::PlayingState) {
        m_parameterAlerts[paramName].player->stop();
    }
    
    // Устанавливаем источник
    m_parameterAlerts[paramName].player->setMedia(QUrl::fromLocalFile(m_parameterAlerts[paramName].soundFile));
    
    // Запускаем воспроизведение
    m_parameterAlerts[paramName].player->play();
    
    // Эмитируем сигнал начала воспроизведения
    emit parameterAlertStarted(paramName);
}

void AlertManager::stopParameterAlert(const QString& paramName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, существует ли оповещение для параметра
    if (!m_parameterAlerts.contains(paramName)) {
        qWarning() << "Parameter alert not found:" << paramName;
        return;
    }
    
    // Останавливаем воспроизведение
    if (m_parameterAlerts[paramName].player->state() == QMediaPlayer::PlayingState) {
        m_parameterAlerts[paramName].player->stop();
        
        // Эмитируем сигнал окончания воспроизведения
        emit parameterAlertStopped(paramName);
    }
}

void AlertManager::onStateChanged(QMediaPlayer::State state) {
    // Получаем отправителя сигнала
    QMediaPlayer* player = qobject_cast<QMediaPlayer*>(sender());
    if (!player) {
        return;
    }
    
    // Находим оповещение, которому принадлежит плеер
    AlertType alertType = AlertType::Default;
    QString paramName;
    bool found = false;
    bool isParameterAlert = false;
    bool looping = false;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Проверяем системные оповещения
        for (auto it = m_alerts.begin(); it != m_alerts.end(); ++it) {
            if (it.value().player.get() == player) {
                alertType = it.key();
                looping = it.value().looping;
                found = true;
                break;
            }
        }
        
        // Если не нашли в системных, проверяем оповещения для параметров
        if (!found) {
            for (auto it = m_parameterAlerts.begin(); it != m_parameterAlerts.end(); ++it) {
                if (it.value().player.get() == player) {
                    paramName = it.key();
                    looping = it.value().looping;
                    found = true;
                    isParameterAlert = true;
                    break;
                }
            }
        }
    }
    
    // Если нашли оповещение
    if (found) {
        // Если плеер остановился и нужно циклическое воспроизведение
        if (state == QMediaPlayer::StoppedState && looping) {
            // Перезапускаем воспроизведение
            player->play();
        } else if (state == QMediaPlayer::StoppedState) {
            // Если плеер остановился и не нужно циклическое воспроизведение
            
            // Эмитируем сигнал окончания воспроизведения
            if (isParameterAlert) {
                emit parameterAlertStopped(paramName);
            } else {
                emit alertStopped(alertType);
            }
        }
    }
}

void AlertManager::initAlert(AlertType type, const QString& soundFile) {
    // Создаем новое оповещение
    AlertInfo alertInfo;
    alertInfo.soundFile = soundFile;
    alertInfo.enabled = true;
    alertInfo.looping = false;
    alertInfo.player = std::make_unique<QMediaPlayer>();
    
    // Подключаем сигнал изменения состояния плеера
    connect(alertInfo.player.get(), &QMediaPlayer::stateChanged,
            this, &AlertManager::onStateChanged);
    
    // Добавляем оповещение в карту
    m_alerts[type] = std::move(alertInfo);
}

void AlertManager::initParameterAlert(const QString& paramName, const QString& soundFile) {
    // Создаем новое оповещение
    AlertInfo alertInfo;
    alertInfo.soundFile = soundFile;
    alertInfo.enabled = true;
    alertInfo.looping = false;
    alertInfo.player = std::make_unique<QMediaPlayer>();
    
    // Подключаем сигнал изменения состояния плеера
    connect(alertInfo.player.get(), &QMediaPlayer::stateChanged,
            this, &AlertManager::onStateChanged);
    
    // Добавляем оповещение в карту
    m_parameterAlerts[paramName] = std::move(alertInfo);
}

} // namespace ParamControl
