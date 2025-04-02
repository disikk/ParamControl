#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <memory>

#include "SotmClient.h"
#include "UpdateManager.h"
#include "AlertManager.h"
#include "MonitoringService.h"

namespace ParamControl {

/**
 * @brief Диалог настроек приложения
 * 
 * Этот диалог обеспечивает доступ к настройкам приложения через
 * табулированный интерфейс с вкладками. Он позволяет настраивать
 * подключение к СОТМ, звуковые оповещения, КА и ЗС, обновления и т.д.
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param sotmClient Клиент СОТМ
     * @param updateManager Менеджер обновлений
     * @param alertManager Менеджер оповещений
     * @param monitoringService Сервис мониторинга
     * @param parent Родительский виджет
     */
    explicit SettingsDialog(const std::shared_ptr<SotmClient>& sotmClient,
                          const std::shared_ptr<UpdateManager>& updateManager,
                          const std::shared_ptr<AlertManager>& alertManager,
                          const std::shared_ptr<MonitoringService>& monitoringService,
                          QWidget* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~SettingsDialog();

signals:
    /**
     * @brief Сигнал изменения настроек СОТМ
     * @param settings Новые настройки
     */
    void sotmSettingsChanged(const SotmSettings& settings);
    
    /**
     * @brief Сигнал изменения настроек обновлений
     * @param path Путь к обновлениям
     * @param checkAtStartup Проверять при запуске
     * @param autoInstall Автоматически устанавливать
     * @param interval Интервал проверки
     */
    void updateSettingsChanged(const QString& path, bool checkAtStartup, 
                              bool autoInstall, int interval);
    
    /**
     * @brief Сигнал изменения настроек оповещений
     * @param type Тип оповещения
     * @param sound Путь к звуковому файлу
     * @param enabled Включено ли оповещение
     */
    void alertSettingsChanged(AlertType type, const QString& sound, bool enabled);
    
    /**
     * @brief Сигнал запроса на проверку обновлений
     */
    void checkForUpdatesRequested();
    
    /**
     * @brief Сигнал запроса на перезапуск приложения
     * @param reason Причина перезапуска
     */
    void restartRequested(const QString& reason);

private slots:
    /**
     * @brief Обработчик нажатия кнопки ОК
     */
    void onOkClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Применить
     */
    void onApplyClicked();
    
    /**
     * @brief Обработчик нажатия кнопки Отмена
     */
    void onCancelClicked();
    
    /**
     * @brief Обработчик нажатия кнопки выбора звукового файла
     * @param type Тип оповещения
     */
    void onSelectSoundClicked(AlertType type);
    
    /**
     * @brief Обработчик нажатия кнопки выбора пути к обновлениям
     */
    void onSelectUpdatePathClicked();
    
    /**
     * @brief Обработчик нажатия кнопки проверки обновлений
     */
    void onCheckUpdatesClicked();
    
    /**
     * @brief Обработчик нажатия кнопки изменения КА и ЗС
     */
    void onChangeKaZsClicked();
    
    /**
     * @brief Обработчик сигнала доступности обновления
     * @param info Информация об обновлении
     */
    void onUpdateAvailable(const UpdateInfo& info);
    
    /**
     * @brief Обработчик сигнала ошибки проверки обновлений
     * @param errorMessage Сообщение об ошибке
     */
    void onCheckError(const QString& errorMessage);

private:
    std::shared_ptr<SotmClient> m_sotmClient;            ///< Клиент СОТМ
    std::shared_ptr<UpdateManager> m_updateManager;      ///< Менеджер обновлений
    std::shared_ptr<AlertManager> m_alertManager;        ///< Менеджер оповещений
    std::shared_ptr<MonitoringService> m_monitoringService; ///< Сервис мониторинга
    
    QTabWidget* m_tabWidget;              ///< Виджет с вкладками
    
    // Вкладка Соединение
    QLineEdit* m_ipAddressEdit;           ///< Поле для ввода IP-адреса
    QSpinBox* m_portSpinBox;              ///< Поле для ввода порта
    QSpinBox* m_timeoutSpinBox;           ///< Поле для ввода таймаута
    QSpinBox* m_pollingIntervalSpinBox;   ///< Поле для ввода интервала опроса
    QSpinBox* m_watchdogTimeoutSpinBox;   ///< Поле для ввода таймаута сторожевого таймера
    
    // Вкладка Звуки
    struct SoundWidgets {
        QLineEdit* pathEdit;              ///< Поле для пути к звуковому файлу
        QPushButton* selectButton;        ///< Кнопка выбора звукового файла
        QCheckBox* enabledCheckBox;       ///< Флажок включения звука
    };
    
    QMap<AlertType, SoundWidgets> m_soundWidgets;  ///< Виджеты для настройки звуков
    
    // Вкладка КА и ЗС
    QSpinBox* m_kaNumberSpinBox;          ///< Поле для ввода номера КА
    QSpinBox* m_zsNumberSpinBox;          ///< Поле для ввода номера ЗС
    QPushButton* m_changeKaZsButton;      ///< Кнопка изменения КА и ЗС
    
    // Вкладка Обновления
    QLineEdit* m_updatePathEdit;          ///< Поле для пути к обновлениям
    QPushButton* m_selectUpdatePathButton; ///< Кнопка выбора пути к обновлениям
    QCheckBox* m_checkAtStartupCheckBox;  ///< Флажок проверки при запуске
    QCheckBox* m_autoInstallCheckBox;     ///< Флажок автоматической установки
    QSpinBox* m_checkIntervalSpinBox;     ///< Поле для ввода интервала проверки
    QPushButton* m_checkUpdatesButton;    ///< Кнопка проверки обновлений
    QLabel* m_updateStatusLabel;          ///< Метка статуса обновлений
    
    // Кнопки диалога
    QPushButton* m_okButton;              ///< Кнопка ОК
    QPushButton* m_applyButton;           ///< Кнопка Применить
    QPushButton* m_cancelButton;          ///< Кнопка Отмена
    
    /**
     * @brief Инициализация интерфейса
     */
    void setupUi();
    
    /**
     * @brief Загрузка настроек
     */
    void loadSettings();
    
    /**
     * @brief Сохранение настроек
     */
    void saveSettings();
    
    /**
     * @brief Обновление статуса кнопок
     */
    void updateButtonsStatus();
    
    /**
     * @brief Создание виджетов для настройки звуков
     * @param type Тип оповещения
     * @param title Заголовок группы
     * @param parent Родительский виджет
     * @return QGroupBox с виджетами
     */
    QGroupBox* createSoundGroup(AlertType type, const QString& title, QWidget* parent);
};

} // namespace ParamControl
