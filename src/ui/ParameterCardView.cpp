#include "ParameterCardView.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QDateTime>

namespace ParamControl {

ParameterCardView::ParameterCardView(const std::shared_ptr<ParameterModel>& parameterModel,
                                   const std::shared_ptr<LogManager>& logManager,
                                   QWidget* parent)
    : QWidget(parent)
    , m_parameterModel(parameterModel)
    , m_logManager(logManager)
    , m_parameterName("")
    , m_parameterType(ParameterType::Equals)
{
    // Инициализация интерфейса
    setupUi();
    
    // Создание модели для таблицы условий
    m_conditionsModel = std::make_unique<ParameterCardTableModel>(m_parameterModel, this);
    
    // Установка модели для таблицы условий
    m_conditionsTable->setModel(m_conditionsModel.get());
    
    // Подключение сигналов
    connect(m_editButton, &QPushButton::clicked, this, &ParameterCardView::onEditButtonClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &ParameterCardView::onDeleteButtonClicked);
    connect(m_addButton, &QPushButton::clicked, this, &ParameterCardView::onAddButtonClicked);
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &ParameterCardView::onDescriptionChanged);
    connect(m_soundEnabledCheckBox, &QCheckBox::stateChanged, this, &ParameterCardView::onSoundEnabledChanged);
    
    // Подключение сигналов модели параметров
    connect(m_parameterModel.get(), &ParameterModel::parameterValueChanged, 
            this, &ParameterCardView::onValueChanged);
    connect(m_parameterModel.get(), &ParameterModel::parameterStatusChanged, 
            this, &ParameterCardView::onStatusChanged);
}

ParameterCardView::~ParameterCardView() {
}

void ParameterCardView::setParameter(const QString& name, ParameterType type) {
    m_parameterName = name;
    m_parameterType = type;
    
    // Обновление модели для таблицы условий
    m_conditionsModel->setParameterName(name);
    
    // Обновление данных
    updateInfo();
}

QString ParameterCardView::getParameterName() const {
    return m_parameterName;
}

ParameterType ParameterCardView::getParameterType() const {
    return m_parameterType;
}

void ParameterCardView::updateInfo() {
    // Обновление данных параметра
    updateParameterData();
    
    // Обновление истории
    updateHistory();
    
    // Обновление статуса
    updateStatus();
    
    // Обновление таблицы условий
    m_conditionsModel->refresh();
}

void ParameterCardView::onEditButtonClicked() {
    // Эмитируем сигнал запроса на редактирование
    emit editRequested(m_parameterName, m_parameterType);
}

void ParameterCardView::onDeleteButtonClicked() {
    // Запрашиваем подтверждение
    QMessageBox msgBox;
    msgBox.setWindowTitle("Подтверждение");
    msgBox.setText(QString("Точно удалить параметр %1?").arg(m_parameterName));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    if (msgBox.exec() == QMessageBox::Yes) {
        // Эмитируем сигнал запроса на удаление
        emit deleteRequested(m_parameterName, m_parameterType);
    }
}

void ParameterCardView::onAddButtonClicked() {
    // Эмитируем сигнал запроса на добавление
    emit addRequested(m_parameterName);
}

void ParameterCardView::onDescriptionChanged() {
    // Получаем текущее описание
    QString description = m_descriptionEdit->toPlainText();
    
    // Эмитируем сигнал изменения описания
    emit descriptionChanged(m_parameterName, m_parameterType, description);
}

void ParameterCardView::onSoundEnabledChanged(int state) {
    // Получаем состояние звука
    bool enabled = (state == Qt::Checked);
    
    // Эмитируем сигнал изменения состояния звука
    emit soundEnabledChanged(m_parameterName, m_parameterType, enabled);
}

void ParameterCardView::onValueChanged(const QString& name, const QVariant& value) {
    // Проверяем, это ли наш параметр
    if (name == m_parameterName) {
        // Обновляем метку значения
        m_valueLabel->setText(value.toString());
    }
}

void ParameterCardView::onStatusChanged(const QString& name, ParameterType type, ParameterStatus status) {
    // Проверяем, это ли наш параметр
    if (name == m_parameterName && type == m_parameterType) {
        // Обновляем статус
        updateStatus();
    }
}

void ParameterCardView::setupUi() {
    // Создание элементов интерфейса
    
    // Основной Layout
    auto mainLayout = new QVBoxLayout(this);
    
    // Заголовок
    auto headerLayout = new QHBoxLayout();
    m_nameLabel = new QLabel("Параметр:", this);
    m_nameLabel->setFont(QFont("Arial", 12, QFont::Bold));
    headerLayout->addWidget(m_nameLabel);
    
    m_typeLabel = new QLabel("Тип:", this);
    headerLayout->addWidget(m_typeLabel);
    
    m_statusLabel = new QLabel("Статус:", this);
    headerLayout->addWidget(m_statusLabel);
    
    m_valueLabel = new QLabel("Значение:", this);
    headerLayout->addWidget(m_valueLabel);
    
    headerLayout->addStretch();
    
    // Кнопки управления
    m_editButton = new QPushButton("Редактировать", this);
    headerLayout->addWidget(m_editButton);
    
    m_deleteButton = new QPushButton("Удалить", this);
    headerLayout->addWidget(m_deleteButton);
    
    m_addButton = new QPushButton("Добавить условие", this);
    headerLayout->addWidget(m_addButton);
    
    mainLayout->addLayout(headerLayout);
    
    // Группа с описанием
    auto descriptionGroup = new QGroupBox("Описание", this);
    auto descriptionLayout = new QVBoxLayout(descriptionGroup);
    
    m_descriptionEdit = new QTextEdit(descriptionGroup);
    descriptionLayout->addWidget(m_descriptionEdit);
    
    // Чекбокс для звука
    m_soundEnabledCheckBox = new QCheckBox("Включить звуковое оповещение", descriptionGroup);
    descriptionLayout->addWidget(m_soundEnabledCheckBox);
    
    mainLayout->addWidget(descriptionGroup);
    
    // Разделитель для таблиц
    auto splitter = new QSplitter(Qt::Vertical, this);
    
    // Группа с условиями контроля
    auto conditionsGroup = new QGroupBox("Условия контроля", this);
    auto conditionsLayout = new QVBoxLayout(conditionsGroup);
    
    m_conditionsTable = new QTableView(conditionsGroup);
    m_conditionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    conditionsLayout->addWidget(m_conditionsTable);
    
    splitter->addWidget(conditionsGroup);
    
    // Группа с историей изменений
    auto historyGroup = new QGroupBox("История изменений", this);
    auto historyLayout = new QVBoxLayout(historyGroup);
    
    m_historyTable = new QTableView(historyGroup);
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyLayout->addWidget(m_historyTable);
    
    splitter->addWidget(historyGroup);
    
    // Установка размеров сплиттера
    splitter->setSizes(QList<int>() << 200 << 300);
    
    mainLayout->addWidget(splitter);
    
    // Установка размеров окна
    setMinimumSize(800, 600);
    resize(800, 600);
}

void ParameterCardView::updateParameterData() {
    // Получаем параметр
    auto parameter = m_parameterModel->getParameter(m_parameterName, m_parameterType);
    
    if (!parameter) {
        return;
    }
    
    // Обновляем метки
    m_nameLabel->setText(QString("Параметр: %1").arg(parameter->getName()));
    
    QString typeText;
    switch (parameter->getType()) {
        case ParameterType::Equals:
            typeText = "Равенство";
            break;
        case ParameterType::NotEquals:
            typeText = "Неравенство";
            break;
        case ParameterType::InLimits:
            typeText = "В пределах";
            break;
        case ParameterType::OutOfLimits:
            typeText = "Вне пределов";
            break;
        case ParameterType::Changed:
            typeText = "Изменение";
            break;
        default:
            typeText = "Неизвестный";
            break;
    }
    
    m_typeLabel->setText(QString("Тип: %1").arg(typeText));
    m_valueLabel->setText(QString("Значение: %1").arg(parameter->getCurrentValue().toString()));
    
    // Обновляем описание
    m_descriptionEdit->setPlainText(parameter->getDescription());
    
    // Обновляем состояние звука
    m_soundEnabledCheckBox->setChecked(parameter->isSoundEnabled());
}

void ParameterCardView::updateHistory() {
    // Получаем историю из логов
    QVector<LogEntry> allEntries = m_logManager->getEntriesByCategory(m_parameterName);
    
    // TODO: Создать модель для истории и отобразить ее в таблице
}

void ParameterCardView::updateStatus() {
    // Получаем параметр
    auto parameter = m_parameterModel->getParameter(m_parameterName, m_parameterType);
    
    if (!parameter) {
        return;
    }
    
    // Обновляем статус
    QString statusText;
    QString styleSheet;
    
    switch (parameter->getStatus()) {
        case ParameterStatus::Ok:
            statusText = "В норме";
            styleSheet = "color: green; font-weight: bold;";
            break;
        case ParameterStatus::Error:
            statusText = "Ошибка";
            styleSheet = "color: red; font-weight: bold;";
            break;
        case ParameterStatus::Unknown:
            statusText = "Неизвестно";
            styleSheet = "color: gray; font-weight: bold;";
            break;
        default:
            statusText = "Неизвестно";
            styleSheet = "color: gray; font-weight: bold;";
            break;
    }
    
    m_statusLabel->setText(QString("Статус: %1").arg(statusText));
    m_statusLabel->setStyleSheet(styleSheet);
}

// ---------------------------------------------------------------------

ParameterCardTableModel::ParameterCardTableModel(const std::shared_ptr<ParameterModel>& parameterModel, 
                                               QObject* parent)
    : QAbstractTableModel(parent)
    , m_parameterModel(parameterModel)
{
}

int ParameterCardTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    return m_parameters.size();
}

int ParameterCardTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    return 3; // Тип, Условие, Статус
}

QVariant ParameterCardTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || 
        index.row() < 0 || 
        index.row() >= m_parameters.size() || 
        index.column() < 0 || 
        index.column() >= 3) {
        return QVariant();
    }
    
    const auto& parameter = m_parameters[index.row()];
    
    switch (role) {
        case Qt::DisplayRole: {
            switch (index.column()) {
                case 0: { // Тип
                    QString typeText;
                    switch (parameter->getType()) {
                        case ParameterType::Equals:
                            typeText = "Равенство";
                            break;
                        case ParameterType::NotEquals:
                            typeText = "Неравенство";
                            break;
                        case ParameterType::InLimits:
                            typeText = "В пределах";
                            break;
                        case ParameterType::OutOfLimits:
                            typeText = "Вне пределов";
                            break;
                        case ParameterType::Changed:
                            typeText = "Изменение";
                            break;
                        default:
                            typeText = "Неизвестный";
                            break;
                    }
                    return typeText;
                }
                
                case 1: // Условие
                    return parameter->getConditionDescription();
                    
                case 2: { // Статус
                    QString statusText;
                    switch (parameter->getStatus()) {
                        case ParameterStatus::Ok:
                            statusText = "В норме";
                            break;
                        case ParameterStatus::Error:
                            statusText = "Ошибка";
                            break;
                        case ParameterStatus::Unknown:
                            statusText = "Неизвестно";
                            break;
                        default:
                            statusText = "Неизвестно";
                            break;
                    }
                    return statusText;
                }
                
                default:
                    return QVariant();
            }
            break;
        }
        
        case Qt::TextAlignmentRole: {
            return Qt::AlignCenter;
        }
        
        case Qt::BackgroundRole: {
            // Цвет фона в зависимости от статуса
            if (index.column() == 2) {
                switch (parameter->getStatus()) {
                    case ParameterStatus::Ok:
                        return QColor(204, 255, 204); // Светло-зеленый
                        
                    case ParameterStatus::Error:
                        return QColor(255, 102, 102); // Светло-красный
                        
                    case ParameterStatus::Unknown:
                        return QColor(Qt::lightGray);
                        
                    default:
                        return QColor(Qt::white);
                }
            }
            return QVariant();
        }
        
        case Qt::ForegroundRole: {
            // Цвет текста в зависимости от статуса
            if (index.column() == 2) {
                switch (parameter->getStatus()) {
                    case ParameterStatus::Error:
                        return QColor(Qt::darkRed);
                        
                    case ParameterStatus::Ok:
                        return QColor(Qt::darkGreen);
                        
                    default:
                        return QColor(Qt::black);
                }
            }
            return QVariant();
        }
        
        case Qt::FontRole: {
            // Выделяем текст жирным для статуса
            if (index.column() == 2) {
                QFont font;
                font.setBold(true);
                return font;
            }
            return QVariant();
        }
        
        default:
            return QVariant();
    }
}

QVariant ParameterCardTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }
    
    switch (role) {
        case Qt::DisplayRole: {
            switch (section) {
                case 0:
                    return QString("Тип");
                    
                case 1:
                    return QString("Условие");
                    
                case 2:
                    return QString("Статус");
                    
                default:
                    return QVariant();
            }
            break;
        }
        
        case Qt::TextAlignmentRole: {
            return Qt::AlignCenter;
        }
        
        case Qt::FontRole: {
            QFont font;
            font.setBold(true);
            return font;
        }
        
        default:
            return QVariant();
    }
}

void ParameterCardTableModel::setParameterName(const QString& name) {
    if (m_parameterName != name) {
        m_parameterName = name;
        refresh();
    }
}

void ParameterCardTableModel::refresh() {
    beginResetModel();
    
    // Получаем все параметры с указанным именем
    m_parameters.clear();
    const auto& allParameters = m_parameterModel->getAllParameters();
    
    for (const auto& parameter : allParameters) {
        if (parameter->getName() == m_parameterName) {
            m_parameters.append(parameter);
        }
    }
    
    endResetModel();
}

} // namespace ParamControl
