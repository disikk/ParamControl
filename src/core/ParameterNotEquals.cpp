// src/core/ParameterNotEquals.cpp
#include "ParameterNotEquals.h"

namespace ParamControl {

ParameterNotEquals::ParameterNotEquals(const QString& name, const QVariant& targetValue)
    : Parameter(name, ParameterType::NotEquals)
    , m_targetValue(targetValue)
{
}

bool ParameterNotEquals::checkCondition(const QVariant& value) {
    // Проверка неравенства значений
    // Используем QVariant::compare для более надежного сравнения разных типов
    return QVariant::compare(value, m_targetValue) != 0;
}

QString ParameterNotEquals::getConditionDescription() const {
    return QString("Не равно %1").arg(m_targetValue.toString());
}

QVariant ParameterNotEquals::getTargetValue() const {
    return m_targetValue;
}

void ParameterNotEquals::setTargetValue(const QVariant& value) {
    m_targetValue = value;
}

} // namespace ParamControl
