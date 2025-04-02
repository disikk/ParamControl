#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QMediaPlayer>
#include <QTimer>
#include <memory>
#include <mutex>

namespace ParamControl {

/**
 * @brief Типы оповещений
 */
enum class AlertType {
    NoTmi,          ///< Отсутствие телеметрии
    ParameterChange, ///< Изменение параметра
    ParameterLimit, ///< Выход параметра за допустимые пределы
    Default         ///< Оповещение по умолчанию
};

/**
 * @brief Класс для управления звуковыми оповещениями
 *
 * Этот класс отвечает за воспроизведение звуковых оповещений в зависимости от типа события.
 * Он поддерживает несколько типов оповещений и позволяет настраивать звуковые файлы для каждого типа.
 */
class AlertManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param parent Родительский объект
     */
    explicit AlertManager(QObject* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~AlertManager();
    
    /**
     * @brief Воспроизведение оповещения
     * @param type Тип оповещения
     * @param looping Флаг циклического воспроизведения
     */
    void playAlert(AlertType type, bool looping = false);
    
    /**
     * @brief Остановка воспроизведения оповещения
     * @param type Тип оповещения (AlertType::Default - все оповещения)
     */
    void stopAlert(AlertType type = AlertType::Default);
    
    /**
     * @brief Остановка всех оповещений
     */
    void stopAllAlerts();
    
    /**
     * @brief Установка звукового файла для оповещения
     * @param type Тип оповещения
     * @param filePath Путь к звуковому файлу
     */
    void setAlertSound(AlertType type, const QString& filePath);
    
    /**
     * @brief Получение пути к звуковому файлу оповещения
     * @param type Тип оповещения
     * @return Путь к звуковому файлу
     */
    QString getAlertSound(AlertType type) const;
    
    /**
     * @brief Включение/выключение оповещения
     * @param type Тип оповещения
     * @param enabled Флаг включения
     */
    void setAlertEnabled(AlertType type, bool enabled);
    
    /**
     * @brief Проверка, включено ли оповещение
     * @param type Тип оповещения
     * @return true, если оповещение включено
     */
    bool isAlertEnabled(AlertType type) const;
    
    /**
     * @brief Воспроизведение оповещения для параметра
     * @param paramName Имя параметра
     * @param soundFile Путь к звуковому файлу
     * @param looping Флаг циклического воспроизведения
     */
    void playParameterAlert(const QString& paramName, const QString& soundFile, bool looping = false);
    
    /**
     * @brief Остановка воспроизведения оповещения для параметра
     * @param paramName Имя параметра
     */
    void stopParameterAlert(const QString& paramName);

signals:
    /**
     * @brief Сигнал начала воспроизведения оповещения
     * @param type Тип оповещения
     */
    void alertStarted(AlertType type);
    
    /**
     * @brief Сигнал окончания воспроизведения оповещения
     * @param type Тип оповещения
     */
    void alertStopped(AlertType type);
    
    /**
     * @brief Сигнал начала воспроизведения оповещения для параметра
     * @param paramName Имя параметра
     */
    void parameterAlertStarted(const QString& paramName);
    
    /**
     * @brief Сигнал окончания воспроизведения оповещения для параметра
     * @param paramName Имя параметра
     */
    void parameterAlertStopped(const QString& paramName);

private slots:
    /**
     * @brief Обработчик окончания воспроизведения
     * @param state Состояние плеера
     */
    void onStateChanged(QMediaPlayer::State state);

private:
    struct AlertInfo {
        QString soundFile;                    ///< Путь к звуковому файлу
        bool enabled;                         ///< Флаг включения
        std::unique_ptr<QMediaPlayer> player; ///< Плеер для воспроизведения
        bool looping;                         ///< Флаг циклического воспроизведения
    };
    
    QMap<AlertType, AlertInfo> m_alerts;      ///< Карта оповещений
    QMap<QString, AlertInfo> m_parameterAlerts; ///< Карта оповещений для параметров
    
    mutable std::mutex m_mutex;               ///< Мьютекс для защиты доступа к данным
    
    /**
     * @brief Инициализация оповещения
     * @param type Тип оповещения
     * @param soundFile Путь к звуковому файлу
     */
    void initAlert(AlertType type, const QString& soundFile);
    
    /**
     * @brief Инициализация оповещения для параметра
     * @param paramName Имя параметра
     * @param soundFile Путь к звуковому файлу
     */
    void initParameterAlert(const QString& paramName, const QString& soundFile);
};

} // namespace ParamControl
