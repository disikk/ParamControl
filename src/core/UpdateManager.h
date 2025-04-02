#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QVersionNumber>
#include <QProcess>
#include <memory>

namespace ParamControl {

/**
 * @brief Структура для хранения информации об обновлении
 */
struct UpdateInfo {
    QVersionNumber version;      ///< Версия обновления
    QString description;         ///< Описание обновления
    QString installerPath;       ///< Путь к установщику обновления
    QDateTime releaseDate;       ///< Дата выпуска обновления
    bool isCompatible;           ///< Признак совместимости с текущей версией
};

/**
 * @brief Класс для управления обновлениями приложения
 * 
 * Этот класс отвечает за проверку наличия обновлений и их установку.
 * Обновления могут быть расположены на сетевом или локальном ресурсе.
 */
class UpdateManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param currentVersion Текущая версия приложения
     * @param parent Родительский объект
     */
    explicit UpdateManager(const QVersionNumber& currentVersion, QObject* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~UpdateManager();
    
    /**
     * @brief Инициализация менеджера обновлений
     * @param updatePath Путь к каталогу обновлений
     * @param checkAtStartup Проверять обновления при запуске
     * @param autoInstall Автоматически устанавливать обновления
     * @param checkInterval Интервал проверки обновлений в миллисекундах (0 - не проверять)
     * @return true, если инициализация успешна
     */
    bool initialize(const QString& updatePath, 
                    bool checkAtStartup = true, 
                    bool autoInstall = false,
                    int checkInterval = 0);
    
    /**
     * @brief Получение информации о последнем обновлении
     * @return Информация о последнем обновлении
     */
    UpdateInfo getLastUpdateInfo() const;
    
    /**
     * @brief Получение текущей версии приложения
     * @return Текущая версия приложения
     */
    QVersionNumber getCurrentVersion() const;
    
    /**
     * @brief Получение пути к каталогу обновлений
     * @return Путь к каталогу обновлений
     */
    QString getUpdatePath() const;
    
    /**
     * @brief Установка пути к каталогу обновлений
     * @param path Путь к каталогу обновлений
     */
    void setUpdatePath(const QString& path);
    
    /**
     * @brief Проверка, включена ли автоматическая установка обновлений
     * @return true, если автоматическая установка обновлений включена
     */
    bool isAutoInstallEnabled() const;
    
    /**
     * @brief Включение/отключение автоматической установки обновлений
     * @param enabled true, если автоматическая установка обновлений включена
     */
    void setAutoInstallEnabled(bool enabled);
    
    /**
     * @brief Проверка, включена ли проверка обновлений при запуске
     * @return true, если проверка обновлений при запуске включена
     */
    bool isCheckAtStartupEnabled() const;
    
    /**
     * @brief Включение/отключение проверки обновлений при запуске
     * @param enabled true, если проверка обновлений при запуске включена
     */
    void setCheckAtStartupEnabled(bool enabled);
    
    /**
     * @brief Получение интервала проверки обновлений
     * @return Интервал проверки обновлений в миллисекундах
     */
    int getCheckInterval() const;
    
    /**
     * @brief Установка интервала проверки обновлений
     * @param interval Интервал проверки обновлений в миллисекундах (0 - не проверять)
     */
    void setCheckInterval(int interval);
    
public slots:
    /**
     * @brief Проверка наличия обновлений
     * @return true, если есть доступные обновления
     */
    bool checkForUpdates();
    
    /**
     * @brief Установка обновления
     * @param path Путь к установщику обновления (пусто - использовать последнее найденное)
     * @return true, если установка запущена успешно
     */
    bool installUpdate(const QString& path = QString());

signals:
    /**
     * @brief Сигнал обнаружения обновления
     * @param info Информация об обновлении
     */
    void updateAvailable(const UpdateInfo& info);
    
    /**
     * @brief Сигнал начала установки обновления
     */
    void installationStarted();
    
    /**
     * @brief Сигнал завершения установки обновления
     * @param success true, если установка успешна
     * @param errorMessage Сообщение об ошибке (если установка не удалась)
     */
    void installationFinished(bool success, const QString& errorMessage = QString());
    
    /**
     * @brief Сигнал ошибки проверки обновлений
     * @param errorMessage Сообщение об ошибке
     */
    void checkError(const QString& errorMessage);

private slots:
    /**
     * @brief Обработчик таймера проверки обновлений
     */
    void onCheckTimerTimeout();
    
    /**
     * @brief Обработчик завершения процесса установки обновления
     * @param exitCode Код выхода процесса
     * @param exitStatus Статус выхода процесса
     */
    void onInstallerFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QVersionNumber m_currentVersion;             ///< Текущая версия приложения
    QString m_updatePath;                        ///< Путь к каталогу обновлений
    bool m_checkAtStartup;                       ///< Проверять обновления при запуске
    bool m_autoInstall;                          ///< Автоматически устанавливать обновления
    QTimer* m_checkTimer;                        ///< Таймер для периодической проверки обновлений
    UpdateInfo m_lastUpdateInfo;                 ///< Информация о последнем обновлении
    std::unique_ptr<QProcess> m_installerProcess; ///< Процесс установки обновления
    
    /**
     * @brief Чтение информации об обновлении из файла
     * @param infoFilePath Путь к файлу с информацией об обновлении
     * @return Информация об обновлении
     */
    UpdateInfo readUpdateInfo(const QString& infoFilePath);
    
    /**
     * @brief Поиск последнего подходящего обновления
     * @return Информация о последнем подходящем обновлении
     */
    UpdateInfo findLastUpdate();
    
    /**
     * @brief Проверка, существует ли путь к обновлениям
     * @return true, если путь существует
     */
    bool checkUpdatePathExists();
};

} // namespace ParamControl
