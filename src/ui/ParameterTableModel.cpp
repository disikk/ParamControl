#include "ParameterTableModel.h"

namespace ParamControl {

ParameterTableModel::ParameterTableModel(const std::shared_ptr<ParameterModel>& parameterModel, 
                                        QObject* parent)
    : QAbstractTableModel(parent)
    , m_parameterModel(parameterModel)
{
    // Загрузка иконок для отображения статуса звука
    m_soundEnabledIcon = QIcon(":/icons/sound_on.png");
    m_soundDisabledIcon = QIcon(":/icons/sound_off.png");
    
    // Если иконки не загрузились, используем запасной вариант
    if (m_soundEnabledIcon.isNull()) {
        m_soundEnabledIcon = QIcon::fromTheme("audio-volume-high");
    }
    
    if (m_soundDisabledIcon.isNull()) {
        m_soundDisabledIcon = QIcon::fromTheme("audio-volume-muted");
    }
}

ParameterTableModel::~ParameterTableModel() {
}

int ParameterTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    return m_parameterModel->getAllParameters().size();
}

int ParameterTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    return ColumnCount;
}

QVariant ParameterTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || 
        index.row() < 0 || 
        index.row() >= m_parameterModel->getAllParameters().size() || 
        index.column() < 0 || 
        index.column() >= ColumnCount) {
        return QVariant();
    }
    
    const auto& parameters = m_parameterModel->getAllParameters();
    const auto& parameter = parameters[index.row()];
    
    switch (role) {
        case Qt::DisplayRole: {
            switch (index.column()) {
                case SoundColumn:
                    // Столбец звука отображается как иконка
                    return QVariant();
                    
                case NameColumn:
                    return parameter->getName();
                    
                case ConditionColumn:
                    return parameter->getConditionDescription();
                    
                default:
                    return QVariant();
            }
            break;
        }
        
        case Qt::DecorationRole: {
            if (index.column() == SoundColumn) {
                return parameter->isSoundEnabled() ? m_soundEnabledIcon : m_soundDisabledIcon;
            }
            return QVariant();
        }
        
        case Qt::TextAlignmentRole: {
            switch (index.column()) {
                case SoundColumn:
                    return Qt::AlignCenter;
                    
                case NameColumn:
                    return Qt::AlignLeft | Qt::AlignVCenter;
                    
                case ConditionColumn:
                    return Qt::AlignLeft | Qt::AlignVCenter;
                    
                default:
                    return Qt::AlignLeft | Qt::AlignVCenter;
            }
            break;
        }
        
        case Qt::BackgroundRole: {
            // Цвет фона в зависимости от статуса параметра
            return getStatusColor(parameter->getStatus());
        }
        
        case Qt::ForegroundRole: {
            // Цвет текста в зависимости от статуса параметра
            return parameter->getStatus() == ParameterStatus::Error ? 
                   QColor(Qt::white) : QColor(Qt::black);
        }
        
        case Qt::ToolTipRole: {
            // Подсказка с описанием параметра
            QString tooltip = QString("%1\n%2\n\nСтатус: %3\nЗвук: %4")
                              .arg(parameter->getName())
                              .arg(parameter->getConditionDescription())
                              .arg(parameter->getStatus() == ParameterStatus::Error ? 
                                   "Ошибка" : (parameter->getStatus() == ParameterStatus::Ok ? 
                                              "В норме" : "Неизвестно"))
                              .arg(parameter->isSoundEnabled() ? "Включен" : "Отключен");
            
            if (!parameter->getDescription().isEmpty()) {
                tooltip += "\n\nОписание: " + parameter->getDescription();
            }
            
            return tooltip;
        }
        
        case Qt::FontRole: {
            // Выделяем текст жирным, если параметр в ошибке
            if (parameter->getStatus() == ParameterStatus::Error) {
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

QVariant ParameterTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }
    
    switch (role) {
        case Qt::DisplayRole: {
            switch (section) {
                case SoundColumn:
                    return QString("Звук");
                    
                case NameColumn:
                    return QString("Параметр");
                    
                case ConditionColumn:
                    return QString("Условие контроля");
                    
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

Qt::ItemFlags ParameterTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    // Разрешаем редактирование только для столбца звука
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    if (index.column() == SoundColumn) {
        flags |= Qt::ItemIsEditable;
    }
    
    return flags;
}

bool ParameterTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.column() != SoundColumn || role != Qt::EditRole) {
        return false;
    }
    
    const auto& parameters = m_parameterModel->getAllParameters();
    if (index.row() < 0 || index.row() >= parameters.size()) {
        return false;
    }
    
    const auto& parameter = parameters[index.row()];
    
    // Меняем состояние звука параметра
    bool enabled = value.toBool();
    m_parameterModel->updateParameterSound(parameter->getName(), parameter->getType(), enabled);
    
    // Оповещаем о изменении данных
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::DecorationRole});
    
    return true;
}

void ParameterTableModel::onParameterAdded(const QString& name, ParameterType type) {
    Q_UNUSED(name);
    Q_UNUSED(type);
    
    // Оповещаем о добавлении строки
    beginResetModel();
    endResetModel();
}

void ParameterTableModel::onParameterRemoved(const QString& name, ParameterType type) {
    Q_UNUSED(name);
    Q_UNUSED(type);
    
    // Оповещаем о удалении строки
    beginResetModel();
    endResetModel();
}

void ParameterTableModel::onParameterUpdated(const QString& name, ParameterType type) {
    // Находим индекс параметра
    int row = findParameterIndex(name, type);
    if (row >= 0) {
        // Оповещаем о изменении данных
        emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
    }
}

void ParameterTableModel::onParameterSoundChanged(const QString& name, ParameterType type, bool enabled) {
    Q_UNUSED(enabled);
    
    // Находим индекс параметра
    int row = findParameterIndex(name, type);
    if (row >= 0) {
        // Оповещаем о изменении данных
        emit dataChanged(index(row, SoundColumn), index(row, SoundColumn));
    }
}

void ParameterTableModel::onParameterStatusChanged(const QString& name, ParameterType type, ParameterStatus status) {
    Q_UNUSED(status);
    
    // Находим индекс параметра
    int row = findParameterIndex(name, type);
    if (row >= 0) {
        // Оповещаем о изменении данных
        emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
    }
}

QColor ParameterTableModel::getStatusColor(ParameterStatus status) const {
    switch (status) {
        case ParameterStatus::Ok:
            return QColor(Qt::white);
            
        case ParameterStatus::Error:
            return QColor(Qt::red).lighter(150); // Немного светлее красный
            
        case ParameterStatus::Unknown:
            return QColor(Qt::lightGray);
            
        default:
            return QColor(Qt::white);
    }
}

int ParameterTableModel::findParameterIndex(const QString& name, ParameterType type) const {
    const auto& parameters = m_parameterModel->getAllParameters();
    
    for (int i = 0; i < parameters.size(); ++i) {
        if (parameters[i]->getName() == name && parameters[i]->getType() == type) {
            return i;
        }
    }
    
    return -1;
}

} // namespace ParamControl
