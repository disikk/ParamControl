// src/core/ParameterChanged.h
#pragma once

#include "Parameter.h"

namespace ParamControl {

/**
 * @brief Класс параметра, отслеживающий изменение значения.
 *
 * Условие считается нарушенным (status = Error), когда значение изменяется.
 * Возвращается в норму (status = Ok), когда значение стабилизируется (не меняется).
 */
class ParameterChanged : public Parameter {
public:
    /**
     * @brief Конструктор.
     * @param name Имя параметра.
     */
    ParameterChanged(const QString& name);
    ~ParameterChanged() override = default;

    /**
     * @brief Проверяет, изменилось ли значение по сравнению с предыдущим.
     * @param value Текущее значение параметра.
     * @return true, если значение НЕ изменилось, иначе false.
     */
    bool checkCondition(const QVariant& value) override;

    /**
     * @brief Возвращает описание условия контроля.
     * @return Строка с описанием условия.
     */
    QString getConditionDescription() const override;

    /**
     * @brief Возвращает целевое значение (всегда QVariant()).
     * @return Пустой QVariant.
     */
    QVariant getTargetValue() const override;

    /**
     * @brief Устанавливает целевое значение (игнорируется).
     * @param value Новое целевое значение (не используется).
     */
    void setTargetValue(const QVariant& value) override;

    /**
     * @brief Переопределенный метод обновления значения для сброса состояния.
     * @param value Новое значение параметра.
     * @return true, если статус изменился, иначе false.
     */
    bool updateValue(const QVariant& value) override;


private:
    QVariant m_lastValue;  ///< Последнее зафиксированное значение.
    bool m_firstCheck;     ///< Флаг первой проверки после создания или сброса.
    bool m_justChanged;    ///< Флаг, указывающий, что значение только что изменилось.
};

} // namespace ParamControl
