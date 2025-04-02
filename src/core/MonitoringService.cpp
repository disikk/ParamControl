#include "MonitoringService.h"
#include <QDebug>

// Интервалы времени для таймеров (в миллисекундах)
constexpr int MONITORING_INTERVAL_MS = 1000;   // Интервал запросов к СОТМ
constexpr int WATCHDOG_TIMEOUT_MS = 5000;      // Таймаут сторожевого таймера

MonitoringService::MonitoringService(
    std::shared_ptr<SotmClient> sotmClient,
    std::shared_ptr<ParameterModel> parameterModel,
    std::shared_ptr<XmlParser> xmlParser,
    std::shared_ptr<AlertManager> alertManager,
    std::shared_ptr<LogManager> logManager,
    QObject* parent)
    : QObject(parent)
    , m_sotmClient(std::move(sotmClient))
    , m_parameterModel(std::move(parameterModel))
    , m_xmlParser(std::move(xmlParser))
    , m_alertManager(std::move(alertManager))
    , m_logManager(std::move(logManager))
    , m_monitoringTimer(new QTimer(this))
    , m_watchdogTimer(new QTimer(this))
    , m_paramListRefreshTimer(new QTimer(this))
    , m_running(false)
    , m_watchdogTriggered(false)
    , m_parameterListChanged(false)
{
    // Настраиваем таймер мониторинга
    m_monitoringTimer->setInterval(MONITORING_INTERVAL_MS);
    connect(m_monitoringTimer, &QTimer::timeout, this, &MonitoringService::checkParameters);
    
    // Настраиваем сторожевой таймер
    m_watchdogTimer->setInterval(WATCHDOG_TIMEOUT_MS);
    m_watchdogTimer->setSingleShot(true);
    connect(m_watchdogTimer, &QTimer::timeout, this, &MonitoringService::onWatchdogTimeout);
    
    // Настраиваем таймер обновления списка параметров
    m_paramListRefreshTimer->setInterval(3000); // Обновление каждые 3 секунды
    connect(m_paramListRefreshTimer, &QTimer::timeout, this, &MonitoringService::refreshParameterList);
    
    // Подключаем сигналы SotmClient
    connect(m_sotmClient.get(), &SotmClient::connectionStatusChanged,
            this, &MonitoringService::connectionStatusChanged);
    
    // Подключаем сигналы ParameterModel для отслеживания изменений списка параметров
    connect(m_parameterModel.get(), &ParameterModel::parameterAdded,
            this, &MonitoringService::onParameterListChanged);
    connect(m_parameterModel.get(), &ParameterModel::parameterRemoved,
            this, &MonitoringService::onParameterListChanged);
    connect(m_parameterModel.get(), &ParameterModel::parameterUpdated,
            this, &MonitoringService::onParameterListChanged);
}

MonitoringService::~MonitoringService() {
    stop();
}

void MonitoringService::start() {
    if (m_running) {
        return;
    }
    
    // Проверяем, есть ли подключение к СОТМ
    if (!m_sotmClient->isConnected()) {
        // Пытаемся подключиться
        if (!m_sotmClient->connect(m_sotmClient->getSettings())) {
            m_logManager->log(LogLevel::Error, "Соед. с СОТМ", "Не удалось подключиться к СОТМ", "", false);
            return;
        }
    }
    
    m_running = true;
    m_watchdogTriggered = false;
    m_parameterListChanged = true; // Инициируем первоначальную загрузку списка параметров
    
    // Инициализируем список параметров
    refreshParameterList();
    
    // Запускаем таймеры
    m_monitoringTimer->start();
    m_paramListRefreshTimer->start();
    resetWatchdog();
    
    m_logManager->log(LogLevel::Info, "Мониторинг", "Мониторинг запущен");
    emit statusChanged(true);
}

void MonitoringService::stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    // Останавливаем таймеры
    m_monitoringTimer->stop();
    m_watchdogTimer->stop();
    m_paramListRefreshTimer->stop();
    
    // Останавливаем все оповещения
    m_alertManager->stopAllAlerts();
    
    m_logManager->log(LogLevel::Info, "Мониторинг", "Мониторинг остановлен");
    emit statusChanged(false);
}

void MonitoringService::checkParameters() {
    // Если сервис не запущен, ничего не делаем
    if (!m_running) {
        return;
    }
    
    // Сбрасываем сторожевой таймер
    resetWatchdog();
    
    // Используем текущий список имен параметров
    QVector<QString> parameterNames = m_currentParameterNames;
    
    // Добавляем параметр СЕК, если его нет в списке
    if (!parameterNames.contains("СЕК")) {
        parameterNames.append("СЕК");
    }
    
    // Создаем структуру для запроса
    SotmRequestParams requestParams;
    requestParams.kaNumber = m_sotmClient->getSettings().kaNumber;
    requestParams.zsNumber = m_sotmClient->getSettings().zsNumber;
    requestParams.updateIntervalMs = MONITORING_INTERVAL_MS;
    requestParams.parameterNames = parameterNames;
    
    // Создаем XML-запрос
    QByteArray requestData = m_xmlParser->createParameterRequest(requestParams);
    
    // Отправляем запрос
    auto responseOpt = m_sotmClient->sendRequest(requestData);
    if (!responseOpt) {
        // Ошибка при отправке запроса
        m_logManager->log(LogLevel::Error, "СОТМ", "Ошибка при отправке запроса", "", false);
        emit connectionStatusChanged(false);
        
        // Включаем оповещение о проблемах с ТМИ
        m_alertManager->playAlert(AlertType::NoTmi);
        emit tmiStatusChanged(false);
        return;
    }
    
    // Парсим ответ
    QVector<ParameterValue> parameterValues;
    try {
        parameterValues = m_xmlParser->parseParameterResponse(*responseOpt);
    } catch (const std::exception& e) {
        m_logManager->log(LogLevel::Error, "СОТМ", 
                          QString("Ошибка при разборе ответа: %1").arg(e.what()), "", false);
        
        // Включаем оповещение о проблемах с ТМИ
        m_alertManager->playAlert(AlertType::NoTmi);
        emit tmiStatusChanged(false);
        return;
    }
    
    // Если ответ пустой, считаем что проблемы с ТМИ
    if (parameterValues.isEmpty()) {
        m_logManager->log(LogLevel::Error, "СОТМ", "Пустой ответ от СОТМ", "", false);
        
        // Включаем оповещение о проблемах с ТМИ
        m_alertManager->playAlert(AlertType::NoTmi);
        emit tmiStatusChanged(false);
        return;
    }
    
    // ТМИ в порядке, выключаем оповещение
    m_alertManager->stopAlert(AlertType::NoTmi);
    emit tmiStatusChanged(true);
    
    // Проверяем параметр СЕК
    for (const auto& paramValue : parameterValues) {
        if (paramValue.name == "СЕК") {
            // Сигнализируем об изменении значения СЕК
            emit parameterValueChanged(paramValue.name, paramValue.value);
            break;
        }
    }
    
    // Проверяем все параметры
    m_parameterModel->checkParameters(parameterValues);
}

void MonitoringService::onWatchdogTimeout() {
    // Если сервис не запущен, ничего не делаем
    if (!m_running) {
        return;
    }
    
    // Устанавливаем флаг срабатывания
    m_watchdogTriggered = true;
    
    m_logManager->log(LogLevel::Error, "Мониторинг", 
                      "Сработал сторожевой таймер - перезапуск мониторинга", "", false);
    
    // Останавливаем и перезапускаем мониторинг
    stop();
    start();
}

void MonitoringService::resetWatchdog() {
    // Перезапускаем сторожевой таймер
    m_watchdogTimer->stop();
    m_watchdogTimer->start();
}

void MonitoringService::refreshParameterList() {
    // Обновляем список только если он изменился
    if (m_parameterListChanged) {
        // Получаем актуальный список имен параметров
        m_currentParameterNames = m_parameterModel->getAllParameterNames();
        m_parameterListChanged = false;
        
        // Логируем обновление списка
        m_logManager->log(LogLevel::Debug, "Мониторинг", 
                          QString("Обновлен список контролируемых параметров (%1)")
                          .arg(m_currentParameterNames.size()));
        
        // Сохраняем параметры
        const SotmSettings& settings = m_sotmClient->getSettings();
        QString paramFileName = QString("parameters_ka%1.json").arg(settings.kaNumber);
        m_parameterModel->saveParameters(paramFileName);
    }
}

void MonitoringService::onParameterListChanged(const QString& name, ParameterType type) {
    // Устанавливаем флаг необходимости обновления списка параметров
    m_parameterListChanged = true;
    
    // Логируем изменение
    QString typeStr;
    switch (type) {
        case ParameterType::Equals: typeStr = "Equals"; break;
        case ParameterType::NotEquals: typeStr = "NotEquals"; break;
        case ParameterType::InLimits: typeStr = "InLimits"; break;
        case ParameterType::OutOfLimits: typeStr = "OutOfLimits"; break;
        case ParameterType::Changed: typeStr = "Changed"; break;
        default: typeStr = "Unknown"; break;
    }
    
    m_logManager->log(LogLevel::Debug, "Мониторинг", 
                     QString("Изменен список параметров: %1 (%2)").arg(name).arg(typeStr));
}
