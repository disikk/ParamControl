#include "LogDialog.h"
#include "ui_LogDialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSet>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QDateTime>
#include <QMenu>
#include <QAction>

namespace ParamControl {

LogDialog::LogDialog(const std::shared_ptr<LogManager>& logManager, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LogDialog)
    , m_logManager(logManager)
{
    ui->setupUi(this);
    
    // Создаем модель для таблицы
    m_model = std::make_unique<LogTableModel>(logManager);
    
    // Настраиваем таблицу
    ui->logTableView->setModel(m_model.get());
    ui->logTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->logTableView->horizontalHeader()->setSectionResizeMode(LogTableModel::TimestampColumn, QHeaderView::ResizeToContents);
    ui->logTableView->horizontalHeader()->setSectionResizeMode(LogTableModel::CategoryColumn, QHeaderView::ResizeToContents);
    ui->logTableView->setAlternatingRowColors(true);
    ui->logTableView->setSortingEnabled(true);
    ui->logTableView->verticalHeader()->setVisible(false);
    ui->logTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Настраиваем контекстное меню
    ui->logTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->logTableView, &QTableView::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu contextMenu(tr("Контекстное меню"), this);
        
        QAction* clearAction = contextMenu.addAction(tr("Очистить журнал"));
        QAction* exportAction = contextMenu.addAction(tr("Экспорт журнала"));
        
        QAction* selectedAction = contextMenu.exec(ui->logTableView->viewport()->mapToGlobal(pos));
        
        if (selectedAction == clearAction) {
            onClearClicked();
        } else if (selectedAction == exportAction) {
            onExportClicked();
        }
    });
    
    // Заполняем список категорий
    fillCategoryList();
    
    // Заполняем список уровней
    ui->levelComboBox->addItem("Все", -1);
    ui->levelComboBox->addItem("Информация", static_cast<int>(LogLevel::Info));
    ui->levelComboBox->addItem("Ошибка", static_cast<int>(LogLevel::Error));
    
    // Подключаем сигналы
    connect(ui->closeButton, &QPushButton::clicked, this, &LogDialog::onCloseClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &LogDialog::onClearClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &LogDialog::onRefreshClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &LogDialog::onExportClicked);
    connect(ui->categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LogDialog::onCategoryFilterChanged);
    connect(ui->levelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LogDialog::onLevelFilterChanged);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &LogDialog::onSearchTextChanged);
    
    // Подключаем сигнал добавления новой записи в журнал
    connect(m_logManager.get(), &LogManager::logEntryAdded, this, &LogDialog::onLogEntryAdded);
    
    // Настройка окна
    setWindowTitle("Журнал событий");
    resize(800, 600);
}

LogDialog::~LogDialog()
{
    delete ui;
}

void LogDialog::onCloseClicked()
{
    accept();
}

void LogDialog::onClearClicked()
{
    if (QMessageBox::question(this, "Подтверждение", 
                            "Вы уверены, что хотите очистить журнал?",
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // Очищаем журнал
        m_logManager->clearLog();
        
        // Обновляем модель
        m_model->onLogCleared();
    }
}

void LogDialog::onRefreshClicked()
{
    // Заполняем список категорий заново
    fillCategoryList();
    
    // Обновляем модель
    m_model->refresh();
}

void LogDialog::onExportClicked()
{
    // Выбираем файл для экспорта
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт журнала",
                                                 QDir::homePath() + "/log_" + 
                                                 QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".txt",
                                                 "Текстовые файлы (*.txt)");
    
    if (!fileName.isEmpty()) {
        // Сохраняем журнал в файл
        if (m_logManager->saveLog(fileName)) {
            QMessageBox::information(this, "Экспорт журнала", 
                                    "Журнал успешно экспортирован в файл:\n" + fileName);
        } else {
            QMessageBox::warning(this, "Экспорт журнала", 
                               "Не удалось экспортировать журнал в файл:\n" + fileName);
        }
    }
}

void LogDialog::onCategoryFilterChanged(int index)
{
    // Применяем фильтр
    QString category = ui->categoryComboBox->itemData(index).toString();
    m_model->setCategoryFilter(category);
}

void LogDialog::onLevelFilterChanged(int index)
{
    // Применяем фильтр
    int level = ui->levelComboBox->itemData(index).toInt();
    m_model->setLevelFilter(level);
}

void LogDialog::onSearchTextChanged(const QString& text)
{
    // TODO: Реализовать поиск по тексту
    // Это не сложно сделать с помощью QSortFilterProxyModel,
    // но для простоты опустим в данной реализации
}

void LogDialog::onLogEntryAdded(const LogEntry& entry)
{
    // Обновляем список категорий при необходимости
    QString category = entry.category;
    if (ui->categoryComboBox->findData(category) == -1) {
        ui->categoryComboBox->addItem(category, category);
    }
}

void LogDialog::fillCategoryList()
{
    // Получаем все записи журнала
    QVector<LogEntry> entries = m_logManager->getAllEntries();
    
    // Собираем уникальные категории
    QSet<QString> categories;
    for (const auto& entry : entries) {
        categories.insert(entry.category);
    }
    
    // Заполняем выпадающий список категорий
    ui->categoryComboBox->clear();
    ui->categoryComboBox->addItem("Все", "");
    
    for (const auto& category : categories) {
        ui->categoryComboBox->addItem(category, category);
    }
}

void LogDialog::applyFilters()
{
    // Применяем фильтры
    QString category = ui->categoryComboBox->currentData().toString();
    int level = ui->levelComboBox->currentData().toInt();
    
    m_model->setCategoryFilter(category);
    m_model->setLevelFilter(level);
}

} // namespace ParamControl
