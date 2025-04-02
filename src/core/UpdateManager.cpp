#include "UpdateManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace ParamControl {

UpdateManager::UpdateManager(const QVersionNumber& currentVersion, QObject* parent)
    : QObject(parent)
    , m_currentVersion(currentVersion)
    , m_checkAtStartup(true)
    , m_autoInstall(false)
    , m_checkTimer(new QTimer(this))
{
    // Подключаем сигнал таймера
    connect(m_checkTimer, &QTimer::timeout, this, &UpdateManager::onCheckTimerTimeout);
}

UpdateManager::~UpdateManager() {
    // Останавливаем таймер перед уничтожением
    if (m_checkTimer->isActive()) {
        m_checkTimer->stop();
    }
    
    // Если установщик запущен, завершаем его
    if (m_installerProcess && m_installerProcess->state() != QProcess::NotRunning) {
        m_installerProcess->terminate();
        m_installerProcess->waitForFinished(1000);
        if (m_installerProcess->state() != QProcess::NotRunning) {
            m_installerProcess->kill();
        }
    }
}

bool UpdateManager::initialize(const QString& updatePath, bool checkAtStartup, 
                              bool autoInstall, int checkInterval) {
    m_updatePath = updatePath;
    m_checkAtStartup = checkAtStartup;
    m_autoInstall = autoInstall;
    
    // Проверяем, существует ли путь к обновлениям
    if (!checkUpdatePathExists()) {
        qWarning() << "Update path does not exist:" << updatePath;
        return false;
    }
    
    // Если нужно проверять обновления при запуске, проверяем
    if (m_checkAtStartup) {
        QTimer::singleShot(1000, this, &UpdateManager::checkForUpdates);
    }
    
    // Если задан интервал проверки, запускаем таймер
    if (checkInterval > 0) {
        m_checkTimer->setInterval(checkInterval);
        m_checkTimer->start();
    }
    
    return true;
}

UpdateInfo UpdateManager::getLastUpdateInfo() const {
    return m_lastUpdateInfo;
}

QVersionNumber UpdateManager::getCurrentVersion() const {
    return m_currentVersion;
}

QString UpdateManager::getUpdatePath() const {
    return m_updatePath;
}

void UpdateManager::setUpdatePath(const QString& path) {
    if (m_updatePath != path) {
        m_updatePath = path;
    }
}

bool UpdateManager::isAutoInstallEnabled() const {
    return m_autoInstall;
}

void UpdateManager::setAutoInstallEnabled(bool enabled) {
    m_autoInstall = enabled;
}

bool UpdateManager::isCheckAtStartupEnabled() const {
    return m_checkAtStartup;
}

void UpdateManager::setCheckAtStartupEnabled(bool enabled) {
    m_checkAtStartup = enabled;
}

int UpdateManager::getCheckInterval() const {
    return m_checkTimer->interval();
}

void UpdateManager::setCheckInterval(int interval) {
    // Останавливаем текущий таймер
    if (m_checkTimer->isActive()) {
        m_checkTimer->stop();
    }
    
    // Если интервал положительный, запускаем таймер
    if (interval > 0) {
        m_checkTimer->setInterval(interval);
        m_checkTimer->start();
    }
}

bool UpdateManager::checkForUpdates() {
    // Проверяем, существует ли путь к обновлениям
    if (!checkUpdatePathExists()) {
        emit checkError(QString("Путь к обновлениям не существует: %1").arg(m_updatePath));
        return false;
    }
    
    // Ищем последнее обновление
    m_lastUpdateInfo = findLastUpdate();
    
    // Если нашли обновление и оно новее текущей версии
    if (!m_lastUpdateInfo.version.isNull() && m_lastUpdateInfo.version > m_currentVersion) {
        // Эмитируем сигнал о наличии обновления
        emit updateAvailable(m_lastUpdateInfo);
        
        // Если включена автоматическая установка, запускаем установку
        if (m_autoInstall) {
            installUpdate();
        }
        
        return true;
    }
    
    return false;
}

bool UpdateManager::installUpdate(const QString& path) {
    // Путь к установщику
    QString installerPath = path;
    
    // Если путь не указан, используем путь из последнего обновления
    if (installerPath.isEmpty()) {
        if (m_lastUpdateInfo.installerPath.isEmpty()) {
            emit installationFinished(false, "Не указан путь к установщику");
            return false;
        }
        installerPath = m_lastUpdateInfo.installerPath;
    }
    
    // Проверяем, существует ли установщик
    QFileInfo installerInfo(installerPath);
    if (!installerInfo.exists() || !installerInfo.isFile() || !installerInfo.isExecutable()) {
        emit installationFinished(false, QString("Установщик не существует или не является исполняемым файлом: %1").arg(installerPath));
        return false;
    }
    
    // Создаем процесс установки
    m_installerProcess = std::make_unique<QProcess>();
    
    // Подключаем сигнал о завершении процесса
    connect(m_installerProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &UpdateManager::onInstallerFinished);
    
    // Запускаем установщик
    try {
        m_installerProcess->start(installerPath, QStringList());
        
        // Эмитируем сигнал о начале установки
        emit installationStarted();
        
        return true;
    } catch (const std::exception& e) {
        emit installationFinished(false, QString("Ошибка запуска установщика: %1").arg(e.what()));
        return false;
    }
}

void UpdateManager::onCheckTimerTimeout() {
    // Проверяем наличие обновлений
    checkForUpdates();
}

void UpdateManager::onInstallerFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    // Проверяем, успешно ли завершился процесс установки
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        emit installationFinished(true);
    } else {
        QString errorOutput = QString::fromUtf8(m_installerProcess->readAllStandardError());
        emit installationFinished(false, QString("Установка не удалась. Код выхода: %1. Ошибка: %2")
                                .arg(exitCode).arg(errorOutput));
    }
    
    // Очищаем указатель на процесс
    m_installerProcess.reset();
}

UpdateInfo UpdateManager::readUpdateInfo(const QString& infoFilePath) {
    UpdateInfo info;
    
    // Открываем файл с информацией об обновлении
    QFile file(infoFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open update info file:" << infoFilePath;
        return info;
    }
    
    // Читаем JSON
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format in update info file:" << infoFilePath;
        return info;
    }
    
    QJsonObject obj = doc.object();
    
    // Парсим информацию об обновлении
    if (obj.contains("version")) {
        info.version = QVersionNumber::fromString(obj["version"].toString());
    }
    
    if (obj.contains("description")) {
        info.description = obj["description"].toString();
    }
    
    if (obj.contains("installerPath")) {
        // Получаем полный путь к установщику
        QString relativePath = obj["installerPath"].toString();
        QFileInfo fileInfo(infoFilePath);
        QDir dir = fileInfo.dir();
        info.installerPath = dir.absoluteFilePath(relativePath);
    } else {
        // Если путь к установщику не указан, пытаемся найти его в той же директории
        QFileInfo fileInfo(infoFilePath);
        QDir dir = fileInfo.dir();
        QStringList filters;
        filters << "*.run" << "*.sh" << "*.bin" << "*.AppImage";
        QStringList installers = dir.entryList(filters, QDir::Files | QDir::Executable);
        
        if (!installers.isEmpty()) {
            info.installerPath = dir.absoluteFilePath(installers.first());
        }
    }
    
    if (obj.contains("releaseDate")) {
        info.releaseDate = QDateTime::fromString(obj["releaseDate"].toString(), Qt::ISODate);
    }
    
    if (obj.contains("minVersion")) {
        QVersionNumber minVersion = QVersionNumber::fromString(obj["minVersion"].toString());
        // Обновление совместимо, если текущая версия >= минимальной версии
        info.isCompatible = (m_currentVersion >= minVersion);
    } else {
        // Если минимальная версия не указана, считаем обновление совместимым
        info.isCompatible = true;
    }
    
    return info;
}

UpdateInfo UpdateManager::findLastUpdate() {
    UpdateInfo latestUpdate;
    
    // Открываем директорию обновлений
    QDir updateDir(m_updatePath);
    if (!updateDir.exists()) {
        qWarning() << "Update directory does not exist:" << m_updatePath;
        return latestUpdate;
    }
    
    // Получаем список поддиректорий (версий)
    QStringList versionDirs = updateDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    // Преобразуем список версий в список пар (версия, путь)
    struct VersionInfo {
        QVersionNumber version;
        QString path;
    };
    
    std::vector<VersionInfo> versions;
    for (const QString& dirName : versionDirs) {
        QVersionNumber version = QVersionNumber::fromString(dirName);
        if (!version.isNull()) {
            VersionInfo vInfo;
            vInfo.version = version;
            vInfo.path = updateDir.absoluteFilePath(dirName);
            versions.push_back(vInfo);
        }
    }
    
    // Сортируем версии по убыванию
    std::sort(versions.begin(), versions.end(), 
              [](const VersionInfo& a, const VersionInfo& b) {
                  return a.version > b.version;
              });
    
    // Ищем подходящее обновление
    for (const VersionInfo& vInfo : versions) {
        // Если версия обновления меньше или равна текущей, пропускаем
        if (vInfo.version <= m_currentVersion) {
            continue;
        }
        
        // Ищем файл с информацией об обновлении
        QDir versionDir(vInfo.path);
        QStringList infoFiles = versionDir.entryList(QStringList() << "update_info.json", QDir::Files);
        
        if (!infoFiles.isEmpty()) {
            // Читаем информацию об обновлении
            UpdateInfo info = readUpdateInfo(versionDir.absoluteFilePath(infoFiles.first()));
            
            // Если обновление совместимо и новее текущей версии, берем его
            if (info.isCompatible && info.version > m_currentVersion) {
                return info;
            }
        }
    }
    
    return latestUpdate;
}

bool UpdateManager::checkUpdatePathExists() {
    // Проверяем, что путь к обновлениям не пустой
    if (m_updatePath.isEmpty()) {
        return false;
    }
    
    // Проверяем, существует ли путь
    QDir dir(m_updatePath);
    return dir.exists();
}

} // namespace ParamControl
