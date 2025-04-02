#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <memory>

#include "ParameterModel.h"

namespace ParamControl {

/**
 * @brief Модель для отображения параметров в таблице
 * 
 * Эта модель отображает параметры из ParameterModel в виде таблицы
 * для отображения в QTableView. Она предоставляет доступ к основным
 * свойствам параметров: имя, условие контроля, статус и состояние звука.
 */
class ParameterTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /**
     * @brief Колонки таблицы параметров
     */
    enum Columns {
        SoundColumn = 0,    ///< Состояние звука
        NameColumn,         ///< Имя параметра
        ConditionColumn,    ///< Условие контроля
        ColumnCount         ///< Количество колонок
    };

    /**
     * @brief Конструктор
     * @param parameterModel Модель параметров
     * @param parent Родительский объект
     */
    explicit ParameterTableModel(const std::shared_ptr<ParameterModel>& parameterModel, 
                                QObject* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~ParameterTableModel() override;

    /**
     * @brief Получение количества строк
     * @param parent Родительский индекс
     * @return Количество строк
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    
    /**
     * @brief Получение количества столбцов
     * @param parent Родительский индекс
     * @return Количество столбцов
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    
    /**
     * @brief Получение данных для отображения
     * @param index Индекс элемента
     * @param role Роль данных
     * @return Данные для отображения
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    
    /**
     * @brief Получение заголовков
     * @param section Номер секции
     * @param orientation Ориентация
     * @param role Роль данных
     * @return Данные заголовка
     */
    QVariant headerData(int section, Qt::Orientation orientation, 
                        int role = Qt::DisplayRole) const override;
    
    /**
     * @brief Получение флагов элемента
     * @param index Индекс элемента
     * @return Флаги элемента
     */
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    
    /**
     * @brief Установка данных
     * @param index Индекс элемента
     * @param value Новое значение
     * @param role Роль данных
     * @return true, если данные успешно установлены
     */
    bool setData(const QModelIndex& index, const QVariant& value, 
                int role = Qt::EditRole) override;

public slots:
    /**
     * @brief Обработчик добавления параметра
     * @param name Имя параметра
     * @param type Тип параметра
     */
    void onParameterAdded(const QString& name, ParameterType type);
    
    /**
     * @brief Обработчик удаления параметра
     * @param name Имя параметра
     * @param type Тип параметра
     */
    void onParameterRemoved(const QString& name, ParameterType type);
    
    /**
     * @brief Обработчик обновления параметра
     * @param name Имя параметра
     * @param type Тип параметра
     */
    void onParameterUpdated(const QString& name, ParameterType type);
    
    /**
     * @brief Обработчик изменения состояния звука параметра
     * @param name Имя параметра
     * @param type Тип параметра
     * @param enabled Состояние звука
     */
    void onParameterSoundChanged(const QString& name, ParameterType type, bool enabled);
    
    /**
     * @brief Обработчик изменения статуса параметра
     * @param name Имя параметра
     * @param type Тип параметра
     * @param status Статус параметра
     */
    void onParameterStatusChanged(const QString& name, ParameterType type, ParameterStatus status);

private:
    std::shared_ptr<ParameterModel> m_parameterModel;  ///< Модель параметров
    QIcon m_soundEnabledIcon;                         ///< Иконка включенного звука
    QIcon m_soundDisabledIcon;                        ///< Иконка отключенного звука

    /**
     * @brief Получение цвета для отображения статуса параметра
     * @param status Статус параметра
     * @return Цвет для отображения
     */
    QColor getStatusColor(ParameterStatus status) const;
    
    /**
     * @brief Поиск индекса параметра в модели
     * @param name Имя параметра
     * @param type Тип параметра
     * @return Индекс параметра в модели или -1, если не найден
     */
    int findParameterIndex(const QString& name, ParameterType type) const;
};

} // namespace ParamControl
