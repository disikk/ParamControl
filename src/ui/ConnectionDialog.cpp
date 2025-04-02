#include "ConnectionDialog.h"
#include "ui_ConnectionDialog.h"

#include <QMessageBox>
#include <QHostAddress>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace ParamControl {

ConnectionDialog::ConnectionDialog(const SotmSettings& settings, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ConnectionDialog)
    , m_settings(settings)
{
    ui->setupUi(this);
    
    // Устанавливаем значения из настроек
    ui->ipAddressLineEdit->setText(settings.ipAddress);
    ui->portSpinBox->setValue(settings.port);
    ui->timeoutSpinBox->setValue(settings.responseTimeoutMs);
    
    // Устанавливаем валидатор для IP-адреса
    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    ui->ipAddressLineEdit->setValidator(new QRegularExpressionValidator(ipRegex, this));
    
    // Подключаем сигналы кнопок
    connect(ui->okButton, &QPushButton::clicked, this, &ConnectionDialog::onOkClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ConnectionDialog::onCancelClicked);
    connect(ui->testButton, &QPushButton::clicked, this, &ConnectionDialog::onTestConnectionClicked);
    
    // Настройка окна
    setWindowTitle("Настройки подключения к СОТМ");
    setFixedSize(sizeHint());
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

SotmSettings ConnectionDialog::getSettings() const
{
    return m_settings;
}

void ConnectionDialog::onOkClicked()
{
    if (validateInputs()) {
        updateSettings();
        accept();
    }
}

void ConnectionDialog::onCancelClicked()
{
    reject();
}

void ConnectionDialog::onTestConnectionClicked()
{
    if (validateInputs()) {
        // Создаем временные настройки
        SotmSettings testSettings;
        testSettings.ipAddress = ui->ipAddressLineEdit->text();
        testSettings.port = ui->portSpinBox->value();
        testSettings.responseTimeoutMs = ui->timeoutSpinBox->value();
        
        // Пытаемся подключиться
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        // Создаем временный клиент
        SotmClient testClient;
        bool success = testClient.connect(testSettings);
        
        QApplication::restoreOverrideCursor();
        
        // Показываем результат
        if (success) {
            QMessageBox::information(this, "Тест соединения", 
                                    "Соединение с СОТМ успешно установлено.");
            testClient.disconnect();
        } else {
            QMessageBox::warning(this, "Тест соединения", 
                               "Не удалось установить соединение с СОТМ. Проверьте настройки.");
        }
    }
}

bool ConnectionDialog::validateInputs()
{
    // Проверка IP-адреса
    QString ipAddress = ui->ipAddressLineEdit->text();
    QHostAddress address(ipAddress);
    
    if (address.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Неверный формат IP-адреса.");
        ui->ipAddressLineEdit->setFocus();
        return false;
    }
    
    // Проверка порта
    int port = ui->portSpinBox->value();
    if (port <= 0 || port > 65535) {
        QMessageBox::warning(this, "Ошибка", "Номер порта должен быть в диапазоне от 1 до 65535.");
        ui->portSpinBox->setFocus();
        return false;
    }
    
    // Проверка таймаута
    int timeout = ui->timeoutSpinBox->value();
    if (timeout < 1000) {
        QMessageBox::warning(this, "Ошибка", "Таймаут должен быть не менее 1000 мс.");
        ui->timeoutSpinBox->setFocus();
        return false;
    }
    
    return true;
}

void ConnectionDialog::updateSettings()
{
    m_settings.ipAddress = ui->ipAddressLineEdit->text();
    m_settings.port = ui->portSpinBox->value();
    m_settings.responseTimeoutMs = ui->timeoutSpinBox->value();
}

} // namespace ParamControl
