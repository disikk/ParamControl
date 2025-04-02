#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <memory>

namespace ParamControl {

/**
 * @brief Уровни логирования
 */
enum class LogLevel {
    Info,       ///< Информационное сообщение
    Error       ///< Ошибка
};

/**
 * @brief Статус логируемого события
 */
enum class LogStatus {
    Normal,     ///< Нормальное состояние
    Error       ///< Ошибка
};

/**
 * @brief Структура для хранения записи журнала
 */
struct LogEntry {
    QDateTime timestamp;     ///< Время события
    LogLevel level;          ///< Уровень логирования
    QString category;        ///< Категория события
    QString message;         ///< Сообщение
    QString value;           ///< Значение (для параметров)
    LogStatus status;        ///< Статус события
};

/**
 * @brief Класс для управления журналом событий
 * 
 * Этот класс отвечает за журналирование событий и их сохранение в файл.
 * Он поддерживает фильтрацию по категориям и уровням логирования.
 */
class LogManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param parent Родительский объект
     */
    explicit LogManager(QObject* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~LogManager();
    
    /**
     * @brief Инициализация менеджера журнала
     * @param logFilePath Путь к файлу журнала
     * @param maxEntries Максимальное количество записей в памяти
     * @return true, если инициализация успешна
     */
    bool initialize(const QString& logFilePath, int maxEntries = 1000);
    
    /**
     * @brief Добавление записи в журнал
     * @param level Уровень логирования
     * @param category Категория события
     * @param message Сообщение
     * @param value Значение (для параметров)
     * @param status Статус события
     */
    void log(LogLevel level, const QString& category, const QString& message, 
             const QString& value = QString(), LogStatus status = LogStatus::Normal);
    
    /**
     * @brief Очистка журнала
     */
    void clearLog();
    
    /**
     * @brief Получение всех записей журнала
     * @return Список записей журнала
     */
    QVector<LogEntry> getAllEntries() const;
    
    /**
     * @brief Получение записей журнала, отфильтрованных по категории
     * @param category Категория события
     * @return Список записей журнала
     */
    QVector<LogEntry> getEntriesByCategory(const QString& category) const;
    
    /**
     * @brief Получение записей журнала, отфильтрованных по уровню логирования
     * @param level Уровень логирования
     * @return Список записей журнала
     */
    QVector<LogEntry> getEntriesByLevel(LogLevel level) const;
    
    /**
     * @brief Получение записей журнала, отфильтрованных по статусу
     * @param status Статус события
     * @return Список записей журнала
     */
    QVector<LogEntry> getEntriesByStatus(LogStatus status) const;
    
    /**
     * @brief Получение пути к файлу журнала
     * @return Путь к файлу журнала
     */
    QString getLogFilePath() const;
    
    /**
     * @brief Установка пути к файлу журнала
     * @param logFilePath Путь к файлу журнала
     * @return true, если установка успешна
     */
    bool setLogFilePath(const QString& logFilePath);
    
    /**
     * @brief Получение максимального количества записей в памяти
     * @return Максимальное количество записей в памяти
     */
    int getMaxEntries() const;
    
    /**
     * @brief Установка максимального количества записей в памяти
     * @param maxEntries Максимальное количество записей в памяти
     */
    void setMaxEntries(int maxEntries);
    
    /**
     * @brief Сохранение журнала в файл
     * @param filePath Путь к файлу (пусто - использовать текущий файл)
     * @return true, если сохранение успешно
     */
    bool saveLog(const QString& filePath = QString());

signals:
    /**
     * @brief Сигнал добавления записи в журнал
     * @param entry Запись журнала
     */
    void logEntryAdded(const LogEntry& entry);
    
    /**
     * @brief Сигнал очистки журнала
     */
    void logCleared();

private:
    QString m_logFilePath;                    ///< Путь к файлу журнала
    int m_maxEntries;                         ///< Максимальное количество записей в памяти
    QVector<LogEntry> m_entries;              ///< Список записей журнала в памяти
    
    mutable QMutex m_mutex;                   ///< Мьютекс для защиты доступа к данным
    
    /**
     * @brief Запись в файл журнала
     * @param entry Запись журнала
     * @return true, если запись успешна
     */
    bool writeToFile(const LogEntry& entry);
    
    /**
     * @brief Чтение журнала из файла
     * @return true, если чтение успешно
     */
    bool readFromFile();
    
    /**
     * @brief Форматирование записи журнала для записи в файл
     * @param entry Запись журнала
     * @return Форматированная строка
     */
    QString formatLogEntry(const LogEntry& entry) const;
    
    /**
     * @brief Парсинг строки журнала
     * @param line Строка журнала
     * @return Запись журнала
     */
    LogEntry parseLogLine(const QString& line) const;
};

} // namespace ParamControl
