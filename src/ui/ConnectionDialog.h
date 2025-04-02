#pragma once

#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

#include "../core/SotmClient.h"

namespace Ui {
class ConnectionDialog;
}

namespace ParamControl {

/**
 * @brief Диалог настройки соединения с СОТМ
 *
 * Этот диалог позволяет пользователю настроить параметры
 * подключения к СОТМ: IP-адрес, порт и таймаут ответа.
 */
class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param settings Текущие настройки соединения
     * @param parent Родительский виджет
     */
    explicit ConnectionDialog(const SotmSettings& settings, QWidget* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~ConnectionDialog();
    
    /**
     * @brief Получение настроек соединения
     * @return Настройки соединения
     */
    SotmSettings getSettings() const;

private slots:
    /**
     * @brief Обработчик нажатия кнопки ОК
     */
    void onOkClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Отмена
     */
    void onCancelClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Тест соединения
     */
    void onTestConnectionClicked();

private:
    Ui::ConnectionDialog* ui;         ///< UI диалога
    SotmSettings m_settings;          ///< Настройки соединения
    
    /**
     * @brief Валидация введенных данных
     * @return true, если данные валидны
     */
    bool validateInputs();
    
    /**
     * @brief Обновление настроек из элементов интерфейса
     */
    void updateSettings();
};

} // namespace ParamControl
