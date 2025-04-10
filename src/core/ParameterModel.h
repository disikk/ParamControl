// src/core/ParameterModel.h
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QSet>
#include <QMap>
#include <memory> // Для std::shared_ptr
#include <mutex>  // Для std::mutex
#include <atomic> // Для std::atomic (если потребуется)

#include "Parameter.h" // Включаем базовый класс Parameter

// Прямое объявление не нужно, т.к. Parameter.h уже включен

namespace ParamControl {

/**
 * @brief Модель данных для хранения и управления контролируемыми параметрами.
 *
 * Этот класс обеспечивает централизованное хранилище для всех параметров,
 * предоставляет методы для добавления, удаления, получения и обновления параметров,
 * а также для их сохранения и загрузки из файла. Класс потокобезопасен.
 */
class ParameterModel : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский объект Qt.
     */
    explicit ParameterModel(QObject* parent = nullptr);

    /**
     * @brief Деструктор.
     */
    ~ParameterModel() override;

    // Запрещаем копирование и присваивание
    ParameterModel(const ParameterModel&) = delete;
    ParameterModel& operator=(const ParameterModel&) = delete;
    ParameterModel(ParameterModel&&) = delete;
    ParameterModel& operator=(ParameterModel&&) = delete;

    /**
     * @brief Добавляет новый параметр в модель.
     *
     * Параметр не будет добавлен, если параметр с таким же именем и типом уже существует.
     * @param parameter Умный указатель на добавляемый параметр.
     * @return true, если параметр успешно добавлен, иначе false.
     */
    bool addParameter(const std::shared_ptr<Parameter>& parameter);

    /**
     * @brief Удаляет параметр из модели по имени и типу.
     * @param name Имя удаляемого параметра.
     * @param type Тип удаляемого параметра.
     * @return true, если параметр успешно удален, иначе false.
     */
    bool removeParameter(const QString& name, ParameterType type);

    /**
     * @brief Возвращает параметр по имени и типу.
     * @param name Имя искомого параметра.
     * @param type Тип искомого параметра.
     * @return Умный указатель на найденный параметр или nullptr, если не найден.
     */
    std::shared_ptr<Parameter> getParameter(const QString& name, ParameterType type) const;

    /**
     * @brief Возвращает все параметры с указанным именем (разных типов).
     * @param name Имя параметра.
     * @return Вектор умных указателей на найденные параметры.
     */
     QVector<std::shared_ptr<Parameter>> getParametersByName(const QString& name) const;


    /**
     * @brief Возвращает копию вектора всех параметров в модели.
     * @return QVector<std::shared_ptr<Parameter>> со всеми параметрами.
     */
    QVector<std::shared_ptr<Parameter>> getAllParameters() const;

    /**
     * @brief Возвращает список уникальных имен всех параметров в модели.
     * @return QVector<QString> с уникальными именами параметров.
     */
    QVector<QString> getAllParameterNames() const;

    /**
     * @brief Обновляет целевое значение и описание существующего параметра.
     * @param name Имя обновляемого параметра.
     * @param type Тип обновляемого параметра.
     * @param targetValue Новое целевое значение(я).
     * @param description Новое описание параметра.
     * @return true, если параметр найден и обновлен, иначе false.
     */
    bool updateParameter(const QString& name, ParameterType type,
                         const QVariant& targetValue, const QString& description);

    /**
     * @brief Обновляет настройки звукового оповещения для параметра.
     * @param name Имя обновляемого параметра.
     * @param type Тип обновляемого параметра.
     * @param enabled Новое состояние включения звука.
     * @param soundFile Новый путь к звуковому файлу (если null, путь не меняется).
     * @return true, если параметр найден и обновлен, иначе false.
     */
    bool updateParameterSound(const QString& name, ParameterType type,
                              bool enabled, const QString& soundFile = QString());

    /**
     * @brief Сохраняет все параметры модели в JSON файл.
     * @param filename Путь к файлу для сохранения.
     * @return true, если сохранение прошло успешно, иначе false.
     */
    bool saveParameters(const QString& filename) const;

    /**
     * @brief Загружает параметры из JSON файла, заменяя текущие.
     * @param filename Путь к файлу для загрузки.
     * @return true, если загрузка прошла успешно (даже если файл пуст или не найден),
     * false при ошибках чтения или парсинга файла.
     */
    bool loadParameters(const QString& filename);

    /**
     * @brief Проверяет все параметры на соответствие условиям по новым значениям.
     *
     * Обновляет текущие значения и статусы параметров.
     * Эмитирует сигналы parameterValueChanged и parameterStatusChanged.
     * @param values Вектор новых значений параметров (имя + значение).
     */
    void checkParameters(const QVector<ParameterValue>& values);

signals:
    // Сигналы для оповещения UI и других компонентов о изменениях в модели

    /**
     * @brief Сигнал о добавлении нового параметра.
     * @param name Имя добавленного параметра.
     * @param type Тип добавленного параметра.
     */
    void parameterAdded(const QString& name, ParameterType type);

    /**
     * @brief Сигнал об удалении параметра.
     * @param name Имя удаленного параметра.
     * @param type Тип удаленного параметра.
     */
    void parameterRemoved(const QString& name, ParameterType type);

    /**
     * @brief Сигнал об обновлении данных параметра (целевое значение, описание).
     * @param name Имя обновленного параметра.
     * @param type Тип обновленного параметра.
     */
    void parameterUpdated(const QString& name, ParameterType type);

    /**
     * @brief Сигнал об изменении настроек звука для параметра.
     * @param name Имя параметра.
     * @param type Тип параметра.
     * @param enabled Новое состояние включения звука.
     */
    void parameterSoundChanged(const QString& name, ParameterType type, bool enabled);

    /**
     * @brief Сигнал об изменении статуса параметра (Ok, Error, Unknown).
     * @param name Имя параметра.
     * @param type Тип параметра.
     * @param status Новый статус параметра.
     */
    void parameterStatusChanged(const QString& name, ParameterType type, ParameterStatus status);

    /**
     * @brief Сигнал об изменении текущего значения параметра.
     * @param name Имя параметра.
     * @param value Новое текущее значение параметра.
     */
    void parameterValueChanged(const QString& name, const QVariant& value);

    /**
     * @brief Сигнал о полной перезагрузке модели (после loadParameters).
     */
    void modelReset();


private:
    mutable std::mutex m_mutex; ///< Мьютекс для защиты доступа к вектору параметров.
    QVector<std::shared_ptr<Parameter>> m_parameters; ///< Вектор умных указателей на параметры.
};

} // namespace ParamControl
