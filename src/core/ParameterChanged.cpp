// src/core/ParameterChanged.cpp
#include "ParameterChanged.h"

namespace ParamControl {

ParameterChanged::ParameterChanged(const QString& name)
    : Parameter(name, ParameterType::Changed)
    , m_firstCheck(true)
    , m_justChanged(false) // Изначально значение не менялось
{
    // Для типа Changed начальный статус должен быть Ok (в норме)
    m_status = ParameterStatus::Ok;
}

bool ParameterChanged::checkCondition(const QVariant& value) {
    if (m_firstCheck) {
        // Первая проверка, просто запоминаем значение
        m_lastValue = value;
        m_firstCheck = false;
        m_justChanged = false; // Считаем, что не менялось
        return true; // Условие "не изменилось" выполнено
    }

    // Сравниваем текущее значение с последним
    // Используем QVariant::compare для надежности
    bool changed = (QVariant::compare(value, m_lastValue) != 0);

    if (changed) {
        m_justChanged = true; // Запоминаем, что только что изменилось
    } else {
        m_justChanged = false; // Стабилизировалось
    }

    // Запоминаем текущее значение как последнее
    m_lastValue = value;

    // Условие checkCondition возвращает true, если параметр В НОРМЕ.
    // Для типа Changed норма - это когда значение НЕ МЕНЯЕТСЯ.
    // Поэтому возвращаем !changed.
    return !changed;
}

QString ParameterChanged::getConditionDescription() const {
    return "Изменение значения";
}

QVariant ParameterChanged::getTargetValue() const {
    // Для этого типа параметра нет целевого значения
    return QVariant();
}

void ParameterChanged::setTargetValue(const QVariant& value) {
    // Для этого типа параметра нет целевого значения
    Q_UNUSED(value);
    // Сбрасываем состояние при попытке установить значение (например, при редактировании)
    m_firstCheck = true;
    m_justChanged = false;
    m_status = ParameterStatus::Ok;
}

bool ParameterChanged::updateValue(const QVariant& value) {
    // Запоминаем старый статус перед проверкой
    ParameterStatus oldStatus = m_status;

    // Обновляем текущее значение
    m_currentValue = value;

    // Проверяем условие (checkCondition вернет true, если НЕ изменилось)
    bool conditionMet = checkCondition(value); // true = не изменилось, false = изменилось

    // Обновляем статус: Error если изменилось, Ok если не изменилось
    m_status = conditionMet ? ParameterStatus::Ok : ParameterStatus::Error;

    // Возвращаем true, если статус изменился
    return oldStatus != m_status;
}


} // namespace ParamControl
