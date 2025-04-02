#pragma once

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <memory>

#include "ParameterModel.h"
#include "LogManager.h"

namespace ParamControl {

class ParameterCardTableModel;

/**
 * @brief Виджет для отображения детальной информации о параметре
 * 
 * Этот виджет отображает детальную информацию о параметре, включая
 * описание, все условия контроля, текущее значение и историю изменений.
 * Позволяет редактировать описание и управлять звуковыми оповещениями.
 */
class ParameterCardView : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param parameterModel Модель параметров
     * @param logManager Менеджер журнала
     * @param parent Родительский виджет
     */
    explicit ParameterCardView(const std::shared_ptr<ParameterModel>& parameterModel,
                             const std::shared_ptr<LogManager>& logManager,
                             QWidget* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~ParameterCardView();

    /**
     * @brief Установка отображаемого параметра
     * @param name Имя параметра
     * @param type Тип параметра (если не указан, берется первый найденный)
     */
    void setParameter(const QString& name, ParameterType type = ParameterType::Equals);
    
    /**
     * @brief Получение имени текущего параметра
     * @return Имя параметра
     */
    QString getParameterName() const;
    
    /**
     * @brief Получение типа текущего параметра
     * @return Тип параметра
     */
    ParameterType getParameterType() const;
    
    /**
     * @brief Обновление отображаемой информации
     */
    void updateInfo();

signals:
    /**
     * @brief Сигнал изменения описания параметра
     * @param name Имя параметра
     * @param type Тип параметра
     * @param description Новое описание
     */
    void descriptionChanged(const QString& name, ParameterType type, const QString& description);
    
    /**
     * @brief Сигнал изменения состояния звука
     * @param name Имя параметра
     * @param type Тип параметра
     * @param enabled Новое состояние
     */
    void soundEnabledChanged(const QString& name, ParameterType type, bool enabled);
    
    /**
     * @brief Сигнал запроса на редактирование параметра
     * @param name Имя параметра
     * @param type Тип параметра
     */
    void editRequested(const QString& name, ParameterType type);
    
    /**
     * @brief Сигнал запроса на удаление параметра
     * @param name Имя параметра
     * @param type Тип параметра
     */
    void deleteRequested(const QString& name, ParameterType type);
    
    /**
     * @brief Сигнал запроса на добавление параметра с тем же именем
     * @param name Имя параметра
     */
    void addRequested(const QString& name);

private slots:
    /**
     * @brief Обработчик нажатия кнопки редактирования
     */
    void onEditButtonClicked();
    
    /**
     * @brief Обработчик нажатия кнопки удаления
     */
    void onDeleteButtonClicked();
    
    /**
     * @brief Обработчик нажатия кнопки добавления
     */
    void onAddButtonClicked();
    
    /**
     * @brief Обработчик изменения описания
     */
    void onDescriptionChanged();
    
    /**
     * @brief Обработчик изменения состояния звука
     * @param state Новое состояние
     */
    void onSoundEnabledChanged(int state);
    
    /**
     * @brief Обработчик изменения значения параметра
     * @param name Имя параметра
     * @param value Новое значение
     */
    void onValueChanged(const QString& name, const QVariant& value);
    
    /**
     * @brief Обработчик изменения статуса параметра
     * @param name Имя параметра
     * @param type Тип параметра
     * @param status Новый статус
     */
    void onStatusChanged(const QString& name, ParameterType type, ParameterStatus status);

private:
    std::shared_ptr<ParameterModel> m_parameterModel;  ///< Модель параметров
    std::shared_ptr<LogManager> m_logManager;         ///< Менеджер журнала
    QString m_parameterName;                         ///< Имя текущего параметра
    ParameterType m_parameterType;                   ///< Тип текущего параметра
    
    QLabel* m_nameLabel;                  ///< Метка с именем параметра
    QLabel* m_typeLabel;                  ///< Метка с типом параметра
    QLabel* m_valueLabel;                 ///< Метка с текущим значением
    QLabel* m_statusLabel;                ///< Метка со статусом
    QTextEdit* m_descriptionEdit;         ///< Поле для редактирования описания
    QTableView* m_conditionsTable;        ///< Таблица с условиями контроля
    QTableView* m_historyTable;           ///< Таблица с историей изменений
    QPushButton* m_editButton;            ///< Кнопка редактирования
    QPushButton* m_deleteButton;          ///< Кнопка удаления
    QPushButton* m_addButton;             ///< Кнопка добавления
    QCheckBox* m_soundEnabledCheckBox;    ///< Флажок включения звука
    
    std::unique_ptr<ParameterCardTableModel> m_conditionsModel;  ///< Модель для таблицы условий
    
    /**
     * @brief Инициализация интерфейса
     */
    void setupUi();
    
    /**
     * @brief Обновление текущих данных параметра
     */
    void updateParameterData();
    
    /**
     * @brief Обновление истории изменений параметра
     */
    void updateHistory();
    
    /**
     * @brief Обновление статуса параметра
     */
    void updateStatus();
};

/**
 * @brief Модель для отображения условий контроля параметра
 */
class ParameterCardTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param parameterModel Модель параметров
     * @param parent Родительский объект
     */
    explicit ParameterCardTableModel(const std::shared_ptr<ParameterModel>& parameterModel, 
                                   QObject* parent = nullptr);
    
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
     * @brief Установка отображаемого параметра
     * @param name Имя параметра
     */
    void setParameterName(const QString& name);
    
    /**
     * @brief Обновление модели
     */
    void refresh();

private:
    std::shared_ptr<ParameterModel> m_parameterModel;  ///< Модель параметров
    QString m_parameterName;                         ///< Имя параметра
    QVector<std::shared_ptr<Parameter>> m_parameters;  ///< Список параметров с указанным именем
};

} // namespace ParamControl
