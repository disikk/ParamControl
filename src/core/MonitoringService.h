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

class MonitoringService : public QObject {
    Q_OBJECT

public:
    explicit MonitoringService(
        std::shared_ptr<SotmClient> sotmClient,
        std::shared_ptr<ParameterModel> parameterModel,
        std::shared_ptr<XmlParser> xmlParser,
        std::shared_ptr<AlertManager> alertManager,
        std::shared_ptr<LogManager> logManager,
        QObject* parent = nullptr);

    ~MonitoringService();

public slots:
    void start();
    void stop();
    void checkParameters();
    void refreshParameterList();

signals:
    void statusChanged(bool running);
    void tmiStatusChanged(bool available);
    void connectionStatusChanged(bool connected);
    void parameterStatusChanged(const QString& name, bool status);
    void parameterValueChanged(const QString& name, const QVariant& value);

private slots:
    void onWatchdogTimeout();
    void onParameterListChanged(const QString& name, ParameterType type);

private:
    std::shared_ptr<SotmClient> m_sotmClient;
    std::shared_ptr<ParameterModel> m_parameterModel;
    std::shared_ptr<XmlParser> m_xmlParser;
    std::shared_ptr<AlertManager> m_alertManager;
    std::shared_ptr<LogManager> m_logManager;

    QTimer* m_monitoringTimer;     // Таймер для периодической проверки параметров
    QTimer* m_watchdogTimer;       // Таймер-сторожевой пес для обнаружения зависаний
    QTimer* m_paramListRefreshTimer; // Таймер для обновления списка параметров
    
    std::atomic<bool> m_running;   // Флаг работы сервиса
    std::atomic<bool> m_watchdogTriggered; // Флаг срабатывания сторожевого таймера
    std::atomic<bool> m_parameterListChanged; // Флаг изменения списка параметров
    
    QVector<QString> m_currentParameterNames; // Текущий список имен параметров
    
    void resetWatchdog();
};
