#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QProcess>
#include <memory>

#include "MonitoringService.h"
#include "ParameterModel.h"
#include "SotmClient.h"
#include "LogManager.h"
#include "AlertManager.h"
#include "XmlParser.h"
#include "ParameterTableModel.h"
#include "LogTableModel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для кнопок управления
    void onStartMonitoringClicked();
    void onStopMonitoringClicked();
    void onAddParameterClicked();
    void onRemoveParameterClicked();
    void onEditParameterClicked();
    void onConfigureConnectionClicked();
    void onToggleTmiSoundClicked();
    void onShowLogClicked();
    void onChangeKaZsClicked();
    void onToggleParameterPanelClicked();
    
    // Слоты для обработки событий сервисов
    void onMonitoringStatusChanged(bool running);
    void onTmiStatusChanged(bool available);
    void onConnectionStatusChanged(bool connected);
    void onParameterStatusChanged(const QString& name, bool status);
    void onParameterValueChanged(const QString& name, const QVariant& value);
    
    // Контекстное меню
    void onParameterContextMenu(const QPoint& pos);
    void onLogContextMenu(const QPoint& pos);
    void onClearLog();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::MainWindow* ui;
    
    // Сервисы и модели
    std::shared_ptr<MonitoringService> m_monitoringService;
    std::shared_ptr<ParameterModel> m_parameterModel;
    std::shared_ptr<SotmClient> m_sotmClient;
    std::shared_ptr<LogManager> m_logManager;
    std::shared_ptr<AlertManager> m_alertManager;
    std::shared_ptr<XmlParser> m_xmlParser;
    
    // Модели для таблиц
    std::unique_ptr<ParameterTableModel> m_parameterTableModel;
    std::unique_ptr<LogTableModel> m_logTableModel;
    
    void setupUi();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void updateStatusIndicators();
    void updateTmiSoundButton();
};
