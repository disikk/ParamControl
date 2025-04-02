#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QTranslator>
#include <memory>

#include "ui/MainWindow.h"
#include "core/ParameterModel.h"
#include "core/SotmClient.h"
#include "core/XmlParser.h"
#include "core/MonitoringService.h"
#include "core/AlertManager.h"
#include "core/LogManager.h"
#include "core/TmiAnalyzer.h"
#include "core/UpdateManager.h"

using namespace ParamControl;

int main(int argc, char *argv[])
{
    // Задаем информацию о приложении
    QCoreApplication::setOrganizationName("ParamControl");
    QCoreApplication::setApplicationName("ParamControl");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // Инициализация приложения Qt
    QApplication app(argc, argv);
    
    // Создаем и показываем заставку
    QPixmap splashPixmap(":/icons/splash.png");
    if (splashPixmap.isNull()) {
        // Если иконка не найдена, создаем простую заставку
        splashPixmap = QPixmap(400, 200);
        splashPixmap.fill(Qt::lightGray);
    }
    
    QSplashScreen splash(splashPixmap);
    splash.show();
    app.processEvents();
    
    // Вывод сообщения на заставке
    splash.showMessage("Инициализация приложения...", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
    app.processEvents();
    
    // Проверяем наличие необходимых директорий и файлов
    QDir dataDir("./data");
    if (!dataDir.exists()) {
        if (!QDir().mkdir("./data")) {
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось создать директорию для данных.");
            return 1;
        }
    }
    
    // Проверяем наличие файлов списков КА и ЗС
    if (!QFile::exists("./data/KA_list.txt")) {
        QFile kaFile("./data/KA_list.txt");
        if (kaFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&kaFile);
            out << "101\n102\n103\n";
            kaFile.close();
        } else {
            QMessageBox::warning(nullptr, "Предупреждение", "Не удалось создать файл списка КА.");
        }
    }
    
    if (!QFile::exists("./data/ZS_list.txt")) {
        QFile zsFile("./data/ZS_list.txt");
        if (zsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&zsFile);
            out << "111\n222\n333\n";
            zsFile.close();
        } else {
            QMessageBox::warning(nullptr, "Предупреждение", "Не удалось создать файл списка ЗС.");
        }
    }
    
    // Проверяем наличие файла интервала СОТМ
    if (!QFile::exists("./data/SotmInterval.txt")) {
        QFile intervalFile("./data/SotmInterval.txt");
        if (intervalFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&intervalFile);
            out << "1000\n";
            intervalFile.close();
        } else {
            QMessageBox::warning(nullptr, "Предупреждение", "Не удалось создать файл интервала СОТМ.");
        }
    }
    
    // Обновляем сообщение на заставке
    splash.showMessage("Создание компонентов приложения...", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
    app.processEvents();
    
    // Создаем основные компоненты приложения
    std::shared_ptr<ParameterModel> parameterModel = std::make_shared<ParameterModel>();
    std::shared_ptr<SotmClient> sotmClient = std::make_shared<SotmClient>();
    std::shared_ptr<XmlParser> xmlParser = std::make_shared<XmlParser>();
    std::shared_ptr<AlertManager> alertManager = std::make_shared<AlertManager>();
    std::shared_ptr<LogManager> logManager = std::make_shared<LogManager>();
    std::shared_ptr<TmiAnalyzer> tmiAnalyzer = std::make_shared<TmiAnalyzer>();
    std::shared_ptr<UpdateManager> updateManager = std::make_shared<UpdateManager>(QVersionNumber(1, 0, 0));
    std::shared_ptr<MonitoringService> monitoringService = std::make_shared<MonitoringService>(
        sotmClient, parameterModel, xmlParser, alertManager, logManager);
    
    // Загружаем настройки и инициализируем компоненты
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ParamControl", "ParamControl");
    
    // Инициализация логирования
    logManager->initialize("./data/LOG_main.txt");
    
    // Получаем номер КА и ЗС из настроек или запрашиваем у пользователя
    SotmSettings sotmSettings;
    sotmSettings.ipAddress = settings.value("sotm/ipAddress", "127.0.0.1").toString();
    sotmSettings.port = settings.value("sotm/port", 1234).toUInt();
    sotmSettings.kaNumber = settings.value("sotm/kaNumber", 101).toUInt();
    sotmSettings.zsNumber = settings.value("sotm/zsNumber", 111).toUInt();
    sotmSettings.responseTimeoutMs = settings.value("sotm/responseTimeoutMs", 5000).toInt();
    sotmClient->setSettings(sotmSettings);
    
    // Загружаем настройки обновлений
    QString updatePath = settings.value("updates/updatePath", "./updates").toString();
    bool checkAtStartup = settings.value("updates/checkAtStartup", true).toBool();
    bool autoInstall = settings.value("updates/autoInstall", false).toBool();
    int checkInterval = settings.value("updates/checkInterval", 0).toInt();
    updateManager->initialize(updatePath, checkAtStartup, autoInstall, checkInterval);
    
    // Обновляем сообщение на заставке
    splash.showMessage("Запуск приложения...", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
    app.processEvents();
    
    // Создаем и показываем главное окно
    MainWindow mainWindow(parameterModel, sotmClient, monitoringService, 
                        alertManager, logManager, updateManager, tmiAnalyzer);
    mainWindow.show();
    
    // Скрываем заставку
    splash.finish(&mainWindow);
    
    // Запускаем приложение
    return app.exec();
}
