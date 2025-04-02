#pragma once

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QStringList>

namespace Ui {
class KaZsDialog;
}

namespace ParamControl {

/**
 * @brief Диалог выбора КА и ЗС
 *
 * Этот диалог позволяет пользователю выбрать номер
 * космического аппарата (КА) и земной станции (ЗС).
 */
class KaZsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param currentKaNumber Текущий номер КА
     * @param currentZsNumber Текущий номер ЗС
     * @param parent Родительский виджет
     */
    explicit KaZsDialog(quint16 currentKaNumber, quint16 currentZsNumber, QWidget* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~KaZsDialog();
    
    /**
     * @brief Получение выбранного номера КА
     * @return Номер КА
     */
    quint16 getKaNumber() const;
    
    /**
     * @brief Получение выбранного номера ЗС
     * @return Номер ЗС
     */
    quint16 getZsNumber() const;

private slots:
    /**
     * @brief Обработчик нажатия кнопки ОК
     */
    void onOkClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Отмена
     */
    void onCancelClicked();

private:
    Ui::KaZsDialog* ui;     ///< UI диалога
    quint16 m_kaNumber;     ///< Выбранный номер КА
    quint16 m_zsNumber;     ///< Выбранный номер ЗС
    
    /**
     * @brief Загрузка списков КА и ЗС из файлов
     */
    void loadLists();
    
    /**
     * @brief Валидация выбранных значений
     * @return true, если значения валидны
     */
    bool validateSelections();
};

} // namespace ParamControl
