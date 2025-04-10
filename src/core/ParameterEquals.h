// src/core/ParameterEquals.h
#pragma once

#include "Parameter.h"

namespace ParamControl {

/**
 * @brief Класс параметра, проверяющий равенство значения.
 */
class ParameterEquals : public Parameter {
public:
    /**
     * @brief Конструктор.
     * @param name Имя параметра.
     * @param targetValue Целевое значение для сравнения.
     */
    ParameterEquals(const QString& name, const QVariant& targetValue);
    ~ParameterEquals() override = default;

    /**
     * @brief Проверяет, равно ли текущее значение целевому.
     * @param value Текущее значение параметра.
     * @return true, если значение равно целевому, иначе false.
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
