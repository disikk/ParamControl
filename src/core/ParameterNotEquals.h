// src/core/ParameterNotEquals.h
#pragma once

#include "Parameter.h"

namespace ParamControl {

/**
 * @brief Класс параметра, проверяющий неравенство значения.
 */
class ParameterNotEquals : public Parameter {
public:
    /**
     * @brief Конструктор.
     * @param name Имя параметра.
     * @param targetValue Целевое значение для сравнения.
     */
    ParameterNotEquals(const QString& name, const QVariant& targetValue);
    ~ParameterNotEquals() override = default;

    /**
     * @brief Проверяет, не равно ли текущее значение целевому.
     * @param value Текущее значение параметра.
     * @return true, если значение не равно целевому, иначе false.
     */
    bool checkCondition(const QVariant& value) override;

    /**
     * @brief Возвращает описание условия контроля.
     * @return Строка с описанием условия.
     */
    QString getConditionDescription() const override;

    /**
     * @brief Возвращает целевое значение.
     * @return Целевое значение.
     */
    QVariant getTargetValue() const override;

    /**
     * @brief Устанавливает целевое значение.
     * @param value Новое целевое значение.
     */
    void setTargetValue(const QVariant& value) override;

private:
    QVariant m_targetValue; ///< Целевое значение для сравнения.
};

} // namespace ParamControl
