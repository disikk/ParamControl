#pragma once

#include <QObject>
#include <QTimer>
#include <atomic>
#include <memory>

#include "SotmClient.h"
#include "ParameterModel.h"
#include "XmlParser.h"
#include "AlertManager.h"
#include "LogManager.h"
#include "TmiAnalyzer.h"

namespace ParamControl {

class MonitoringService : public QObject {
    Q_OBJECT

public:
    explicit MonitoringService(
        std::shared_ptr<SotmClient> sotmClient,
        std::shared_ptr<ParameterModel> parameterModel,
        std::shared_ptr<XmlParser> xmlParser,
        std::shared_ptr<AlertManager> alertManager,
        std::shared_ptr<LogManager> logManager,
        std::shared_ptr<TmiAnalyzer> tmiAnalyzer,
        QObject* parent = nullptr);

    ~MonitoringService();

    /**
     * @brief Запуск мониторинга
     */
    void start();
    
    /**
     * @brief Остановка мониторинга
     */
    void stop();
    
    /**
     * @brief Проверка состояния мониторинга
     * @return true, если мониторинг запущен
     */
    bool isRunning() const;
    
    /**
     * @brief Установка интервала опроса
     * @param interval Интервал в миллисекундах
     */
    void setPollingInterval(int interval);
    
    /**
     * @brief Получение интервала опроса
     * @return Интервал в миллисекундах
     */
    int getPollingInterval() const;
    
    /**
     * @brief Установка таймаута сторожевого таймера
     * @param timeout Таймаут в миллисекундах
     */
    void setWatchdogTimeout(int timeout);
    
    /**
     * @brief Получение таймаута сторожевого таймера
     * @return Таймаут в миллисекундах
     */
    int getWatchdogTimeout() const;

public slots:
    /**
     * @brief Проверка параметров
     */
    void checkParameters();
    
    /**
     * @brief Обновление списка параметров
     */
    void refreshParameterList();

signals:
    /**
     * @brief Сигнал изменения статуса мониторинга
     * @param running Статус мониторинга
     */
    void statusChanged(bool running);
    
    /**
     * @brief Сигнал изменения статуса ТМИ
     * @param available Доступность ТМИ
     */
    void tmiStatusChanged(bool available);
    
    /**
     * @brief Сигнал изменения статуса соединения
     * @param connected Статус соединения
     */
    void connectionStatusChanged(bool connected);
    
    /**
     * @brief Сигнал изменения значения параметра
     * @param name Имя параметра
     * @param value Значение параметра
     */
    void parameterValueChanged(const QString& name, const QVariant& value);

private slots:
    /**
     * @brief Обработчик срабатывания сторожевого таймера
     */
    void onWatchdogTimeout();
    
    /**
     * @brief Обработчик изменения списка параметров
     * @param name Имя параметра
     * @param type Тип параметра
     */
    void onParameterListChanged(const QString& name, ParameterType type);
    
    /**
     * @brief Обработчик изменения статуса ТМИ от анализатора
     * @param status Новый статус ТМИ
     */
    void onTmiAnalyzerStatusChanged(bool status);
    
    /**
     * @brief Обработчик обнаружения аномалии ТМИ
     * @param type Тип аномалии
     * @param message Сообщение об аномалии
     */
    void onTmiAnomalyDetected(int type, const QString& message);

private:
    std::shared_ptr<SotmClient> m_sotmClient;          ///< Клиент СОТМ
    std::shared_ptr<ParameterModel> m_parameterModel;  ///< Модель параметров
    std::shared_ptr<XmlParser> m_xmlParser;            ///< Парсер XML
    std::shared_ptr<AlertManager> m_alertManager;      ///< Менеджер оповещений
    std::shared_ptr<LogManager> m_logManager;          ///< Менеджер журнала
    std::shared_ptr<TmiAnalyzer> m_tmiAnalyzer;        ///< Анализатор телеметрии

    QTimer* m_monitoringTimer;                        ///< Таймер мониторинга
    QTimer* m_watchdogTimer;                          ///< Сторожевой таймер
    QTimer* m_paramListRefreshTimer;                  ///< Таймер обновления списка параметров
    
    std::atomic<bool> m_running;                      ///< Флаг работы сервиса
    std::atomic<bool> m_watchdogTriggered;            ///< Флаг срабатывания сторожевого таймера
    std::atomic<bool> m_parameterListChanged;         ///< Флаг изменения списка параметров
    std::atomic<bool> m_tmiStatus;                    ///< Статус ТМИ
    
    QVector<QString> m_currentParameterNames;         ///< Текущий список имен параметров
    
    /**
     * @brief Сброс сторожевого таймера
     */
    void resetWatchdog();
};

} // namespace ParamControl