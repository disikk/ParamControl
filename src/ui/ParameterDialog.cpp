#include "ParameterDialog.h"
#include "ui_ParameterDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVariantList>

ParameterDialog::ParameterDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ParameterDialog)
{
    ui->setupUi(this);
    
    // Настраиваем интерфейс
    setupUi();
    
    // Устанавливаем соединения сигналов и слотов
    setupConnections();
    
    // Обновляем состояние элементов управления
    updateControlsState();
}

ParameterDialog::~ParameterDialog() {
    delete ui;
}

void ParameterDialog::setupUi() {
    // Настраиваем группы радиокнопок
    m_equalsRadio = ui->equalsRadio;
    m_notEqualsRadio = ui->notEqualsRadio;
    m_inLimitsRadio = ui->inLimitsRadio;
    m_outOfLimitsRadio = ui->outOfLimitsRadio;
    m_changedRadio = ui->changedRadio;
    
    // Настраиваем поля ввода
    m_parameterNameEdit = ui->parameterNameEdit;
    m_equalsValueEdit = ui->equalsValueEdit;
    m_notEqualsValueEdit = ui->notEqualsValueEdit;
    m_lowerLimitEdit = ui->lowerLimitEdit;
    m_upperLimitEdit = ui->upperLimitEdit;
    m_lowerOutLimitEdit = ui->lowerOutLimitEdit;
    m_upperOutLimitEdit = ui->upperOutLimitEdit;
    
    // Настраиваем поля для звука
    m_soundGroupBox = ui->soundGroupBox;
    m_soundFileEdit = ui->soundFileEdit;
    m_soundFileBrowseButton = ui->soundFileBrowseButton;
    m_enableSoundCheckBox = ui->enableSoundCheckBox;
    
    // Устанавливаем значения по умолчанию
    m_enableSoundCheckBox->setChecked(true);
    m_soundFileEdit->setText("./Trevoga.wav");
    
    // Выбираем тип параметра по умолчанию
    m_equalsRadio->setChecked(true);
}

void ParameterDialog::setupConnections() {
    // Подключаем изменение типа параметра
    connect(m_equalsRadio, &QRadioButton::toggled,
            this, &ParameterDialog::onParameterTypeChanged);
    connect(m_notEqualsRadio, &QRadioButton::toggled,
            this, &ParameterDialog::onParameterTypeChanged);
    connect(m_inLimitsRadio, &QRadioButton::toggled,
            this, &ParameterDialog::onParameterTypeChanged);
    connect(m_outOfLimitsRadio, &QRadioButton::toggled,
            this, &ParameterDialog::onParameterTypeChanged);
    connect(m_changedRadio, &QRadioButton::toggled,
            this, &ParameterDialog::onParameterTypeChanged);
    
    // Подключаем выбор звукового файла
    connect(m_soundFileBrowseButton, &QPushButton::clicked,
            this, &ParameterDialog::onSelectSoundFileClicked);
    
    // Подключаем кнопки диалога
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ParameterDialog::onAcceptClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
}

void ParameterDialog::onParameterTypeChanged() {
    updateControlsState();
}

void ParameterDialog::updateControlsState() {
    // Обновляем состояние полей в зависимости от выбранного типа параметра
    
    // Поля для Equals
    m_equalsValueEdit->setEnabled(m_equalsRadio->isChecked());
    
    // Поля для NotEquals
    m_notEqualsValueEdit->setEnabled(m_notEqualsRadio->isChecked());
    
    // Поля для InLimits
    m_lowerLimitEdit->setEnabled(m_inLimitsRadio->isChecked());
    m_upperLimitEdit->setEnabled(m_inLimitsRadio->isChecked());
    
    // Поля для OutOfLimits
    m_lowerOutLimitEdit->setEnabled(m_outOfLimitsRadio->isChecked());
    m_upperOutLimitEdit->setEnabled(m_outOfLimitsRadio->isChecked());
    
    // Статусы Label
    ui->labelLowerLimit->setEnabled(m_inLimitsRadio->isChecked());
    ui->labelUpperLimit->setEnabled(m_inLimitsRadio->isChecked());
    ui->labelLowerOutLimit->setEnabled(m_outOfLimitsRadio->isChecked());
    ui->labelUpperOutLimit->setEnabled(m_outOfLimitsRadio->isChecked());
}

void ParameterDialog::onSelectSoundFileClicked() {
    QString file = QFileDialog::getOpenFileName(this, "Выбрать звуковой файл",
                                              QString(), "WAV Files (*.wav)");
    if (!file.isEmpty()) {
        m_soundFileEdit->setText(file);
    }
}

void ParameterDialog::onAcceptClicked() {
    // Проверяем правильность введенных данных
    if (!validateInputs()) {
        return;
    }
    
    // Закрываем диалог с кодом QDialog::Accepted
    accept();
}

bool ParameterDialog::validateInputs() {
    // Проверяем, что имя параметра задано
    if (m_parameterNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не заполнено поле названия параметра.");
        return false;
    }
    
    // Проверяем поля в зависимости от типа параметра
    if (m_equalsRadio->isChecked()) {
        if (m_equalsValueEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Не заполнено поле значения параметра.");
            return false;
        }
    } else if (m_notEqualsRadio->isChecked()) {
        if (m_notEqualsValueEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Не заполнено поле значения параметра.");
            return false;
        }
    } else if (m_inLimitsRadio->isChecked()) {
        if (m_lowerLimitEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Не заполнено поле нижней границы.");
            return false;
        }
        if (m_upperLimitEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Не заполнено поле верхней границы.");
            return false;
        }
        
        // Проверяем, что нижняя граница меньше верхней
        bool lowerOk, upperOk;
        double lowerLimit = m_lowerLimitEdit->text().toDouble(&lowerOk);
        double upperLimit = m_upperLimitEdit->text().toDouble(&upperOk);
        
        if (!lowerOk || !upperOk) {
            QMessageBox::warning(this, "Ошибка", "Неверный формат чисел. Используйте точку в качестве разделителя дробной части.");
            return false;
        }
        
        if (lowerLimit >= upperLimit) {
            QMessageBox::warning(this, "Ошибка", "Нижняя граница должна быть меньше верхней.");
            return false;
        }
    } else if (m_outOfLimitsRadio->isChecked()) {
        if (m_lowerOutLimitEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Не заполнено поле нижней границы.");
            return false;
        }
        if (m_upperOutLimitEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Не заполнено поле верхней границы.");
            return false;
        }
        
        // Проверяем, что нижняя граница меньше верхней
        bool lowerOk, upperOk;
        double lowerLimit = m_lowerOutLimitEdit->text().toDouble(&lowerOk);
        double upperLimit = m_upperOutLimitEdit->text().toDouble(&upperOk);
        
        if (!lowerOk || !upperOk) {
            QMessageBox::warning(this, "Ошибка", "Неверный формат чисел. Используйте точку в качестве разделителя дробной части.");
            return false;
        }
        
        if (lowerLimit >= upperLimit) {
            QMessageBox::warning(this, "Ошибка", "Нижняя граница должна быть меньше верхней.");
            return false;
        }
    }
    
    // Проверяем звуковой файл, если включен звук
    if (m_enableSoundCheckBox->isChecked() && m_soundFileEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Требуется указать звуковой файл для параметра.");
        return false;
    }
    
    return true;
}

std::shared_ptr<Parameter> ParameterDialog::getParameter() const {
    // Создаем параметр нужного типа на основе введенных данных
    std::shared_ptr<Parameter> parameter;
    
    // Определяем тип параметра и создаем соответствующий объект
    ParameterType type;
    QVariant targetValue;
    
    if (m_equalsRadio->isChecked()) {
        type = ParameterType::Equals;
        targetValue = m_equalsValueEdit->text();
    } else if (m_notEqualsRadio->isChecked()) {
        type = ParameterType::NotEquals;
        targetValue = m_notEqualsValueEdit->text();
    } else if (m_inLimitsRadio->isChecked()) {
        type = ParameterType::InLimits;
        QVariantList limits;
        limits.append(m_lowerLimitEdit->text());
        limits.append(m_upperLimitEdit->text());
        targetValue = limits;
    } else if (m_outOfLimitsRadio->isChecked()) {
        type = ParameterType::OutOfLimits;
        QVariantList limits;
        limits.append(m_lowerOutLimitEdit->text());
        limits.append(m_upperOutLimitEdit->text());
        targetValue = limits;
    } else if (m_changedRadio->isChecked()) {
        type = ParameterType::Changed;
    }
    
    // Создаем параметр
    parameter = Parameter::create(
        m_parameterNameEdit->text(),
        type,
        targetValue,
        m_soundFileEdit->text(),
        m_enableSoundCheckBox->isChecked());
    
    return parameter;
}

void ParameterDialog::setParameter(const std::shared_ptr<Parameter>& parameter) {
    if (!parameter) {
        return;
    }
    
    // Устанавливаем имя параметра
    m_parameterNameEdit->setText(parameter->getName());
    
    // Устанавливаем настройки звука
    m_enableSoundCheckBox->setChecked(parameter->isSoundEnabled());
    m_soundFileEdit->setText(parameter->getSoundFile());
    
    // Устанавливаем тип параметра и соответствующие значения
    ParameterType type = parameter->getType();
    QVariant targetValue = parameter->getTargetValue();
    
    switch (type) {
        case ParameterType::Equals:
            m_equalsRadio->setChecked(true);
            m_equalsValueEdit->setText(targetValue.toString());
            break;
        case ParameterType::NotEquals:
            m_notEqualsRadio->setChecked(true);
            m_notEqualsValueEdit->setText(targetValue.toString());
            break;
        case ParameterType::InLimits: {
            m_inLimitsRadio->setChecked(true);
            QVariantList limits = targetValue.toList();
            if (limits.size() >= 2) {
                m_lowerLimitEdit->setText(limits[0].toString());
                m_upperLimitEdit->setText(limits[1].toString());
            }
            break;
        }
        case ParameterType::OutOfLimits: {
            m_outOfLimitsRadio->setChecked(true);
            QVariantList limits = targetValue.toList();
            if (limits.size() >= 2) {
                m_lowerOutLimitEdit->setText(limits[0].toString());
                m_upperOutLimitEdit->setText(limits[1].toString());
            }
            break;
        }
        case ParameterType::Changed:
            m_changedRadio->setChecked(true);
            break;
        default:
            break;
    }
    
    // Обновляем состояние элементов управления
    updateControlsState();
}
