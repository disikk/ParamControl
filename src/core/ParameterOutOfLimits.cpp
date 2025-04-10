// src/core/ParameterOutOfLimits.cpp
#include "ParameterOutOfLimits.h"
#include <QDebug> // Для возможной отладки

namespace ParamControl {

ParameterOutOfLimits::ParameterOutOfLimits(const QString& name,
                                          const QVariant& lowerLimit,
                                          const QVariant& upperLimit)
    : Parameter(name, ParameterType::OutOfLimits)
    , m_lowerLimit(lowerLimit)
    , m_upperLimit(upperLimit)
{
}

bool ParameterOutOfLimits::checkCondition(const QVariant& value) {
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
        // qDebug() << "ParameterOutOfLimits: Failed to convert values to double for parameter" << getName();
        return false; // Условие "вне диапазона" не выполнено, если данные некорректны
    }

    // Проверка выхода за пределы диапазона
    return doubleValue < lowerLimitValue || doubleValue > upperLimitValue;
}

QString ParameterOutOfLimits::getConditionDescription() const {
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
                 qWarning() << "ParameterOutOfLimits: Invalid limit types provided for parameter" << getName();
            }
        } else {
             qWarning() << "ParameterOutOfLimits: Incorrect number of limits provided for parameter" << getName();
        }
    } else {
         qWarning() << "ParameterOutOfLimits: Invalid target value type provided for parameter" << getName();
    }
}

QVariant ParameterOutOfLimits::getLowerLimit() const {
    return m_lowerLimit;
}

QVariant ParameterOutOfLimits::getUpperLimit() const {
    return m_upperLimit;
}

void ParameterOutOfLimits::setLimits(const QVariant& lowerLimit, const QVariant& upperLimit) {
    // Дополнительно можно проверить, что границы можно преобразовать в числа
    bool lowerOk, upperOk;
    lowerLimit.toDouble(&lowerOk);
    upperLimit.toDouble(&upperOk);
     if (lowerOk && upperOk) {
        m_lowerLimit = lowerLimit;
        m_upperLimit = upperLimit;
     } else {
        qWarning() << "ParameterOutOfLimits: Invalid limit types provided during setLimits for parameter" << getName();
     }
}

} // namespace ParamControl
