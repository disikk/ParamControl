#include "SettingsDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QTimer>

namespace ParamControl {

SettingsDialog::SettingsDialog(const std::shared_ptr<SotmClient>& sotmClient,
                             const std::shared_ptr<UpdateManager>& updateManager,
                             const std::shared_ptr<AlertManager>& alertManager,
                             const std::shared_ptr<MonitoringService>& monitoringService,
                             QWidget* parent)
    : QDialog(parent)
    , m_sotmClient(sotmClient)
    , m_updateManager(updateManager)
    , m_alertManager(alertManager)
    , m_monitoringService(monitoringService)
{
    // Инициализация интерфейса
    setupUi();
    
    // Загрузка настроек
    loadSettings();
    
    // Обновление статуса кнопок
    updateButtonsStatus();
    
    // Подключение сигналов
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    
    // Подключение сигналов выбора звуковых файлов
    for (auto it = m_soundWidgets.begin(); it != m_soundWidgets.end(); ++it) {
        AlertType type = it.key();
        SoundWidgets& widgets = it.value();
        
        connect(widgets.selectButton, &QPushButton::clicked, this, [this, type]() {
            onSelectSoundClicked(type);
        });
    }
    
    // Подключение сигналов вкладки обновлений
    connect(m_selectUpdatePathButton, &QPushButton::clicked, this, &SettingsDialog::onSelectUpdatePathClicked);
    connect(m_checkUpdatesButton, &QPushButton::clicked, this, &SettingsDialog::onCheckUpdatesClicked);
    connect(m_updateManager.get(), &UpdateManager::updateAvailable, this, &SettingsDialog::onUpdateAvailable);
    connect(m_updateManager.get(), &UpdateManager::checkError, this, &SettingsDialog::onCheckError);
    
    // Подключение сигналов вкладки КА и ЗС
    connect(m_changeKaZsButton, &QPushButton::clicked, this, &SettingsDialog::onChangeKaZsClicked);
    
    // Установка заголовка
    setWindowTitle("Настройки");
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::onOkClicked() {
    // Сохраняем настройки
    saveSettings();
    
    // Закрываем диалог
    accept();
}

void SettingsDialog::onApplyClicked() {
    // Сохраняем настройки
    saveSettings();
    
    // Обновляем статус кнопок
    updateButtonsStatus();
}

void SettingsDialog::onCancelClicked() {
    // Закрываем диалог без сохранения
    reject();
}

void SettingsDialog::onSelectSoundClicked(AlertType type) {
    // Проверяем, есть ли виджеты для этого типа
    if (!m_soundWidgets.contains(type)) {
        return;
    }
    
    // Получаем виджеты
    SoundWidgets& widgets = m_soundWidgets[type];
    
    // Показываем диалог выбора файла
    QString filePath = QFileDialog::getOpenFileName(this, "Выбор звукового файла",
                                                 QDir::currentPath(),
                                                 "Звуковые файлы (*.wav)");
    
    if (!filePath.isEmpty()) {
        // Устанавливаем путь в поле
        widgets.pathEdit->setText(filePath);
    }
}

void SettingsDialog::onSelectUpdatePathClicked() {
    // Показываем диалог выбора директории
    QString dirPath = QFileDialog::getExistingDirectory(this, "Выбор директории обновлений",
                                                     QDir::currentPath(),
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dirPath.isEmpty()) {
        // Устанавливаем путь в поле
        m_updatePathEdit->setText(dirPath);
    }
}

void SettingsDialog::onCheckUpdatesClicked() {
    // Обновляем статус
    m_updateStatusLabel->setText("Проверка обновлений...");
    
    // Запускаем проверку обновлений с текущими настройками
    m_updateManager->setUpdatePath(m_updatePathEdit->text());
    
    // Эмитируем сигнал запроса на проверку обновлений
    emit checkForUpdatesRequested();
}

void SettingsDialog::onChangeKaZsClicked() {
    // Показываем предупреждение
    QMessageBox msgBox;
    msgBox.setWindowTitle("Предупреждение");
    msgBox.setText("Для изменения КА и ЗС потребуется перезапуск приложения.");
    msgBox.setInformativeText("Продолжить?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    if (msgBox.exec() == QMessageBox::Yes) {
        // Сохраняем настройки
        saveSettings();
        
        // Эмитируем сигнал запроса на перезапуск
        emit restartRequested("Изменение КА и ЗС");
    }
}

void SettingsDialog::onUpdateAvailable(const UpdateInfo& info) {
    // Обновляем статус
    m_updateStatusLabel->setText(QString("Доступно обновление %1 от %2")
                               .arg(info.version.toString())
                               .arg(info.releaseDate.toString("dd.MM.yyyy")));
}

void SettingsDialog::onCheckError(const QString& errorMessage) {
    // Обновляем статус
    m_updateStatusLabel->setText(QString("Ошибка проверки: %1").arg(errorMessage));
}

void SettingsDialog::setupUi() {
    // Создание основного Layout
    auto mainLayout = new QVBoxLayout(this);
    
    // Создание виджета с вкладками
    m_tabWidget = new QTabWidget(this);
    
    // ------------------------
    // Вкладка Соединение
    // ------------------------
    auto connectionTab = new QWidget();
    auto connectionLayout = new QFormLayout(connectionTab);
    
    // IP-адрес
    m_ipAddressEdit = new QLineEdit(connectionTab);
    connectionLayout->addRow("IP-адрес СОТМ:", m_ipAddressEdit);
    
    // Порт
    m_portSpinBox = new QSpinBox(connectionTab);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(1234);
    connectionLayout->addRow("Порт:", m_portSpinBox);
    
    // Таймаут
    m_timeoutSpinBox = new QSpinBox(connectionTab);
    m_timeoutSpinBox->setRange(1000, 60000);
    m_timeoutSpinBox->setSingleStep(1000);
    m_timeoutSpinBox->setValue(5000);
    m_timeoutSpinBox->setSuffix(" мс");
    connectionLayout->addRow("Таймаут ответа:", m_timeoutSpinBox);
    
    // Интервал опроса
    m_pollingIntervalSpinBox = new QSpinBox(connectionTab);
    m_pollingIntervalSpinBox->setRange(100, 10000);
    m_pollingIntervalSpinBox->setSingleStep(100);
    m_pollingIntervalSpinBox->setValue(1000);
    m_pollingIntervalSpinBox->setSuffix(" мс");
    connectionLayout->addRow("Интервал опроса:", m_pollingIntervalSpinBox);
    
    // Таймаут сторожевого таймера
    m_watchdogTimeoutSpinBox = new QSpinBox(connectionTab);
    m_watchdogTimeoutSpinBox->setRange(1000, 60000);
    m_watchdogTimeoutSpinBox->setSingleStep(1000);
    m_watchdogTimeoutSpinBox->setValue(10000);
    m_watchdogTimeoutSpinBox->setSuffix(" мс");
    connectionLayout->addRow("Таймаут сторожевого таймера:", m_watchdogTimeoutSpinBox);
    
    m_tabWidget->addTab(connectionTab, "Соединение");
    
    // ------------------------
    // Вкладка Звуки
    // ------------------------
    auto soundsTab = new QWidget();
    auto soundsLayout = new QVBoxLayout(soundsTab);
    
    // Создаем группы для каждого типа оповещения
    soundsLayout->addWidget(createSoundGroup(AlertType::NoTmi, "Пропадание ТМИ", soundsTab));
    soundsLayout->addWidget(createSoundGroup(AlertType::ParameterChange, "Изменение параметра", soundsTab));
    soundsLayout->addWidget(createSoundGroup(AlertType::ParameterLimit, "Выход за пределы", soundsTab));
    soundsLayout->addWidget(createSoundGroup(AlertType::Default, "По умолчанию", soundsTab));
    
    soundsLayout->addStretch();
    
    m_tabWidget->addTab(soundsTab, "Звуки");
    
    // ------------------------
    // Вкладка КА и ЗС
    // ------------------------
    auto kaZsTab = new QWidget();
    auto kaZsLayout = new QFormLayout(kaZsTab);
    
    // Номер КА
    m_kaNumberSpinBox = new QSpinBox(kaZsTab);
    m_kaNumberSpinBox->setRange(100, 999);
    m_kaNumberSpinBox->setValue(101);
    kaZsLayout->addRow("Номер КА:", m_kaNumberSpinBox);
    
    // Номер ЗС
    m_zsNumberSpinBox = new QSpinBox(kaZsTab);
    m_zsNumberSpinBox->setRange(0, 999);
    m_zsNumberSpinBox->setValue(123);
    kaZsLayout->addRow("Номер ЗС:", m_zsNumberSpinBox);
    
    // Кнопка изменения
    m_changeKaZsButton = new QPushButton("Изменить КА и ЗС", kaZsTab);
    kaZsLayout->addRow("", m_changeKaZsButton);
    
    // Предупреждение
    auto warningLabel = new QLabel("Внимание! Изменение КА и ЗС\nпотребует перезапуска приложения.", kaZsTab);
    warningLabel->setStyleSheet("color: red;");
    kaZsLayout->addRow("", warningLabel);
    
    m_tabWidget->addTab(kaZsTab, "КА и ЗС");
    
    // ------------------------
    // Вкладка Обновления
    // ------------------------
    auto updatesTab = new QWidget();
    auto updatesLayout = new QVBoxLayout(updatesTab);
    
    // Группа настроек обновлений
    auto updatesGroup = new QGroupBox("Настройки обновлений", updatesTab);
    auto updatesGroupLayout = new QFormLayout(updatesGroup);
    
    // Путь к обновлениям
    auto updatePathLayout = new QHBoxLayout();
    m_updatePathEdit = new QLineEdit(updatesGroup);
    updatePathLayout->addWidget(m_updatePathEdit);
    
    m_selectUpdatePathButton = new QPushButton("Выбрать", updatesGroup);
    updatePathLayout->addWidget(m_selectUpdatePathButton);
    
    updatesGroupLayout->addRow("Путь к обновлениям:", updatePathLayout);
    
    // Флажки
    m_checkAtStartupCheckBox = new QCheckBox("Проверять при запуске", updatesGroup);
    updatesGroupLayout->addRow("", m_checkAtStartupCheckBox);
    
    m_autoInstallCheckBox = new QCheckBox("Автоматически устанавливать", updatesGroup);
    updatesGroupLayout->addRow("", m_autoInstallCheckBox);
    
    // Интервал проверки
    m_checkIntervalSpinBox = new QSpinBox(updatesGroup);
    m_checkIntervalSpinBox->setRange(0, 24 * 60 * 60 * 1000);
    m_checkIntervalSpinBox->setSingleStep(60 * 60 * 1000);
    m_checkIntervalSpinBox->setValue(0);
    m_checkIntervalSpinBox->setSuffix(" мс");
    m_checkIntervalSpinBox->setSpecialValueText("Отключено");
    updatesGroupLayout->addRow("Интервал проверки:", m_checkIntervalSpinBox);
    
    updatesLayout->addWidget(updatesGroup);
    
    // Группа проверки обновлений
    auto checkGroup = new QGroupBox("Проверка обновлений", updatesTab);
    auto checkGroupLayout = new QVBoxLayout(checkGroup);
    
    m_checkUpdatesButton = new QPushButton("Проверить обновления", checkGroup);
    checkGroupLayout->addWidget(m_checkUpdatesButton);
    
    m_updateStatusLabel = new QLabel("Нет информации об обновлениях", checkGroup);
    checkGroupLayout->addWidget(m_updateStatusLabel);
    
    updatesLayout->addWidget(checkGroup);
    
    m_tabWidget->addTab(updatesTab, "Обновления");
    
    // ------------------------
    // Кнопки диалога
    // ------------------------
    auto buttonLayout = new QHBoxLayout();
    
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton("ОК", this);
    buttonLayout->addWidget(m_okButton);
    
    m_applyButton = new QPushButton("Применить", this);
    buttonLayout->addWidget(m_applyButton);
    
    m_cancelButton = new QPushButton("Отмена", this);
    buttonLayout->addWidget(m_cancelButton);
    
    // ------------------------
    // Добавление элементов в основной Layout
    // ------------------------
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(buttonLayout);
    
    // Установка размеров окна
    resize(500, 400);
}

void SettingsDialog::loadSettings() {
    // Загрузка настроек СОТМ
    const SotmSettings& sotmSettings = m_sotmClient->getSettings();
    m_ipAddressEdit->setText(sotmSettings.ipAddress);
    m_portSpinBox->setValue(sotmSettings.port);
    m_timeoutSpinBox->setValue(sotmSettings.responseTimeoutMs);
    
    // Загрузка настроек мониторинга
    m_pollingIntervalSpinBox->setValue(m_monitoringService->getPollingInterval());
    m_watchdogTimeoutSpinBox->setValue(m_monitoringService->getWatchdogTimeout());
    
    // Загрузка настроек КА и ЗС
    m_kaNumberSpinBox->setValue(sotmSettings.kaNumber);
    m_zsNumberSpinBox->setValue(sotmSettings.zsNumber);
    
    // Загрузка настроек звуков
    for (auto it = m_soundWidgets.begin(); it != m_soundWidgets.end(); ++it) {
        AlertType type = it.key();
        SoundWidgets& widgets = it.value();
        
        widgets.pathEdit->setText(m_alertManager->getAlertSound(type));
        widgets.enabledCheckBox->setChecked(m_alertManager->isAlertEnabled(type));
    }
    
    // Загрузка настроек обновлений
    m_updatePathEdit->setText(m_updateManager->getUpdatePath());
    m_checkAtStartupCheckBox->setChecked(m_updateManager->isCheckAtStartupEnabled());
    m_autoInstallCheckBox->setChecked(m_updateManager->isAutoInstallEnabled());
    m_checkIntervalSpinBox->setValue(m_updateManager->getCheckInterval());
}

void SettingsDialog::saveSettings() {
    // Сохранение настроек СОТМ
    SotmSettings sotmSettings = m_sotmClient->getSettings();
    sotmSettings.ipAddress = m_ipAddressEdit->text();
    sotmSettings.port = m_portSpinBox->value();
    sotmSettings.responseTimeoutMs = m_timeoutSpinBox->value();
    sotmSettings.kaNumber = m_kaNumberSpinBox->value();
    sotmSettings.zsNumber = m_zsNumberSpinBox->value();
    
    // Применение настроек СОТМ
    m_sotmClient->setSettings(sotmSettings);
    emit sotmSettingsChanged(sotmSettings);
    
    // Сохранение настроек мониторинга
    m_monitoringService->setPollingInterval(m_pollingIntervalSpinBox->value());
    m_monitoringService->setWatchdogTimeout(m_watchdogTimeoutSpinBox->value());
    
    // Сохранение настроек звуков
    for (auto it = m_soundWidgets.begin(); it != m_soundWidgets.end(); ++it) {
        AlertType type = it.key();
        SoundWidgets& widgets = it.value();
        
        m_alertManager->setAlertSound(type, widgets.pathEdit->text());
        m_alertManager->setAlertEnabled(type, widgets.enabledCheckBox->isChecked());
        
        emit alertSettingsChanged(type, widgets.pathEdit->text(), widgets.enabledCheckBox->isChecked());
    }
    
    // Сохранение настроек обновлений
    m_updateManager->setUpdatePath(m_updatePathEdit->text());
    m_updateManager->setCheckAtStartupEnabled(m_checkAtStartupCheckBox->isChecked());
    m_updateManager->setAutoInstallEnabled(m_autoInstallCheckBox->isChecked());
    m_updateManager->setCheckInterval(m_checkIntervalSpinBox->value());
    
    emit updateSettingsChanged(m_updatePathEdit->text(),
                             m_checkAtStartupCheckBox->isChecked(),
                             m_autoInstallCheckBox->isChecked(),
                             m_checkIntervalSpinBox->value());
}

void SettingsDialog::updateButtonsStatus() {
    // Отключаем кнопку Применить
    m_applyButton->setEnabled(false);
    
    // Запускаем таймер для обновления состояния кнопки
    QTimer::singleShot(100, this, [this]() {
        // Включаем кнопку Применить
        m_applyButton->setEnabled(true);
    });
}

QGroupBox* SettingsDialog::createSoundGroup(AlertType type, const QString& title, QWidget* parent) {
    // Создание группы
    auto group = new QGroupBox(title, parent);
    auto layout = new QFormLayout(group);
    
    // Создание виджетов
    SoundWidgets widgets;
    
    // Layout для пути и кнопки
    auto pathLayout = new QHBoxLayout();
    
    widgets.pathEdit = new QLineEdit(group);
    pathLayout->addWidget(widgets.pathEdit);
    
    widgets.selectButton = new QPushButton("Выбрать", group);
    pathLayout->addWidget(widgets.selectButton);
    
    layout->addRow("Звуковой файл:", pathLayout);
    
    // Чекбокс
    widgets.enabledCheckBox = new QCheckBox("Включить звуковое оповещение", group);
    layout->addRow("", widgets.enabledCheckBox);
    
    // Добавление в карту
    m_soundWidgets[type] = widgets;
    
    return group;
}

} // namespace ParamControl
