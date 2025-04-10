// src/core/Parameter.cpp
#include "Parameter.h"
#include "ParameterEquals.h"
#include "ParameterNotEquals.h"
#include "ParameterInLimits.h"
#include "ParameterOutOfLimits.h"
#include "ParameterChanged.h"

#include <QVariantList>
#include <QDebug> // Для логирования ошибок

namespace ParamControl {

Parameter::Parameter(const QString& name, ParameterType type)
    : m_name(name)
    , m_type(type)
    , m_status(ParameterStatus::Unknown) // Начальный статус - неизвестно
    , m_soundEnabled(true)
    , m_description("") // Инициализируем описание пустой строкой
{
}

Parameter::~Parameter() = default; // Виртуальный деструктор

bool Parameter::updateValue(const QVariant& value) {
    // Запоминаем старый статус перед проверкой
    ParameterStatus oldStatus = m_status;

    // Обновляем текущее значение
    m_currentValue = value;

    // Проверяем условие
    // checkCondition возвращает true, если параметр В НОРМЕ (условие выполнено)
    bool conditionMet = checkCondition(value);

    // Обновляем статус
    m_status = conditionMet ? ParameterStatus::Ok : ParameterStatus::Error;

    // Возвращаем true, если статус изменился
    return oldStatus != m_status;
}

// Фабричный метод для создания параметров
std::shared_ptr<Parameter> Parameter::create(
    const QString& name,
    ParameterType type,
    const QVariant& targetValue,
    const QString& soundFile,
    bool soundEnabled,
    const QString& description) // Добавлен параметр описания
{
    std::shared_ptr<Parameter> parameter;

    try {
        switch (type) {
            case ParameterType::Equals:
                parameter = std::make_shared<ParameterEquals>(name, targetValue);
                break;
            case ParameterType::NotEquals:
                parameter = std::make_shared<ParameterNotEquals>(name, targetValue);
                break;
            case ParameterType::InLimits: {
                if (targetValue.type() == QVariant::List) {
                    QVariantList limits = targetValue.toList();
                    if (limits.size() >= 2) {
                        parameter = std::make_shared<ParameterInLimits>(
                            name, limits[0], limits[1]);
                    } else {
                         qWarning() << "Parameter::create: Incorrect number of limits for InLimits parameter" << name;
                    }
                } else {
                     qWarning() << "Parameter::create: Invalid target value type for InLimits parameter" << name;
                }
                break;
            }
            case ParameterType::OutOfLimits: {
                 if (targetValue.type() == QVariant::List) {
                    QVariantList limits = targetValue.toList();
                    if (limits.size() >= 2) {
                        parameter = std::make_shared<ParameterOutOfLimits>(
                            name, limits[0], limits[1]);
                    } else {
                         qWarning() << "Parameter::create: Incorrect number of limits for OutOfLimits parameter" << name;
                    }
                } else {
                     qWarning() << "Parameter::create: Invalid target value type for OutOfLimits parameter" << name;
                }
                break;
            }
            case ParameterType::Changed:
                parameter = std::make_shared<ParameterChanged>(name);
                break;
            default:
                 qWarning() << "Parameter::create: Unknown parameter type for" << name;
                 break; // Не создаем параметр неизвестного типа
        }
    } catch (const std::bad_alloc& e) {
        qCritical() << "Parameter::create: Memory allocation failed -" << e.what();
        parameter = nullptr; // Убедимся, что возвращаем nullptr при ошибке выделения памяти
    } catch (...) {
         qCritical() << "Parameter::create: Unknown exception during parameter creation for" << name;
         parameter = nullptr;
    }

    // Если параметр успешно создан, устанавливаем общие свойства
    if (parameter) {
        parameter->setSoundFile(soundFile);
        parameter->setSoundEnabled(soundEnabled);
        parameter->setDescription(description); // Устанавливаем описание
    }

    return parameter;
}

// --- Геттеры и сеттеры ---

QString Parameter::getName() const {
    return m_name;
}

ParameterType Parameter::getType() const {
    return m_type;
}

ParameterStatus Parameter::getStatus() const {
    return m_status;
}

QVariant Parameter::getCurrentValue() const {
    return m_currentValue;
}

bool Parameter::isSoundEnabled() const {
    return m_soundEnabled;
}

void Parameter::setSoundEnabled(bool enabled) {
    m_soundEnabled = enabled;
}

QString Parameter::getSoundFile() const {
    return m_soundFile;
}

void Parameter::setSoundFile(const QString& soundFile) {
    m_soundFile = soundFile;
}

QString Parameter::getDescription() const {
    return m_description;
}

void Parameter::setDescription(const QString& description) {
    m_description = description;
}


} // namespace ParamControl
