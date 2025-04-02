#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <QFont>
#include <QDateTime>
#include <memory>

#include "LogManager.h"

namespace ParamControl {

/**
 * @brief Модель для отображения записей журнала в таблице
 * 
 * Эта модель отображает записи журнала из LogManager в виде таблицы
 * для отображения в QTableView. Она предоставляет доступ к основным
 * свойствам записей: время, категория, сообщение, значение, статус.
 */
class LogTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /**
     * @brief Колонки таблицы журнала
     */
    enum Columns {
        TimestampColumn = 0,  ///< Время события
        CategoryColumn,       ///< Категория события
        MessageColumn,        ///< Сообщение
        ValueColumn,          ///< Значение
        ColumnCount           ///< Количество колонок
    };

    /**
     * @brief Конструктор
     * @param logManager Менеджер журнала
     * @param parent Родительский объект
     */
    explicit LogTableModel(const std::shared_ptr<LogManager>& logManager, 
                          QObject* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~LogTableModel() override;

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

public slots:
    /**
     * @brief Обработчик добавления записи в журнал
     * @param entry Новая запись
     */
    void onLogEntryAdded(const LogEntry& entry);
    
    /**
     * @brief Обработчик очистки журнала
     */
    void onLogCleared();
    
    /**
     * @brief Установка фильтра по категории
     * @param category Категория для фильтрации (пустая строка - без фильтра)
     */
    void setCategoryFilter(const QString& category);
    
    /**
     * @brief Установка фильтра по уровню
     * @param level Уровень для фильтрации (-1 - без фильтра)
     */
    void setLevelFilter(int level);
    
    /**
     * @brief Обновление данных модели
     */
    void refresh();

private:
    std::shared_ptr<LogManager> m_logManager;  ///< Менеджер журнала
    QVector<LogEntry> m_filteredEntries;      ///< Отфильтрованные записи
    QString m_categoryFilter;                 ///< Фильтр по категории
    int m_levelFilter;                        ///< Фильтр по уровню

    /**
     * @brief Получение цвета для отображения статуса записи
     * @param status Статус записи
     * @return Цвет для отображения
     */
    QColor getStatusColor(LogStatus status) const;
    
    /**
     * @brief Применение фильтров
     */
    void applyFilters();
};

} // namespace ParamControl
