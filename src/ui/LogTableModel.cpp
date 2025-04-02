#include "LogTableModel.h"

namespace ParamControl {

LogTableModel::LogTableModel(const std::shared_ptr<LogManager>& logManager, 
                            QObject* parent)
    : QAbstractTableModel(parent)
    , m_logManager(logManager)
    , m_levelFilter(-1)  // Без фильтра по умолчанию
{
    // Получаем все записи журнала
    m_filteredEntries = m_logManager->getAllEntries();
}

LogTableModel::~LogTableModel() {
}

int LogTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    return m_filteredEntries.size();
}

int LogTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    return ColumnCount;
}

QVariant LogTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || 
        index.row() < 0 || 
        index.row() >= m_filteredEntries.size() || 
        index.column() < 0 || 
        index.column() >= ColumnCount) {
        return QVariant();
    }
    
    const LogEntry& entry = m_filteredEntries[index.row()];
    
    switch (role) {
        case Qt::DisplayRole: {
            switch (index.column()) {
                case TimestampColumn:
                    return entry.timestamp.toString("dd/MM/yyyy hh:mm:ss");
                    
                case CategoryColumn:
                    return entry.category;
                    
                case MessageColumn:
                    return entry.message;
                    
                case ValueColumn:
                    return entry.value;
                    
                default:
                    return QVariant();
            }
            break;
        }
        
        case Qt::TextAlignmentRole: {
            switch (index.column()) {
                case TimestampColumn:
                    return Qt::AlignCenter;
                    
                case CategoryColumn:
                    return Qt::AlignCenter;
                    
                case MessageColumn:
                    return Qt::AlignLeft | Qt::AlignVCenter;
                    
                case ValueColumn:
                    return Qt::AlignCenter;
                    
                default:
                    return Qt::AlignLeft | Qt::AlignVCenter;
            }
            break;
        }
        
        case Qt::BackgroundRole: {
            // Цвет фона в зависимости от статуса записи
            return getStatusColor(entry.status);
        }
        
        case Qt::ForegroundRole: {
            // Цвет текста в зависимости от статуса записи
            if (entry.status == LogStatus::Error) {
                return QColor(Qt::white);
            } else if (entry.status == LogStatus::Normal) {
                return QColor(Qt::black);
            } else {
                return QColor(Qt::black);
            }
        }
        
        case Qt::ToolTipRole: {
            // Подсказка с полным текстом записи
            QString tooltip = QString("%1\n%2\n%3")
                              .arg(entry.timestamp.toString("dd/MM/yyyy hh:mm:ss"))
                              .arg(entry.category)
                              .arg(entry.message);
            
            if (!entry.value.isEmpty()) {
                tooltip += QString("\nЗначение: %1").arg(entry.value);
            }
            
            return tooltip;
        }
        
        case Qt::FontRole: {
            // Выделяем текст жирным для ошибок
            if (entry.level == LogLevel::Error) {
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

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }
    
    switch (role) {
        case Qt::DisplayRole: {
            switch (section) {
                case TimestampColumn:
                    return QString("Дата/время");
                    
                case CategoryColumn:
                    return QString("Категория");
                    
                case MessageColumn:
                    return QString("Сообщение");
                    
                case ValueColumn:
                    return QString("Значение");
                    
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

Qt::ItemFlags LogTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    // Все элементы не редактируемые
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void LogTableModel::onLogEntryAdded(const LogEntry& entry) {
    // Проверяем, соответствует ли новая запись фильтрам
    bool matchesFilters = true;
    
    if (!m_categoryFilter.isEmpty() && entry.category != m_categoryFilter) {
        matchesFilters = false;
    }
    
    if (m_levelFilter >= 0 && static_cast<int>(entry.level) != m_levelFilter) {
        matchesFilters = false;
    }
    
    // Если соответствует, добавляем в модель
    if (matchesFilters) {
        beginInsertRows(QModelIndex(), m_filteredEntries.size(), m_filteredEntries.size());
        m_filteredEntries.append(entry);
        endInsertRows();
    }
}

void LogTableModel::onLogCleared() {
    beginResetModel();
    m_filteredEntries.clear();
    endResetModel();
}

void LogTableModel::setCategoryFilter(const QString& category) {
    if (m_categoryFilter != category) {
        m_categoryFilter = category;
        applyFilters();
    }
}

void LogTableModel::setLevelFilter(int level) {
    if (m_levelFilter != level) {
        m_levelFilter = level;
        applyFilters();
    }
}

void LogTableModel::refresh() {
    beginResetModel();
    // Получаем все записи журнала
    if (m_categoryFilter.isEmpty() && m_levelFilter < 0) {
        // Без фильтров
        m_filteredEntries = m_logManager->getAllEntries();
    } else {
        // С фильтрами
        applyFilters();
    }
    endResetModel();
}

QColor LogTableModel::getStatusColor(LogStatus status) const {
    switch (status) {
        case LogStatus::Normal:
            // Для нормального статуса используем градацию цветов в зависимости от четности строки
            return QColor(204, 255, 204); // Светло-зеленый
            
        case LogStatus::Error:
            return QColor(255, 102, 102); // Светло-красный
            
        default:
            return QColor(Qt::white);
    }
}

void LogTableModel::applyFilters() {
    beginResetModel();
    
    QVector<LogEntry> allEntries = m_logManager->getAllEntries();
    m_filteredEntries.clear();
    
    for (const LogEntry& entry : allEntries) {
        bool matchesFilters = true;
        
        if (!m_categoryFilter.isEmpty() && entry.category != m_categoryFilter) {
            matchesFilters = false;
        }
        
        if (m_levelFilter >= 0 && static_cast<int>(entry.level) != m_levelFilter) {
            matchesFilters = false;
        }
        
        if (matchesFilters) {
            m_filteredEntries.append(entry);
        }
    }
    
    endResetModel();
}

} // namespace ParamControl
