// src/core/ParameterInLimits.h
#pragma once

#include "Parameter.h"

namespace ParamControl {

/**
 * @brief Класс параметра, проверяющий нахождение значения в пределах диапазона.
 */
class ParameterInLimits : public Parameter {
public:
    /**
     * @brief Конструктор.
     * @param name Имя параметра.
     * @param lowerLimit Нижняя граница диапазона.
     * @param upperLimit Верхняя граница диапазона.
     */
    ParameterInLimits(const QString& name, const QVariant& lowerLimit, const QVariant& upperLimit);
    ~ParameterInLimits() override = default;

    /**
     * @brief Проверяет, находится ли значение в заданном диапазоне (включительно).
     * @param value Текущее значение параметра.
     * @return true, если значение в диапазоне, иначе false.
     */
    bool checkCondition(const QVariant& value) override;

    /**
     * @brief Возвращает описание условия контроля.
     * @return Строка с описанием условия.
     */
    QString getConditionDescription() const override;

    /**
     * @brief Возвращает целевое значение (список из двух границ).
     * @return QVariantList с нижней и верхней границами.
     */
    QVariant getTargetValue() const override;

    /**
     * @brief Устанавливает целевое значение (границы диапазона).
     * @param value QVariantList с новой нижней и верхней границами.
     */
    void setTargetValue(const QVariant& value) override;

    /**
     * @brief Возвращает нижнюю границу диапазона.
     * @return Нижняя граница.
     */
    QVariant getLowerLimit() const;

    /**
     * @brief Возвращает верхнюю границу диапазона.
     * @return Верхняя граница.
     */
    QVariant getUpperLimit() const;

    /**
     * @brief Устанавливает границы диапазона.
     * @param lowerLimit Новая нижняя граница.
     * @param upperLimit Новая верхняя граница.
     */
    void setLimits(const QVariant& lowerLimit, const QVariant& upperLimit);

private:
    QVariant m_lowerLimit; ///< Нижняя граница диапазона.
    QVariant m_upperLimit; ///< Верхняя граница диапазона.
};

} // namespace ParamControl
