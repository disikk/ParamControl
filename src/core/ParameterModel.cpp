#include "ParameterModel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextStream>
#include <QDateTime>

ParameterModel::ParameterModel(QObject* parent)
    : QObject(parent)
{
}

ParameterModel::~ParameterModel() {
}

bool ParameterModel::addParameter(const std::shared_ptr<Parameter>& parameter) {
    if (!parameter) {
        return false;
    }
    
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Проверяем, не существует ли уже параметр с таким именем и типом
    for (const auto& existingParam : m_parameters) {
        if (existingParam->getName() == parameter->getName() && 
            existingParam->getType() == parameter->getType()) {
            return false;
        }
    }
    
    // Добавляем параметр в список
    m_parameters.append(parameter);
    
    // Сигнализируем о добавлении параметра
    emit parameterAdded(parameter->getName(), parameter->getType());
    
    return true;
}

bool ParameterModel::removeParameter(const QString& name, ParameterType type) {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Ищем параметр с указанным именем и типом
    for (int i = 0; i < m_parameters.size(); ++i) {
        if (m_parameters[i]->getName() == name && m_parameters[i]->getType() == type) {
            // Удаляем параметр из списка
            m_parameters.removeAt(i);
            
            // Сигнализируем об удалении параметра
            emit parameterRemoved(name, type);
            
            return true;
        }
    }
    
    return false;
}

std::shared_ptr<Parameter> ParameterModel::getParameter(const QString& name, ParameterType type) const {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Ищем параметр с указанным именем и типом
    for (const auto& parameter : m_parameters) {
        if (parameter->getName() == name && parameter->getType() == type) {
            return parameter;
        }
    }
    
    return nullptr;
}

bool ParameterModel::updateParameter(const QString& name, ParameterType type, 
                                    const QVariant& targetValue) {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Ищем параметр с указанным именем и типом
    for (const auto& parameter : m_parameters) {
        if (parameter->getName() == name && parameter->getType() == type) {
            // Обновляем целевое значение параметра
            parameter->setTargetValue(targetValue);
            
            // Сигнализируем об изменении параметра
            emit parameterUpdated(name, type);
            
            return true;
        }
    }
    
    return false;
}

bool ParameterModel::updateParameterSound(const QString& name, ParameterType type, 
                                         bool enabled, const QString& soundFile) {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Ищем параметр с указанным именем и типом
    for (const auto& parameter : m_parameters) {
        if (parameter->getName() == name && parameter->getType() == type) {
            // Обновляем настройки звука
            parameter->setSoundEnabled(enabled);
            
            if (!soundFile.isEmpty()) {
                parameter->setSoundFile(soundFile);
            }
            
            // Сигнализируем об изменении параметра
            emit parameterSoundChanged(name, type, enabled);
            
            return true;
        }
    }
    
    return false;
}

QVector<std::shared_ptr<Parameter>> ParameterModel::getAllParameters() const {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    return m_parameters;
}

QVector<QString> ParameterModel::getAllParameterNames() const {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    QVector<QString> names;
    QSet<QString> uniqueNames; // Используем Set для уникальности
    
    for (const auto& parameter : m_parameters) {
        QString name = parameter->getName();
        if (!uniqueNames.contains(name)) {
            uniqueNames.insert(name);
            names.append(name);
        }
    }
    
    return names;
}

void ParameterModel::checkParameters(const QVector<ParameterValue>& values) {
    // Создаем временную мапу для быстрого поиска по имени
    QMap<QString, QVariant> valueMap;
    for (const auto& value : values) {
        valueMap[value.name] = value.value;
    }
    
    // Проверяем каждый параметр
    // Используем копию списка параметров для безопасной итерации
    QVector<std::shared_ptr<Parameter>> paramsCopy;
    {
        // Блокируем мьютекс только для копирования списка
        std::lock_guard<std::mutex> lock(m_mutex);
        paramsCopy = m_parameters;
    }
    
    for (const auto& parameter : paramsCopy) {
        QString name = parameter->getName();
        
        // Пропускаем параметр, если для него нет значения
        if (!valueMap.contains(name)) {
            continue;
        }
        
        // Получаем значение параметра
        QVariant value = valueMap[name];
        
        // Проверяем изменение статуса
        bool statusChanged = parameter->updateValue(value);
        
        // Если статус изменился, сигнализируем об этом
        if (statusChanged) {
            emit parameterStatusChanged(name, parameter->getType(), parameter->getStatus());
        }
        
        // В любом случае сигнализируем об изменении значения
        emit parameterValueChanged(name, value);
    }
}

bool ParameterModel::saveParameters(const QString& filename) const {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Создаем JSON-массив для параметров
    QJsonArray parametersArray;
    
    for (const auto& parameter : m_parameters) {
        QJsonObject paramObj;
        
        // Общие свойства
        paramObj["name"] = parameter->getName();
        paramObj["type"] = static_cast<int>(parameter->getType());
        paramObj["sound_enabled"] = parameter->isSoundEnabled();
        paramObj["sound_file"] = parameter->getSoundFile();
        
        // Специфичные для типа свойства
        QJsonValue targetValue;
        QVariant target = parameter->getTargetValue();
        
        // Обрабатываем разные типы данных
        if (target.type() == QVariant::List) {
            QJsonArray array;
            for (const QVariant& item : target.toList()) {
                array.append(item.toString());
            }
            targetValue = array;
        } else if (!target.isNull()) {
            targetValue = target.toString();
        }
        
        paramObj["target_value"] = targetValue;
        
        // Описание для удобства чтения
        paramObj["description"] = parameter->getDescription();
        
        // Добавляем объект в массив
        parametersArray.append(paramObj);
    }
    
    // Создаем основной объект с метаданными
    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    rootObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["parameters"] = parametersArray;
    
    // Добавляем информативный комментарий с описанием формата
    QString jsonComment = 
        "# Configuration file for ParamControl\n"
        "# This file contains a list of parameters to be monitored\n"
        "# Format: JSON\n"
        "# Fields:\n"
        "#   - name: Parameter name\n"
        "#   - type: Parameter type (0 = Equals, 1 = NotEquals, 2 = InLimits, 3 = OutOfLimits, 4 = Changed)\n"
        "#   - target_value: Target value for comparison (string or array of strings)\n"
        "#   - sound_enabled: Whether sound alert is enabled (true/false)\n"
        "#   - sound_file: Path to sound file for alert\n"
        "#   - description: Human-readable description of parameter\n"
        "# \n"
        "# Note: You can edit this file manually, but make sure the application is not running\n"
        "# Date: " + QDateTime::currentDateTime().toString(Qt::ISODate) + "\n"
        "# \n\n";
    
    // Записываем JSON в файл
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << jsonComment;
    out << QJsonDocument(rootObj).toJson(QJsonDocument::Indented);
    file.close();
    
    return true;
}

bool ParameterModel::loadParameters(const QString& filename) {
    // Очищаем текущий список параметров
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_parameters.clear();
    }
    
    // Читаем файл
    QFile file(filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    // Пропускаем комментарии в начале файла
    QTextStream in(&file);
    QString jsonText;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.startsWith('#')) {
            jsonText += line + "\n";
        }
    }
    
    // Разбираем JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing JSON file:" << error.errorString();
        return false;
    }
    
    // Извлекаем массив параметров
    QJsonObject rootObj = doc.object();
    QJsonArray parametersArray = rootObj["parameters"].toArray();
    
    // Создаем параметры
    for (const QJsonValue& value : parametersArray) {
        QJsonObject paramObj = value.toObject();
        
        // Извлекаем общие свойства
        QString name = paramObj["name"].toString();
        ParameterType type = static_cast<ParameterType>(paramObj["type"].toInt());
        bool soundEnabled = paramObj["sound_enabled"].toBool();
        QString soundFile = paramObj["sound_file"].toString();
        
        // Извлекаем целевое значение
        QVariant targetValue;
        QJsonValue jsonTargetValue = paramObj["target_value"];
        
        if (jsonTargetValue.isArray()) {
            QVariantList list;
            QJsonArray array = jsonTargetValue.toArray();
            for (const QJsonValue& item : array) {
                list.append(item.toString());
            }
            targetValue = list;
        } else if (!jsonTargetValue.isNull()) {
            targetValue = jsonTargetValue.toString();
        }
        
        // Создаем параметр
        std::shared_ptr<Parameter> parameter = Parameter::create(name, type, targetValue, soundFile, soundEnabled);
        
        if (parameter) {
            // Добавляем параметр в список
            std::lock_guard<std::mutex> lock(m_mutex);
            m_parameters.append(parameter);
        }
    }
    
    // Оповещаем о загрузке всех параметров
    for (const auto& param : m_parameters) {
        emit parameterAdded(param->getName(), param->getType());
    }
    
    return true;
}
