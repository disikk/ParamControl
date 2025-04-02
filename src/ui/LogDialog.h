#pragma once

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QLineEdit>
#include <memory>

#include "../core/LogManager.h"
#include "LogTableModel.h"

namespace Ui {
class LogDialog;
}

namespace ParamControl {

/**
 * @brief Диалог просмотра журнала событий
 *
 * Этот диалог позволяет просматривать записи журнала,
 * фильтровать их по категории и уровню, а также
 * выполнять поиск по тексту.
 */
class LogDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param logManager Менеджер журнала
     * @param parent Родительский виджет
     */
    explicit LogDialog(const std::shared_ptr<LogManager>& logManager, QWidget* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~LogDialog();

private slots:
    /**
     * @brief Обработчик нажатия кнопки Закрыть
     */
    void onCloseClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Очистить
     */
    void onClearClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Обновить
     */
    void onRefreshClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Экспорт
     */
    void onExportClicked();
    
    /**
     * @brief Обработчик изменения фильтра категории
     * @param index Индекс выбранного элемента
     */
    void onCategoryFilterChanged(int index);
    
    /**
     * @brief Обработчик изменения фильтра уровня
     * @param index Индекс выбранного элемента
     */
    void onLevelFilterChanged(int index);
    
    /**
     * @brief Обработчик изменения текста поиска
     * @param text Новый текст поиска
     */
    void onSearchTextChanged(const QString& text);
    
    /**
     * @brief Обработчик добавления записи в журнал
     * @param entry Новая запись
     */
    void onLogEntryAdded(const LogEntry& entry);

private:
    Ui::LogDialog* ui;                         ///< UI диалога
    std::shared_ptr<LogManager> m_logManager;  ///< Менеджер журнала
    std::unique_ptr<LogTableModel> m_model;    ///< Модель данных для таблицы
    
    /**
     * @brief Заполнение списка категорий
     */
    void fillCategoryList();
    
    /**
     * @brief Применение фильтров
     */
    void applyFilters();
};

} // namespace ParamControl
