#include "LogManager.h"
#include <QDateTime>
#include <QDebug>

namespace ParamControl {

LogManager::LogManager(QObject* parent)
    : QObject(parent)
    , m_maxEntries(1000)
{
}

LogManager::~LogManager() {
}

bool LogManager::initialize(const QString& logFilePath, int maxEntries) {
    m_logFilePath = logFilePath;
    m_maxEntries = maxEntries;
    
    // Загружаем существующие записи из файла при инициализации
    return readFromFile();
}

void LogManager::log(LogLevel level, const QString& category, const QString& message, 
                    const QString& value, LogStatus status) {
    QMutexLocker locker(&m_mutex);
    
    // Создаем запись лога
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.category = category;
    entry.message = message;
    entry.value = value;
    entry.status = status;
    
    // Добавляем запись в список
    m_entries.append(entry);
    
    // Если превышено максимальное количество записей, удаляем самую старую
    if (m_entries.size() > m_maxEntries) {
        m_entries.removeFirst();
    }
    
    // Записываем в файл
    writeToFile(entry);
    
    // Уведомляем о добавлении записи
    emit logEntryAdded(entry);
}

void LogManager::clearLog() {
    QMutexLocker locker(&m_mutex);
    
    // Очищаем список записей
    m_entries.clear();
    
    // Очищаем файл лога
    QFile file(m_logFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        // Записываем заголовок
        QTextStream out(&file);
        out << "# Журнал событий ParamControl\n";
        out << "# Формат: Дата Время | Уровень | Категория | Сообщение | Значение | Статус\n";
        out << "# Дата начала: " << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "\n\n";
        file.close();
    }
    
    // Уведомляем об очистке лога
    emit logCleared();
}

QVector<LogEntry> LogManager::getAllEntries() const {
    QMutexLocker locker(&m_mutex);
    return m_entries;
}

QVector<LogEntry> LogManager::getEntriesByCategory(const QString& category) const {
    QMutexLocker locker(&m_mutex);
    
    QVector<LogEntry> result;
    for (const auto& entry : m_entries) {
        if (entry.category == category) {
            result.append(entry);
        }
    }
    
    return result;
}

QVector<LogEntry> LogManager::getEntriesByLevel(LogLevel level) const {
    QMutexLocker locker(&m_mutex);
    
    QVector<LogEntry> result;
    for (const auto& entry : m_entries) {
        if (entry.level == level) {
            result.append(entry);
        }
    }
    
    return result;
}

QVector<LogEntry> LogManager::getEntriesByStatus(LogStatus status) const {
    QMutexLocker locker(&m_mutex);
    
    QVector<LogEntry> result;
    for (const auto& entry : m_entries) {
        if (entry.status == status) {
            result.append(entry);
        }
    }
    
    return result;
}

QString LogManager::getLogFilePath() const {
    return m_logFilePath;
}

bool LogManager::setLogFilePath(const QString& logFilePath) {
    if (m_logFilePath == logFilePath) {
        return true;
    }
    
    // Сохраняем текущие записи в новый файл
    QString oldFilePath = m_logFilePath;
    m_logFilePath = logFilePath;
    
    return saveLog();
}

int LogManager::getMaxEntries() const {
    return m_maxEntries;
}

void LogManager::setMaxEntries(int maxEntries) {
    if (maxEntries <= 0) {
        return;
    }
    
    m_maxEntries = maxEntries;
    
    QMutexLocker locker(&m_mutex);
    
    // Если текущее количество записей превышает новый максимум, удаляем лишние
    while (m_entries.size() > m_maxEntries) {
        m_entries.removeFirst();
    }
}

bool LogManager::saveLog(const QString& filePath) {
    QMutexLocker locker(&m_mutex);
    
    QString path = filePath.isEmpty() ? m_logFilePath : filePath;
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл для записи:" << path;
        return false;
    }
    
    QTextStream out(&file);
    
    // Записываем заголовок
    out << "# Журнал событий ParamControl\n";
    out << "# Формат: Дата Время | Уровень | Категория | Сообщение | Значение | Статус\n";
    out << "# Дата начала: " << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "\n\n";
    
    // Записываем все записи
    for (const auto& entry : m_entries) {
        out << formatLogEntry(entry) << "\n";
    }
    
    file.close();
    return true;
}

bool LogManager::writeToFile(const LogEntry& entry) {
    // Проверяем, существует ли файл
    QFile file(m_logFilePath);
    
    // Если файл не существует, создаем его с заголовком
    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Не удалось создать файл лога:" << m_logFilePath;
            return false;
        }
        
        QTextStream out(&file);
        out << "# Журнал событий ParamControl\n";
        out << "# Формат: Дата Время | Уровень | Категория | Сообщение | Значение | Статус\n";
        out << "# Дата начала: " << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "\n\n";
        
        file.close();
    }
    
    // Открываем файл для добавления записи
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл лога для записи:" << m_logFilePath;
        return false;
    }
    
    QTextStream out(&file);
    out << formatLogEntry(entry) << "\n";
    
    file.close();
    return true;
}

bool LogManager::readFromFile() {
    QFile file(m_logFilePath);
    
    // Если файл не существует, создаем его
    if (!file.exists()) {
        QFile newFile(m_logFilePath);
        if (newFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&newFile);
            out << "# Журнал событий ParamControl\n";
            out << "# Формат: Дата Время | Уровень | Категория | Сообщение | Значение | Статус\n";
            out << "# Дата начала: " << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "\n\n";
            newFile.close();
        }
        return true;  // Новый файл создан, но данных нет
    }
    
    // Открываем файл для чтения
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл лога для чтения:" << m_logFilePath;
        return false;
    }
    
    QTextStream in(&file);
    QVector<LogEntry> loadedEntries;
    
    // Пропускаем заголовок (строки, начинающиеся с #)
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.startsWith('#') && !line.isEmpty()) {
            LogEntry entry = parseLogLine(line);
            loadedEntries.append(entry);
            
            // Ограничиваем количество загружаемых записей
            if (loadedEntries.size() >= m_maxEntries) {
                break;
            }
        }
    }
    
    file.close();
    
    // Обновляем список записей
    QMutexLocker locker(&m_mutex);
    m_entries = loadedEntries;
    
    return true;
}

QString LogManager::formatLogEntry(const LogEntry& entry) const {
    QString levelStr = entry.level == LogLevel::Info ? "INFO" : "ERROR";
    QString statusStr = entry.status == LogStatus::Normal ? "Normal" : "Error";
    
    return QString("%1 | %2 | %3 | %4 | %5 | %6")
        .arg(entry.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(levelStr)
        .arg(entry.category)
        .arg(entry.message)
        .arg(entry.value)
        .arg(statusStr);
}

LogEntry LogManager::parseLogLine(const QString& line) const {
    LogEntry entry;
    
    // Разбиваем строку по разделителю |
    QStringList parts = line.split("|");
    
    if (parts.size() >= 6) {
        // Дата и время
        entry.timestamp = QDateTime::fromString(parts[0].trimmed(), "yyyy-MM-dd hh:mm:ss");
        
        // Уровень логирования
        QString levelStr = parts[1].trimmed();
        entry.level = (levelStr == "INFO") ? LogLevel::Info : LogLevel::Error;
        
        // Категория
        entry.category = parts[2].trimmed();
        
        // Сообщение
        entry.message = parts[3].trimmed();
        
        // Значение
        entry.value = parts[4].trimmed();
        
        // Статус
        QString statusStr = parts[5].trimmed();
        entry.status = (statusStr == "Normal") ? LogStatus::Normal : LogStatus::Error;
    } else {
        // Если формат не соответствует ожидаемому, устанавливаем текущее время и пустые поля
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = LogLevel::Error;
        entry.category = "Лог";
        entry.message = "Ошибка формата: " + line;
        entry.value = "";
        entry.status = LogStatus::Error;
    }
    
    return entry;
}

} // namespace ParamControl