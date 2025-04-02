// Parameter.cpp
#include "Parameter.h"
#include "ParameterEquals.h"
#include "ParameterNotEquals.h"
#include "ParameterInLimits.h"
#include "ParameterOutOfLimits.h"
#include "ParameterChanged.h"

Parameter::Parameter(const QString& name, ParameterType type)
    : m_name(name)
    , m_type(type)
    , m_status(true)
    , m_soundEnabled(true)
{
}

bool Parameter::updateValue(const QVariant& value) {
    // Запоминаем старый статус перед проверкой
    bool oldStatus = m_status;
    
    // Обновляем текущее значение
    m_currentValue = value;
    
    // Проверяем условие
    m_status = checkCondition(value);
    
    // Возвращаем true, если статус изменился
    return oldStatus != m_status;
}

std::shared_ptr<Parameter> Parameter::create(
    const QString& name, 
    ParameterType type, 
    const QVariant& targetValue,
    const QString& soundFile,
    bool soundEnabled) {
    
    std::shared_ptr<Parameter> parameter;
    
    switch (type) {
        case ParameterType::Equals:
            parameter = std::make_shared<ParameterEquals>(name, targetValue);
            break;
        case ParameterType::NotEquals:
            parameter = std::make_shared<ParameterNotEquals>(name, targetValue);
            break;
        case ParameterType::InLimits: {
            // Для этого типа ожидаем пару значений
            QVariantList limits = targetValue.toList();
            if (limits.size() >= 2) {
                parameter = std::make_shared<ParameterInLimits>(
                    name, limits[0], limits[1]);
            }
            break;
        }
        case ParameterType::OutOfLimits: {
            // Для этого типа тоже ожидаем пару значений
            QVariantList limits = targetValue.toList();
            if (limits.size() >= 2) {
                parameter = std::make_shared<ParameterOutOfLimits>(
                    name, limits[0], limits[1]);
            }
            break;
        }
        case ParameterType::Changed:
            parameter = std::make_shared<ParameterChanged>(name);
            break;
    }
    
    // Если создали параметр, установим звук
    if (parameter) {
        parameter->setSoundFile(soundFile);
        parameter->setSoundEnabled(soundEnabled);
    }
    
    return parameter;
}

// ParameterEquals.h
#pragma once

#include "Parameter.h"

class ParameterEquals : public Parameter {
public:
    ParameterEquals(const QString& name, const QVariant& targetValue);
    ~ParameterEquals() = default;
    
    bool checkCondition(const QVariant& value) override;
    QString getDescription() const override;
    QVariant getTargetValue() const override;
    void setTargetValue(const QVariant& value) override;
    
private:
    QVariant m_targetValue;
};

// ParameterEquals.cpp
#include "ParameterEquals.h"

ParameterEquals::ParameterEquals(const QString& name, const QVariant& targetValue)
    : Parameter(name, ParameterType::Equals)
    , m_targetValue(targetValue)
{
}

bool ParameterEquals::checkCondition(const QVariant& value) {
    // Проверка равенства значений
    return value == m_targetValue;
}

QString ParameterEquals::getDescription() const {
    return QString("Равно %1").arg(m_targetValue.toString());
}

QVariant ParameterEquals::getTargetValue() const {
    return m_targetValue;
}

void ParameterEquals::setTargetValue(const QVariant& value) {
    m_targetValue = value;
}

// ParameterNotEquals.h
#pragma once

#include "Parameter.h"

class ParameterNotEquals : public Parameter {
public:
    ParameterNotEquals(const QString& name, const QVariant& targetValue);
    ~ParameterNotEquals() = default;
    
    bool checkCondition(const QVariant& value) override;
    QString getDescription() const override;
    QVariant getTargetValue() const override;
    void setTargetValue(const QVariant& value) override;
    
private:
    QVariant m_targetValue;
};

// ParameterNotEquals.cpp
#include "ParameterNotEquals.h"

ParameterNotEquals::ParameterNotEquals(const QString& name, const QVariant& targetValue)
    : Parameter(name, ParameterType::NotEquals)
    , m_targetValue(targetValue)
{
}

bool ParameterNotEquals::checkCondition(const QVariant& value) {
    // Проверка неравенства значений
    return value != m_targetValue;
}

QString ParameterNotEquals::getDescription() const {
    return QString("Не равно %1").arg(m_targetValue.toString());
}

QVariant ParameterNotEquals::getTargetValue() const {
    return m_targetValue;
}

void ParameterNotEquals::setTargetValue(const QVariant& value) {
    m_targetValue = value;
}

// ParameterInLimits.h
#pragma once

#include "Parameter.h"

class ParameterInLimits : public Parameter {
public:
    ParameterInLimits(const QString& name, const QVariant& lowerLimit, const QVariant& upperLimit);
    ~ParameterInLimits() = default;
    
    bool checkCondition(const QVariant& value) override;
    QString getDescription() const override;
    QVariant getTargetValue() const override;
    void setTargetValue(const QVariant& value) override;
    
    QVariant getLowerLimit() const;
    QVariant getUpperLimit() const;
    void setLimits(const QVariant& lowerLimit, const QVariant& upperLimit);
    
private:
    QVariant m_lowerLimit;
    QVariant m_upperLimit;
};

// ParameterInLimits.cpp
#include "ParameterInLimits.h"

ParameterInLimits::ParameterInLimits(const QString& name, 
                                     const QVariant& lowerLimit, 
                                     const QVariant& upperLimit)
    : Parameter(name, ParameterType::InLimits)
    , m_lowerLimit(lowerLimit)
    , m_upperLimit(upperLimit)
{
}

bool ParameterInLimits::checkCondition(const QVariant& value) {
    bool ok1 = false, ok2 = false;
    double doubleValue = value.toDouble(&ok1);
    
    if (!ok1) {
        // Если не удается преобразовать в число, считаем условие нарушенным
        return false;
    }
    
    double lowerLimitValue = m_lowerLimit.toDouble(&ok1);
    double upperLimitValue = m_upperLimit.toDouble(&ok2);
    
    if (!ok1 || !ok2) {
        // Если с пределами что-то не так, считаем условие нарушенным
        return false;
    }
    
    // Проверка вхождения в диапазон
    return doubleValue >= lowerLimitValue && doubleValue <= upperLimitValue;
}

QString ParameterInLimits::getDescription() const {
    return QString("%1 <= значение <= %2")
        .arg(m_lowerLimit.toString())
        .arg(m_upperLimit.toString());
}

QVariant ParameterInLimits::getTargetValue() const {
    QVariantList limits;
    limits << m_lowerLimit << m_upperLimit;
    return limits;
}

void ParameterInLimits::setTargetValue(const QVariant& value) {
    QVariantList limits = value.toList();
    if (limits.size() >= 2) {
        m_lowerLimit = limits[0];
        m_upperLimit = limits[1];
    }
}

QVariant ParameterInLimits::getLowerLimit() const {
    return m_lowerLimit;
}

QVariant ParameterInLimits::getUpperLimit() const {
    return m_upperLimit;
}

void ParameterInLimits::setLimits(const QVariant& lowerLimit, const QVariant& upperLimit) {
    m_lowerLimit = lowerLimit;
    m_upperLimit = upperLimit;
}

// ParameterOutOfLimits.h
#pragma once

#include "Parameter.h"

class ParameterOutOfLimits : public Parameter {
public:
    ParameterOutOfLimits(const QString& name, const QVariant& lowerLimit, const QVariant& upperLimit);
    ~ParameterOutOfLimits() = default;
    
    bool checkCondition(const QVariant& value) override;
    QString getDescription() const override;
    QVariant getTargetValue() const override;
    void setTargetValue(const QVariant& value) override;
    
    QVariant getLowerLimit() const;
    QVariant getUpperLimit() const;
    void setLimits(const QVariant& lowerLimit, const QVariant& upperLimit);
    
private:
    QVariant m_lowerLimit;
    QVariant m_upperLimit;
};

// ParameterOutOfLimits.cpp
#include "ParameterOutOfLimits.h"

ParameterOutOfLimits::ParameterOutOfLimits(const QString& name, 
                                          const QVariant& lowerLimit, 
                                          const QVariant& upperLimit)
    : Parameter(name, ParameterType::OutOfLimits)
    , m_lowerLimit(lowerLimit)
    , m_upperLimit(upperLimit)
{
}

bool ParameterOutOfLimits::checkCondition(const QVariant& value) {
    bool ok1 = false, ok2 = false;
    double doubleValue = value.toDouble(&ok1);
    
    if (!ok1) {
        // Если не удается преобразовать в число, считаем условие нарушенным
        return false;
    }
    
    double lowerLimitValue = m_lowerLimit.toDouble(&ok1);
    double upperLimitValue = m_upperLimit.toDouble(&ok2);
    
    if (!ok1 || !ok2) {
        // Если с пределами что-то не так, считаем условие нарушенным
        return false;
    }
    
    // Проверка выхода за пределы диапазона
    return doubleValue < lowerLimitValue || doubleValue > upperLimitValue;
}

QString ParameterOutOfLimits::getDescription() const {
    return QString("значение < %1 ИЛИ значение > %2")
        .arg(m_lowerLimit.toString())
        .arg(m_upperLimit.toString());
}

QVariant ParameterOutOfLimits::getTargetValue() const {
    QVariantList limits;
    limits << m_lowerLimit << m_upperLimit;
    return limits;
}

void ParameterOutOfLimits::setTargetValue(const QVariant& value) {
    QVariantList limits = value.toList();
    if (limits.size() >= 2) {
        m_lowerLimit = limits[0];
        m_upperLimit = limits[1];
    }
}

QVariant ParameterOutOfLimits::getLowerLimit() const {
    return m_lowerLimit;
}

QVariant ParameterOutOfLimits::getUpperLimit() const {
    return m_upperLimit;
}

void ParameterOutOfLimits::setLimits(const QVariant& lowerLimit, const QVariant& upperLimit) {
    m_lowerLimit = lowerLimit;
    m_upperLimit = upperLimit;
}

// ParameterChanged.h
#pragma once

#include "Parameter.h"

class ParameterChanged : public Parameter {
public:
    ParameterChanged(const QString& name);
    ~ParameterChanged() = default;
    
    bool checkCondition(const QVariant& value) override;
    QString getDescription() const override;
    QVariant getTargetValue() const override;
    void setTargetValue(const QVariant& value) override;
    
private:
    QVariant m_lastValue;
    bool m_firstCheck;
};

// ParameterChanged.cpp
#include "ParameterChanged.h"

ParameterChanged::ParameterChanged(const QString& name)
    : Parameter(name, ParameterType::Changed)
    , m_firstCheck(true)
{
}

bool ParameterChanged::checkCondition(const QVariant& value) {
    if (m_firstCheck) {
        // Первая проверка, просто запоминаем значение
        m_lastValue = value;
        m_firstCheck = false;
        return true;
    }
    
    // Если значение изменилось, срабатывает условие (возвращаем false)
    bool changed = (value != m_lastValue);
    m_lastValue = value;
    
    // Если значение изменилось, считаем условие нарушенным
    return !changed;
}

QString ParameterChanged::getDescription() const {
    return "Изменение значения";
}

QVariant ParameterChanged::getTargetValue() const {
    // Для этого типа параметра нет целевого значения
    return QVariant();
}

void ParameterChanged::setTargetValue(const QVariant& value) {
    // Для этого типа параметра нет целевого значения
    Q_UNUSED(value);
}
