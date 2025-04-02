#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QFileDialog>
#include <QDebug>

#include "ParameterDialog.h"
#include "ConnectionDialog.h"
#include "LogDialog.h"
#include "KaZsDialog.h"
#include "MessageBox.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_parameterModel(std::make_shared<ParameterModel>())
    , m_sotmClient(std::make_shared<SotmClient>())
    , m_logManager(std::make_shared<LogManager>())
    , m_alertManager(std::make_shared<AlertManager>())
    , m_xmlParser(std::make_shared<XmlParser>())
{
    ui->setupUi(this);
    
    // Создаем сервис мониторинга
    m_monitoringService = std::make_shared<MonitoringService>(
        m_sotmClient, m_parameterModel, m_xmlParser, m_alertManager, m_logManager, this);
    
    // Создаем модели для таблиц
    m_parameterTableModel = std::make_unique<ParameterTableModel>(m_parameterModel);
    m_logTableModel = std::make_unique<LogTableModel>(m_logManager);
    
    // Настраиваем интерфейс
    setupUi();
    
    // Устанавливаем соединения сигналов и слотов
    setupConnections();
    
    // Загружаем настройки
    loadSettings();
    
    // Инициализируем логирование
    m_logManager->initialize(QString("LOG_%1.txt").arg(m_sotmClient->getSettings().kaNumber));
    
    // Устанавливаем звук для оповещения о пропадании ТМИ
    m_alertManager->setAlertSound(AlertType::NoTmi, ui->textBoxNoTmiSound->text());
    
    // Устанавливаем заголовок окна
    setWindowTitle(QString("КА: %1   ЗС: %2")
                 .arg(m_sotmClient->getSettings().kaNumber)
                 .arg(m_sotmClient->getSettings().zsNumber));
    
    // Обновляем индикаторы
    updateStatusIndicators();
}

MainWindow::~MainWindow() {
    // Останавливаем мониторинг перед удалением окна
    if (m_monitoringService) {
        m_monitoringService->stop();
    }
    
    // Сохраняем настройки
    saveSettings();
    
    delete ui;
}

void MainWindow::setupUi() {
    // Настраиваем таблицу параметров
    ui->parameterTableView->setModel(m_parameterTableModel.get());
    ui->parameterTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->parameterTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->parameterTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->parameterTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Настраиваем таблицу логов
    ui->logTableView->setModel(m_logTableModel.get());
    ui->logTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->logTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Настраиваем сплиттер
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);
    
    // Настраиваем индикаторы
    ui->connectionStatusButton->setEnabled(false);
    ui->tmiStatusButton->setEnabled(false);
    ui->kaIndicatorButton->setEnabled(false);
    ui->zsIndicatorButton->setEnabled(false);
    
    // Настраиваем кнопки управления мониторингом
    ui->stopButton->setEnabled(false);
    ui->tmiSoundToggleButton->setEnabled(false);
    
    // Настраиваем метку для СЕК
    ui->sekLabel->setStyleSheet("background-color: black; color: yellow; font-family: Consolas;");
    ui->sekLabel->setText("СЕК:\n--");
}

void MainWindow::setupConnections() {
    // Подключаем кнопки
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartMonitoringClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::onStopMonitoringClicked);
    connect(ui->addParameterButton, &QPushButton::clicked, this, &MainWindow::onAddParameterClicked);
    connect(ui->removeParameterButton, &QPushButton::clicked, this, &MainWindow::onRemoveParameterClicked);
    connect(ui->connectionSettingsButton, &QPushButton::clicked, this, &MainWindow::onConfigureConnectionClicked);
    connect(ui->tmiSoundToggleButton, &QPushButton::clicked, this, &MainWindow::onToggleTmiSoundClicked);
    connect(ui->showLogButton, &QPushButton::clicked, this, &MainWindow::onShowLogClicked);
    connect(ui->changeKaZsButton, &QPushButton::clicked, this, &MainWindow::onChangeKaZsClicked);
    connect(ui->toggleParameterPanelButton, &QPushButton::clicked, this, &MainWindow::onToggleParameterPanelClicked);
    connect(ui->selectNoTmiSoundButton, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "Выбрать звуковой файл",
                                                   QString(), "WAV Files (*.wav)");
        if (!file.isEmpty()) {
            ui->textBoxNoTmiSound->setText(file);
            m_alertManager->setAlertSound(AlertType::NoTmi, file);
        }
    });
    
    // Подключаем контекстные меню
    connect(ui->parameterTableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onParameterContextMenu);
    connect(ui->logTableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onLogContextMenu);
    
    // Подключаем события от сервисов
    connect(m_monitoringService.get(), &MonitoringService::statusChanged,
            this, &MainWindow::onMonitoringStatusChanged);
    connect(m_monitoringService.get(), &MonitoringService::tmiStatusChanged,
            this, &MainWindow::onTmiStatusChanged);
    connect(m_monitoringService.get(), &MonitoringService::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    connect(m_monitoringService.get(), &MonitoringService::parameterValueChanged,
            this, &MainWindow::onParameterValueChanged);
    
    // Подключаем события от модели параметров
    connect(m_parameterModel.get(), &ParameterModel::parameterStatusChanged,
            m_parameterTableModel.get(), &ParameterTableModel::onParameterStatusChanged);
    connect(m_parameterModel.get(), &ParameterModel::parameterAdded,
            m_parameterTableModel.get(), &ParameterTableModel::onParameterAdded);
    connect(m_parameterModel.get(), &ParameterModel::parameterRemoved,
            m_parameterTableModel.get(), &ParameterTableModel::onParameterRemoved);
    connect(m_parameterModel.get(), &ParameterModel::parameterUpdated,
            m_parameterTableModel.get(), &ParameterTableModel::onParameterUpdated);
    connect(m_parameterModel.get(), &ParameterModel::parameterSoundChanged,
            m_parameterTableModel.get(), &ParameterTableModel::onParameterSoundChanged);
    
    // Подключаем события логирования
    connect(m_logManager.get(), &LogManager::logEntryAdded,
            m_logTableModel.get(), &LogTableModel::onLogEntryAdded);
}

void MainWindow::loadSettings() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ParamControl", "ParamControl");
    
    // Загружаем настройки окна
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
    
    // Загружаем состояние сплиттера
    ui->splitter->restoreState(settings.value("splitterState").toByteArray());
    
    // Загружаем настройки СОТМ
    SotmSettings sotmSettings;
    sotmSettings.ipAddress = settings.value("sotm/ipAddress", "127.0.0.1").toString();
    sotmSettings.port = settings.value("sotm/port", 1234).toUInt();
    sotmSettings.kaNumber = settings.value("sotm/kaNumber", 100).toUInt();
    sotmSettings.zsNumber = settings.value("sotm/zsNumber", 0).toUInt();
    sotmSettings.responseTimeoutMs = settings.value("sotm/responseTimeoutMs", 5000).toInt();
    m_sotmClient->setSettings(sotmSettings);
    
    // Загружаем путь к звуковому файлу
    QString tmiSoundFile = settings.value("sounds/noTmiSound", "./NoTMI.wav").toString();
    ui->textBoxNoTmiSound->setText(tmiSoundFile);
    
    // Загружаем список параметров из JSON файла
    QString paramFileName = QString("parameters_ka%1.json").arg(sotmSettings.kaNumber);
    m_parameterModel->loadParameters(paramFileName);
}

void MainWindow::saveSettings() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ParamControl", "ParamControl");
    
    // Сохраняем настройки окна
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
    
    // Сохраняем состояние сплиттера
    settings.setValue("splitterState", ui->splitter->saveState());
    
    // Сохраняем настройки СОТМ
    const SotmSettings& sotmSettings = m_sotmClient->getSettings();
    settings.setValue("sotm/ipAddress", sotmSettings.ipAddress);
    settings.setValue("sotm/port", sotmSettings.port);
    settings.setValue("sotm/kaNumber", sotmSettings.kaNumber);
    settings.setValue("sotm/zsNumber", sotmSettings.zsNumber);
    settings.setValue("sotm/responseTimeoutMs", sotmSettings.responseTimeoutMs);
    
    // Сохраняем путь к звуковому файлу
    settings.setValue("sounds/noTmiSound", ui->textBoxNoTmiSound->text());
    
    // Сохраняем список параметров
    QString paramFileName = QString("parameters_ka%1.json").arg(sotmSettings.kaNumber);
    m_parameterModel->saveParameters(paramFileName);
}

void MainWindow::onStartMonitoringClicked() {
    // Проверяем, что все необходимые настройки заданы
    const SotmSettings& settings = m_sotmClient->getSettings();
    
    if (settings.ipAddress.isEmpty()) {
        MessageBox msg("Требуется сперва задать параметры подключения к СОТМ.", 
                      "Ошибка", this);
        msg.exec();
        return;
    }
    
    if (settings.kaNumber == 0) {
        MessageBox msg("Требуется сперва указать номер КА.", 
                      "Ошибка", this);
        msg.exec();
        return;
    }
    
    if (ui->textBoxNoTmiSound->text().isEmpty()) {
        MessageBox msg("Требуется указать звуковой файл сигнализации пропадания ТМИ", 
                      "Ошибка", this);
        msg.exec();
        return;
    }
    
    if (m_parameterModel->getAllParameters().isEmpty()) {
        MessageBox msg("Требуется сперва задать параметры для контроля.", 
                      "Ошибка", this);
        msg.exec();
        return;
    }
    
    // Запускаем мониторинг
    m_monitoringService->start();
}

void MainWindow::onStopMonitoringClicked() {
    // Запрашиваем подтверждение
    MessageBox msg("Точно остановить контроль?", 
                  "Внимание!", this, true);
    
    if (msg.result() == QDialog::Accepted) {
        // Останавливаем мониторинг
        m_monitoringService->stop();
    }
}

void MainWindow::onAddParameterClicked() {
    // Открываем диалог добавления параметра
    ParameterDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Получаем новый параметр
        std::shared_ptr<Parameter> parameter = dialog.getParameter();
        
        if (parameter) {
            // Добавляем параметр в модель
            m_parameterModel->addParameter(parameter);
            
            // Сохраняем список параметров
            const SotmSettings& settings = m_sotmClient->getSettings();
            QString paramFileName = QString("parameters_ka%1.json").arg(settings.kaNumber);
            m_parameterModel->saveParameters(paramFileName);
            
            m_logManager->log(LogLevel::Info, "Параметры", 
                             QString("Добавлен параметр %1 (%2)")
                             .arg(parameter->getName())
                             .arg(parameter->getDescription()));
        }
    }
}

void MainWindow::onRemoveParameterClicked() {
    // Проверяем, выбран ли параметр
    QModelIndex index = ui->parameterTableView->currentIndex();
    if (!index.isValid()) {
        MessageBox msg("Не выбран параметр для удаления.", 
                      "Ошибка", this);
        msg.exec();
        return;
    }
    
    // Получаем данные выбранного параметра
    QString paramName = m_parameterTableModel->data(
        m_parameterTableModel->index(index.row(), ParameterTableModel::NameColumn)).toString();
    QString paramDesc = m_parameterTableModel->data(
        m_parameterTableModel->index(index.row(), ParameterTableModel::ConditionColumn)).toString();
    
    // Запрашиваем подтверждение
    MessageBox msg(QString("Точно удалить данное условие контроля?\nПараметр: %1,\nУсловие контроля: %2")
                  .arg(paramName)
                  .arg(paramDesc), 
                  "Подтверждение", this, true);
    
    if (msg.result() == QDialog::Accepted) {
        // Получаем параметр
        int paramIndex = index.row();
        std::shared_ptr<Parameter> param = m_parameterModel->getAllParameters()[paramIndex];
        
        // Удаляем параметр
        m_parameterModel->removeParameter(param->getName(), param->getType());
        
        // Сохраняем список параметров
        const SotmSettings& settings = m_sotmClient->getSettings();
        QString paramFileName = QString("parameters_ka%1.json").arg(settings.kaNumber);
        m_parameterModel->saveParameters(paramFileName);
        
        m_logManager->log(LogLevel::Info, "Параметры", 
                         QString("Удален параметр %1 (%2)")
                         .arg(param->getName())
                         .arg(param->getDescription()));
    }
}

void MainWindow::onEditParameterClicked() {
    // Проверяем, выбран ли параметр
    QModelIndex index = ui->parameterTableView->currentIndex();
    if (!index.isValid()) {
        MessageBox msg("Не выбран параметр для редактирования.", 
                      "Ошибка", this);
        msg.exec();
        return;
    }
    
    // Получаем параметр
    int paramIndex = index.row();
    std::shared_ptr<Parameter> param = m_parameterModel->getAllParameters()[paramIndex];
    
    // Открываем диалог редактирования
    ParameterDialog dialog(this);
    dialog.setParameter(param);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Получаем обновленный параметр
        std::shared_ptr<Parameter> updatedParam = dialog.getParameter();
        
        if (updatedParam) {
            // Удаляем старый параметр
            m_parameterModel->removeParameter(param->getName(), param->getType());
            
            // Добавляем обновленный параметр
            m_parameterModel->addParameter(updatedParam);
            
            // Сохраняем список параметров
            const SotmSettings& settings = m_sotmClient->getSettings();
            QString paramFileName = QString("parameters_ka%1.json").arg(settings.kaNumber);
            m_parameterModel->saveParameters(paramFileName);
            
            m_logManager->log(LogLevel::Info, "Параметры", 
                             QString("Изменен параметр %1 (%2)")
                             .arg(updatedParam->getName())
                             .arg(updatedParam->getDescription()));
        }
    }
}

void MainWindow::onConfigureConnectionClicked() {
    // Открываем диалог настройки соединения
    ConnectionDialog dialog(m_sotmClient->getSettings(), this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Получаем новые настройки
        SotmSettings newSettings = dialog.getSettings();
        
        // Применяем новые настройки
        m_sotmClient->setSettings(newSettings);
        
        // Сохраняем настройки
        saveSettings();
        
        m_logManager->log(LogLevel::Info, "Настройки", 
                         QString("Изменены настройки подключения к СОТМ: %1:%2")
                         .arg(newSettings.ipAddress)
                         .arg(newSettings.port));
    }
}

void MainWindow::onToggleTmiSoundClicked() {
    // Проверяем текущее состояние звука
    bool tmiSoundEnabled = m_alertManager->isAlertEnabled(AlertType::NoTmi);
    
    // Изменяем состояние
    m_alertManager->setAlertEnabled(AlertType::NoTmi, !tmiSoundEnabled);
    
    // Обновляем кнопку
    updateTmiSoundButton();
    
    m_logManager->log(LogLevel::Info, "Настройки", 
                     QString("Звук ТМИ %1").arg(!tmiSoundEnabled ? "включен" : "отключен"));
}

void MainWindow::onShowLogClicked() {
    // Открываем диалог просмотра лога
    LogDialog dialog(m_logManager, this);
    dialog.exec();
}

void MainWindow::onChangeKaZsClicked() {
    // Запрашиваем подтверждение
    MessageBox msg("Для изменения выбора КА и ЗС потребуется\nперезапуск приложения. Продолжить?", 
                  "Внимание!", this, true);
    
    if (msg.result() == QDialog::Accepted) {
        // Открываем диалог выбора КА и ЗС
        KaZsDialog dialog(m_sotmClient->getSettings().kaNumber, 
                          m_sotmClient->getSettings().zsNumber, this);
        
        if (dialog.exec() == QDialog::Accepted) {
            // Получаем новые номера КА и ЗС
            quint16 newKaNumber = dialog.getKaNumber();
            quint16 newZsNumber = dialog.getZsNumber();
            
            // Обновляем настройки
            SotmSettings settings = m_sotmClient->getSettings();
            settings.kaNumber = newKaNumber;
            settings.zsNumber = newZsNumber;
            m_sotmClient->setSettings(settings);
            
            // Сохраняем настройки
            saveSettings();
            
            // Перезапускаем приложение
            qApp->exit(0);
            QProcess::startDetached(qApp->applicationFilePath(), QStringList());
        }
    }
}

void MainWindow::onToggleParameterPanelClicked() {
    // Показываем или скрываем панель параметров
    bool isVisible = !ui->splitter->widget(0)->isHidden();
    ui->splitter->widget(0)->setVisible(!isVisible);
    
    // Изменяем текст кнопки
    ui->toggleParameterPanelButton->setText(isVisible ? ">>>" : "<<<");
}

void MainWindow::onMonitoringStatusChanged(bool running) {
    // Обновляем доступность кнопок
    ui->startButton->setEnabled(!running);
    ui->stopButton->setEnabled(running);
    ui->connectionSettingsButton->setEnabled(!running);
    ui->changeKaZsButton->setEnabled(!running);
    ui->tmiSoundToggleButton->setEnabled(running);
    
    // В новой реализации кнопки добавления/удаления/редактирования
    // параметров доступны всегда, даже когда мониторинг запущен
}

void MainWindow::onTmiStatusChanged(bool available) {
    // Обновляем индикатор ТМИ
    ui->tmiStatusButton->setStyleSheet(
        available ? "background-color: YellowGreen; border-color: YellowGreen;" 
                 : "background-color: IndianRed; border-color: IndianRed;");
}

void MainWindow::onConnectionStatusChanged(bool connected) {
    // Обновляем индикатор соединения
    ui->connectionStatusButton->setStyleSheet(
        connected ? "background-color: YellowGreen; border-color: YellowGreen;" 
                  : "background-color: IndianRed; border-color: IndianRed;");
}

void MainWindow::onParameterStatusChanged(const QString& name, bool status) {
    // Обновление происходит через модель таблицы
    Q_UNUSED(name);
    Q_UNUSED(status);
}

void MainWindow::onParameterValueChanged(const QString& name, const QVariant& value) {
    // Обрабатываем изменение значения параметра СЕК
    if (name == "СЕК") {
        bool ok;
        int sekValue = value.toInt(&ok);
        
        if (ok) {
            // Обновляем дисплей СЕК
            ui->sekLabel->setText(QString("СЕК:\n%1").arg(sekValue));
            
            // Меняем цвет фона в зависимости от четности значения
            if (sekValue % 2 == 0) {
                ui->sekLabel->setStyleSheet("background-color: black; color: yellow; font-family: Consolas;");
            } else {
                ui->sekLabel->setStyleSheet("background-color: silver; color: black; font-family: Consolas;");
            }
        } else {
            // Если не удалось преобразовать в число
            ui->sekLabel->setText("СЕК:\nError");
            ui->sekLabel->setStyleSheet("background-color: IndianRed; color: black; font-family: Consolas;");
        }
    }
}

void MainWindow::onParameterContextMenu(const QPoint& pos) {
    // Проверяем, есть ли выбранный параметр
    QModelIndex index = ui->parameterTableView->indexAt(pos);
    if (!index.isValid()) {
        // Показываем меню только для добавления параметра
        QMenu menu(this);
        QAction* addAction = menu.addAction("Добавить параметр");
        
        QAction* selectedAction = menu.exec(ui->parameterTableView->viewport()->mapToGlobal(pos));
        
        if (selectedAction == addAction) {
            onAddParameterClicked();
        }
        
        return;
    }
    
    // Получаем данные выбранного параметра
    int paramIndex = index.row();
    std::shared_ptr<Parameter> param = m_parameterModel->getAllParameters()[paramIndex];
    
    // Создаем контекстное меню
    QMenu menu(this);
    
    // Добавляем действия
    QAction* addAction = menu.addAction("Добавить параметр");
    QAction* editAction = menu.addAction("Редактировать параметр");
    QAction* deleteAction = menu.addAction("Удалить параметр");
    
    menu.addSeparator();
    
    QAction* soundAction = menu.addAction("Звук");
    soundAction->setCheckable(true);
    soundAction->setChecked(param->isSoundEnabled());
    
    // Показываем меню
    QAction* selectedAction = menu.exec(ui->parameterTableView->viewport()->mapToGlobal(pos));
    
    // Обрабатываем выбранное действие
    if (selectedAction == addAction) {
        onAddParameterClicked();
    } else if (selectedAction == editAction) {
        onEditParameterClicked();
    } else if (selectedAction == deleteAction) {
        onRemoveParameterClicked();
    } else if (selectedAction == soundAction) {
        // Изменяем состояние звука для параметра
        m_parameterModel->updateParameterSound(
            param->getName(), param->getType(), soundAction->isChecked());
        
        // Сохраняем настройки
        const SotmSettings& settings = m_sotmClient->getSettings();
        QString paramFileName = QString("parameters_ka%1.json").arg(settings.kaNumber);
        m_parameterModel->saveParameters(paramFileName);
        
        m_logManager->log(LogLevel::Info, "Параметры", 
                         QString("Звук для параметра %1 %2")
                         .arg(param->getName())
                         .arg(param->isSoundEnabled() ? "включен" : "отключен"));
    }
}

void MainWindow::onLogContextMenu(const QPoint& pos) {
    // Создаем контекстное меню
    QMenu menu(this);
    
    // Добавляем действие очистки лога
    QAction* clearAction = menu.addAction("Удалить все записи");
    
    // Показываем меню
    QAction* selectedAction = menu.exec(ui->logTableView->viewport()->mapToGlobal(pos));
    
    // Обрабатываем выбранное действие
    if (selectedAction == clearAction) {
        onClearLog();
    }
}

void MainWindow::onClearLog() {
    // Запрашиваем подтверждение
    MessageBox msg("Удалить все строки?", "Подтверждение", this, true);
    
    if (msg.result() == QDialog::Accepted) {
        // Очищаем лог
        m_logManager->clearLog();
        
        // Оповещаем модель таблицы
        m_logTableModel->onLogCleared();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Проверяем, запущен ли мониторинг
    if (m_monitoringService && m_monitoringService->statusChanged()) {
        // Запрашиваем подтверждение
        MessageBox msg("Закрыть программу?", "Внимание!", this, true);
        
        if (msg.result() == QDialog::Accepted) {
            // Останавливаем мониторинг
            m_monitoringService->stop();
            
            // Сохраняем настройки
            saveSettings();
            
            // Принимаем событие
            event->accept();
        } else {
            // Отклоняем событие
            event->ignore();
        }
    } else {
        // Сохраняем настройки
        saveSettings();
        
        // Принимаем событие
        event->accept();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    // Обработка клавиши Escape для остановки звуков
    if (event->key() == Qt::Key_Escape) {
        // Останавливаем все звуки
        m_alertManager->stopAllAlerts();
        
        // Сбрасываем состояние параметров изменения в исходное
        for (const auto& param : m_parameterModel->getAllParameters()) {
            if (param->getType() == ParameterType::Changed) {
                param->updateValue(param->getCurrentValue());
            }
        }
    } else {
        // Передаем событие родителю
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::updateStatusIndicators() {
    // Обновляем индикаторы КА и ЗС
    const SotmSettings& settings = m_sotmClient->getSettings();
    ui->kaIndicatorButton->setText(QString("КА %1").arg(settings.kaNumber));
    ui->zsIndicatorButton->setText(QString("ЗС %1").arg(settings.zsNumber));
}

void MainWindow::updateTmiSoundButton() {
    // Обновляем кнопку включения/отключения звука ТМИ
    bool tmiSoundEnabled = m_alertManager->isAlertEnabled(AlertType::NoTmi);
    
    if (tmiSoundEnabled) {
        ui->tmiSoundToggleButton->setText("Отключить сигнал ТМИ");
        ui->tmiSoundToggleButton->setStyleSheet("");
    } else {
        ui->tmiSoundToggleButton->setText("Звук ТМИ отключен. Включить!");
        ui->tmiSoundToggleButton->setStyleSheet("background-color: IndianRed;");
    }
}
