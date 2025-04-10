// src/core/ParameterInLimits.cpp
#include "ParameterInLimits.h"
#include <QDebug> // Для возможной отладки

namespace ParamControl {

ParameterInLimits::ParameterInLimits(const QString& name,
                                     const QVariant& lowerLimit,
                                     const QVariant& upperLimit)
    : Parameter(name, ParameterType::InLimits)
    , m_lowerLimit(lowerLimit)
    , m_upperLimit(upperLimit)
{
}

bool ParameterInLimits::checkCondition(const QVariant& value) {
    bool valueOk = false;
    bool lowerOk = false;
    bool upperOk = false;

    // Пытаемся преобразовать все значения к double для сравнения
    double doubleValue = value.toDouble(&valueOk);
    double lowerLimitValue = m_lowerLimit.toDouble(&lowerOk);
    double upperLimitValue = m_upperLimit.toDouble(&upperOk);

    // Если какое-либо из преобразований не удалось, считаем условие нарушенным
    if (!valueOk || !lowerOk || !upperOk) {
        // Можно добавить логирование для отладки некорректных данных
        // qDebug() << "ParameterInLimits: Failed to convert values to double for parameter" << getName();
        return false;
    }

    // Проверка вхождения в диапазон (включительно)
    return doubleValue >= lowerLimitValue && doubleValue <= upperLimitValue;
}

QString ParameterInLimits::getConditionDescription() const {
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
    if (value.type() == QVariant::List) {
        QVariantList limits = value.toList();
        if (limits.size() >= 2) {
            // Дополнительно можно проверить, что границы можно преобразовать в числа
            bool lowerOk, upperOk;
            limits[0].toDouble(&lowerOk);
            limits[1].toDouble(&upperOk);
            if (lowerOk && upperOk) {
                 m_lowerLimit = limits[0];
                 m_upperLimit = limits[1];
            } else {
                 qWarning() << "ParameterInLimits: Invalid limit types provided for parameter" << getName();
            }
        } else {
             qWarning() << "ParameterInLimits: Incorrect number of limits provided for parameter" << getName();
        }
    } else {
         qWarning() << "ParameterInLimits: Invalid target value type provided for parameter" << getName();
    }
}

QVariant ParameterInLimits::getLowerLimit() const {
    return m_lowerLimit;
}

QVariant ParameterInLimits::getUpperLimit() const {
    return m_upperLimit;
}

void ParameterInLimits::setLimits(const QVariant& lowerLimit, const QVariant& upperLimit) {
    // Дополнительно можно проверить, что границы можно преобразовать в числа
    bool lowerOk, upperOk;
    lowerLimit.toDouble(&lowerOk);
    upperLimit.toDouble(&upperOk);
     if (lowerOk && upperOk) {
        m_lowerLimit = lowerLimit;
        m_upperLimit = upperLimit;
     } else {
        qWarning() << "ParameterInLimits: Invalid limit types provided during setLimits for parameter" << getName();
     }
}

} // namespace ParamControl
