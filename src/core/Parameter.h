// src/core/Parameter.h
#pragma once

#include <QObject> // Для Q_GADGET, если потребуется
#include <QString>
#include <QVariant>
#include <memory> // Для std::shared_ptr

namespace ParamControl {

/**
 * @brief Типы условий контроля параметров
 */
enum class ParameterType {
    Equals,         ///< Равенство заданному значению
    NotEquals,      ///< Неравенство заданному значению
    InLimits,       ///< Нахождение в пределах диапазона [min, max]
    OutOfLimits,    ///< Выход за пределы диапазона [min, max]
    Changed         ///< Изменение значения по сравнению с предыдущим
};
Q_ENUM(ParameterType) // Позволяет использовать enum в метаобъектной системе Qt

/**
 * @brief Статус параметра
 */
enum class ParameterStatus {
    Ok,             ///< Условие выполняется (параметр в норме)
    Error,          ///< Условие нарушено (параметр не в норме)
    Unknown         ///< Статус не определен (например, до первой проверки)
};
Q_ENUM(ParameterStatus)

// Прямое объявление, чтобы избежать циклической зависимости, если ParameterModel включает Parameter.h
class ParameterModel;

/**
 * @brief Базовый класс для представления контролируемого параметра.
 *
 * Этот класс является абстрактным и определяет общий интерфейс
 * для всех типов контролируемых параметров.
 */
class Parameter {
    // Q_GADGET // Можно использовать, если нужны метаобъектные возможности без наследования от QObject

public:
    /**
     * @brief Конструктор.
     * @param name Имя параметра.
     * @param type Тип условия контроля.
     */
    Parameter(const QString& name, ParameterType type);

    /**
     * @brief Виртуальный деструктор.
     */
    virtual ~Parameter();

    // Запрещаем копирование и присваивание для избежания проблем с владением
    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
    Parameter(Parameter&&) = default; // Разрешаем перемещение
    Parameter& operator=(Parameter&&) = default;

    /**
     * @brief Обновляет текущее значение параметра и проверяет условие.
     * @param value Новое значение параметра.
     * @return true, если статус параметра изменился, иначе false.
     */
    virtual bool updateValue(const QVariant& value);

    /**
     * @brief Чисто виртуальный метод для проверки условия контроля.
     *
     * Производные классы должны реализовать этот метод для своей логики проверки.
     * Метод должен возвращать true, если параметр В НОРМЕ (условие выполнено),
     * и false, если параметр НЕ В НОРМЕ (условие нарушено).
     *
     * @param value Текущее значение параметра для проверки.
     * @return true, если условие выполнено (параметр в норме), иначе false.
     */
    virtual bool checkCondition(const QVariant& value) = 0;

    /**
     * @brief Возвращает текстовое описание условия контроля.
     * @return Строка с описанием условия.
     */
    virtual QString getConditionDescription() const = 0;

    /**
     * @brief Возвращает целевое значение (или значения), используемое для проверки.
     *
     * Формат зависит от типа параметра (одно значение, список границ и т.д.).
     * @return Целевое значение(я).
     */
    virtual QVariant getTargetValue() const = 0;

    /**
     * @brief Устанавливает целевое значение (или значения).
     * @param value Новое целевое значение(я).
     */
    virtual void setTargetValue(const QVariant& value) = 0;

    /**
     * @brief Фабричный метод для создания экземпляров параметров.
     * @param name Имя параметра.
     * @param type Тип условия контроля.
     * @param targetValue Целевое значение(я).
     * @param soundFile Путь к звуковому файлу оповещения.
     * @param soundEnabled Флаг включения звукового оповещения.
     * @param description Описание параметра (опционально).
     * @return Умный указатель на созданный параметр или nullptr в случае ошибки.
     */
    static std::shared_ptr<Parameter> create(
        const QString& name,
        ParameterType type,
        const QVariant& targetValue,
        const QString& soundFile = QString(),
        bool soundEnabled = true,
        const QString& description = QString());

    // --- Геттеры и сеттеры для общих свойств ---

    QString getName() const;
    ParameterType getType() const;
    ParameterStatus getStatus() const;
    QVariant getCurrentValue() const;

    bool isSoundEnabled() const;
    void setSoundEnabled(bool enabled);

    QString getSoundFile() const;
    void setSoundFile(const QString& soundFile);

    QString getDescription() const;
    void setDescription(const QString& description);

protected:
    QString m_name;                 ///< Имя параметра.
    ParameterType m_type;           ///< Тип условия контроля.
    ParameterStatus m_status;       ///< Текущий статус параметра.
    QVariant m_currentValue;        ///< Последнее полученное значение параметра.
    bool m_soundEnabled;            ///< Флаг включения звукового оповещения.
    QString m_soundFile;            ///< Путь к звуковому файлу оповещения.
    QString m_description;          ///< Описание параметра.
};

} // namespace ParamControl
