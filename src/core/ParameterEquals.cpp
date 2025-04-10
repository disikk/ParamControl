// src/core/ParameterEquals.cpp
#include "ParameterEquals.h"

namespace ParamControl {

ParameterEquals::ParameterEquals(const QString& name, const QVariant& targetValue)
    : Parameter(name, ParameterType::Equals)
    , m_targetValue(targetValue)
{
}

bool ParameterEquals::checkCondition(const QVariant& value) {
    // Проверка равенства значений
    // Используем QVariant::compare для более надежного сравнения разных типов
    return QVariant::compare(value, m_targetValue) == 0;
}

QString ParameterEquals::getConditionDescription() const {
    return QString("Равно %1").arg(m_targetValue.toString());
}

QVariant ParameterEquals::getTargetValue() const {
    return m_targetValue;
}

void ParameterEquals::setTargetValue(const QVariant& value) {
    m_targetValue = value;
}

} // namespace ParamControl
