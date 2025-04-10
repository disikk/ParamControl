// src/core/ParameterModel.cpp
#include "ParameterModel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextStream>
#include <QDateTime>
#include <QDebug> // Для логирования

// Включаем заголовки для всех типов параметров
#include "ParameterEquals.h"
#include "ParameterNotEquals.h"
#include "ParameterInLimits.h"
#include "ParameterOutOfLimits.h"
#include "ParameterChanged.h"

namespace ParamControl {

ParameterModel::ParameterModel(QObject* parent)
    : QObject(parent)
{
}

ParameterModel::~ParameterModel() {
}

bool ParameterModel::addParameter(const std::shared_ptr<Parameter>& parameter) {
    if (!parameter) {
        qWarning() << "ParameterModel::addParameter: Attempted to add a null parameter.";
        return false;
    }

    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);

    // Проверяем, не существует ли уже параметр с таким именем и типом
    for (const auto& existingParam : m_parameters) {
        if (existingParam->getName() == parameter->getName() &&
            existingParam->getType() == parameter->getType()) {
            qWarning() << "ParameterModel::addParameter: Parameter" << parameter->getName()
                       << "with type" << static_cast<int>(parameter->getType()) << "already exists.";
            return false; // Параметр с таким именем и типом уже существует
        }
    }

    // Добавляем параметр в список
    m_parameters.append(parameter);

    // Сигнализируем о добавлении параметра
    // Эмитируем сигнал *после* разблокировки мьютекса, если это возможно
    // Но в данном случае изменение m_parameters должно быть атомарным с сигналом
    emit parameterAdded(parameter->getName(), parameter->getType());
    qDebug() << "ParameterModel: Added parameter" << parameter->getName() << "type" << static_cast<int>(parameter->getType());


    return true;
}

bool ParameterModel::removeParameter(const QString& name, ParameterType type) {
    std::shared_ptr<Parameter> removedParam = nullptr;
    {
        // Блокируем мьютекс для безопасного доступа к параметрам
        std::lock_guard<std::mutex> lock(m_mutex);

        // Ищем параметр с указанным именем и типом
        for (int i = 0; i < m_parameters.size(); ++i) {
            if (m_parameters[i]->getName() == name && m_parameters[i]->getType() == type) {
                // Сохраняем указатель для сигнала перед удалением
                removedParam = m_parameters[i];
                // Удаляем параметр из списка
                m_parameters.removeAt(i);
                break; // Выходим из цикла после удаления
            }
        }
    } // Мьютекс разблокируется здесь

    if (removedParam) {
        // Сигнализируем об удалении параметра
        emit parameterRemoved(name, type);
        qDebug() << "ParameterModel: Removed parameter" << name << "type" << static_cast<int>(type);
        return true;
    } else {
        qWarning() << "ParameterModel::removeParameter: Parameter" << name
                   << "with type" << static_cast<int>(type) << "not found.";
        return false;
    }
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

    return nullptr; // Параметр не найден
}

QVector<std::shared_ptr<Parameter>> ParameterModel::getParametersByName(const QString& name) const {
     // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);
    QVector<std::shared_ptr<Parameter>> result;
    for (const auto& parameter : m_parameters) {
        if (parameter->getName() == name) {
            result.append(parameter);
        }
    }
    return result;
}


bool ParameterModel::updateParameter(const QString& name, ParameterType type,
                                    const QVariant& targetValue, const QString& description) {
    std::shared_ptr<Parameter> parameter = nullptr;
    {
        // Блокируем мьютекс для безопасного доступа к параметрам
        std::lock_guard<std::mutex> lock(m_mutex);

        // Ищем параметр с указанным именем и типом
        for (const auto& p : m_parameters) {
            if (p->getName() == name && p->getType() == type) {
                parameter = p;
                break;
            }
        }

        if (parameter) {
            // Обновляем целевое значение и описание параметра
            parameter->setTargetValue(targetValue);
            parameter->setDescription(description); // Обновляем описание
        }
    } // Мьютекс разблокируется здесь

    if (parameter) {
        // Сигнализируем об изменении параметра
        emit parameterUpdated(name, type);
         qDebug() << "ParameterModel: Updated parameter" << name << "type" << static_cast<int>(type);
        return true;
    } else {
         qWarning() << "ParameterModel::updateParameter: Parameter" << name
                   << "with type" << static_cast<int>(type) << "not found.";
        return false;
    }
}

bool ParameterModel::updateParameterSound(const QString& name, ParameterType type,
                                         bool enabled, const QString& soundFile) {
    std::shared_ptr<Parameter> parameter = nullptr;
    {
        // Блокируем мьютекс для безопасного доступа к параметрам
        std::lock_guard<std::mutex> lock(m_mutex);

        // Ищем параметр с указанным именем и типом
        for (const auto& p : m_parameters) {
            if (p->getName() == name && p->getType() == type) {
                parameter = p;
                break;
            }
        }

        if (parameter) {
            // Обновляем настройки звука
            parameter->setSoundEnabled(enabled);

            if (!soundFile.isNull()) { // Проверяем на null, а не на empty, чтобы можно было передать пустой путь
                parameter->setSoundFile(soundFile);
            }
        }
    } // Мьютекс разблокируется здесь

    if (parameter) {
        // Сигнализируем об изменении параметра
        emit parameterSoundChanged(name, type, enabled);
         qDebug() << "ParameterModel: Updated sound for parameter" << name << "type" << static_cast<int>(type);
        return true;
    } else {
         qWarning() << "ParameterModel::updateParameterSound: Parameter" << name
                   << "with type" << static_cast<int>(type) << "not found.";
        return false;
    }
}

QVector<std::shared_ptr<Parameter>> ParameterModel::getAllParameters() const {
    // Блокируем мьютекс для безопасного доступа к параметрам
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_parameters; // Возвращаем копию вектора указателей
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

    // Сортируем имена для единообразия
    std::sort(names.begin(), names.end());

    return names;
}

void ParameterModel::checkParameters(const QVector<ParameterValue>& values) {
    // Создаем временную мапу для быстрого поиска по имени
    QMap<QString, QVariant> valueMap;
    for (const auto& value : values) {
        valueMap[value.name] = value.value;
    }

    // Используем копию списка параметров для безопасной итерации
    // и для возможности модификации оригинала внутри цикла (хотя здесь не модифицируем)
    QVector<std::shared_ptr<Parameter>> paramsCopy;
    {
        // Блокируем мьютекс только для копирования списка
        std::lock_guard<std::mutex> lock(m_mutex);
        paramsCopy = m_parameters;
    }

    // Проверяем каждый параметр из копии
    for (const auto& parameter : paramsCopy) {
        QString name = parameter->getName();

        // Пропускаем параметр, если для него нет значения в текущем ответе
        if (!valueMap.contains(name)) {
            // Можно установить статус Unknown, если значение отсутствует?
            // parameter->updateValue(QVariant()); // Или специальный маркер отсутствия значения
            // emit parameterStatusChanged(name, parameter->getType(), ParameterStatus::Unknown);
            continue;
        }

        // Получаем значение параметра
        QVariant value = valueMap[name];

        // Обновляем значение параметра и проверяем изменение статуса
        // Метод updateValue сам обновит m_status и вернет true, если он изменился
        bool statusChanged = parameter->updateValue(value);

        // Если статус изменился, сигнализируем об этом
        if (statusChanged) {
            emit parameterStatusChanged(name, parameter->getType(), parameter->getStatus());
        }

        // В любом случае сигнализируем об изменении значения (даже если статус не изменился)
        // Это нужно, например, для обновления отображения значения в UI
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
        paramObj["description"] = parameter->getDescription(); // Сохраняем описание

        // Специфичные для типа свойства (целевое значение)
        QVariant target = parameter->getTargetValue();
        QJsonValue targetJsonValue; // Используем QJsonValue для обработки null

        if (target.type() == QVariant::List) {
            QJsonArray array;
            for (const QVariant& item : target.toList()) {
                array.append(QJsonValue::fromVariant(item)); // Используем fromVariant для корректной конвертации
            }
            targetJsonValue = array;
        } else if (!target.isNull()) {
            targetJsonValue = QJsonValue::fromVariant(target); // Используем fromVariant
        } else {
            targetJsonValue = QJsonValue(); // Явно указываем null для типов без targetValue (Changed)
        }

        paramObj["target_value"] = targetJsonValue;

        // Добавляем объект в массив
        parametersArray.append(paramObj);
    }

    // Создаем основной объект с метаданными
    QJsonObject rootObj;
    rootObj["version"] = "1.0"; // Можно вынести в константу
    rootObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["parameters"] = parametersArray;

    // Добавляем информативный комментарий с описанием формата
    QString jsonComment =
        "# Configuration file for ParamControl\n"
        "# This file contains a list of parameters to be monitored\n"
        "# Format: JSON\n"
        "# Fields:\n"
        "#   - name: Parameter name (string)\n"
        "#   - type: Parameter type (0=Equals, 1=NotEquals, 2=InLimits, 3=OutOfLimits, 4=Changed)\n"
        "#   - target_value: Target value for comparison (string, number, boolean, array[2] for limits, or null for Changed)\n"
        "#   - sound_enabled: Whether sound alert is enabled (boolean: true/false)\n"
        "#   - sound_file: Path to sound file for alert (string)\n"
        "#   - description: Human-readable description of parameter (string)\n"
        "# \n"
        "# Note: You can edit this file manually, but make sure the application is not running.\n"
        "# Date: " + QDateTime::currentDateTime().toString(Qt::ISODate) + "\n"
        "# \n\n";

    // Записываем JSON в файл
    QFile file(filename);
    // Используем QIODevice::Truncate, чтобы перезаписать файл
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "ParameterModel::saveParameters: Could not open file for writing:" << filename;
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8"); // Убедимся, что используется UTF-8
    out << jsonComment;
    // Используем toJson с форматированием
    out << QJsonDocument(rootObj).toJson(QJsonDocument::Indented);
    file.close();

    qDebug() << "ParameterModel: Parameters saved to" << filename;
    return true;
}

bool ParameterModel::loadParameters(const QString& filename) {
    // Читаем файл
    QFile file(filename);
    if (!file.exists()) {
         qWarning() << "ParameterModel::loadParameters: File not found:" << filename;
        // Не считаем это критической ошибкой, просто не загружаем параметры
        return true; // Возвращаем true, т.к. отсутствие файла - не сбой загрузки
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qWarning() << "ParameterModel::loadParameters: Could not open file for reading:" << filename;
        return false; // Ошибка открытия файла
    }

    // Пропускаем комментарии в начале файла
    QTextStream in(&file);
    in.setCodec("UTF-8"); // Убедимся, что используется UTF-8
    QString jsonText;
    QString line;
    while (in.readLineInto(&line)) { // Более эффективное чтение
        if (!line.trimmed().startsWith('#')) {
            jsonText += line + "\n";
        }
    }
    file.close();

    // Разбираем JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "ParameterModel::loadParameters: Error parsing JSON file" << filename << ":" << error.errorString() << "at offset" << error.offset;
        return false; // Ошибка парсинга JSON
    }

    if (!doc.isObject()) {
         qWarning() << "ParameterModel::loadParameters: Invalid JSON structure (root is not an object) in file:" << filename;
         return false;
    }

    // Извлекаем массив параметров
    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("parameters") || !rootObj["parameters"].isArray()) {
         qWarning() << "ParameterModel::loadParameters: 'parameters' array not found or invalid in file:" << filename;
         return false;
    }
    QJsonArray parametersArray = rootObj["parameters"].toArray();

    // Очищаем текущий список параметров *перед* добавлением новых
    QVector<std::shared_ptr<Parameter>> loadedParameters; // Временный вектор для загруженных
    bool loadSuccess = true;

    // Создаем параметры
    for (const QJsonValue& value : parametersArray) {
        if (!value.isObject()) {
            qWarning() << "ParameterModel::loadParameters: Invalid item in 'parameters' array (not an object) in file:" << filename;
            loadSuccess = false; // Отмечаем ошибку, но продолжаем загрузку остальных
            continue;
        }
        QJsonObject paramObj = value.toObject();

        // Извлекаем общие свойства с проверками
        if (!paramObj.contains("name") || !paramObj["name"].isString() ||
            !paramObj.contains("type") || !paramObj["type"].isDouble()) { // isDouble т.к. JSON числа
             qWarning() << "ParameterModel::loadParameters: Missing or invalid 'name' or 'type' in parameter object in file:" << filename;
             loadSuccess = false;
             continue;
        }

        QString name = paramObj["name"].toString();
        ParameterType type = static_cast<ParameterType>(paramObj["type"].toInt()); // Преобразуем double в int
        bool soundEnabled = paramObj.value("sound_enabled").toBool(true); // По умолчанию true
        QString soundFile = paramObj.value("sound_file").toString();
        QString description = paramObj.value("description").toString(); // Загружаем описание

        // Извлекаем целевое значение
        QVariant targetValue; // QVariant по умолчанию null
        QJsonValue jsonTargetValue = paramObj.value("target_value"); // Используем value() для обработки отсутствия ключа

        if (jsonTargetValue.isArray()) {
            // Для InLimits/OutOfLimits
            QVariantList list;
            QJsonArray array = jsonTargetValue.toArray();
            for (const QJsonValue& item : array) {
                list.append(item.toVariant()); // Используем toVariant
            }
            targetValue = list;
        } else if (!jsonTargetValue.isNull() && !jsonTargetValue.isUndefined()) {
             // Для Equals/NotEquals
             targetValue = jsonTargetValue.toVariant(); // Используем toVariant
        }
        // Для Changed targetValue остается null (по умолчанию)

        // Создаем параметр с помощью фабричного метода
        std::shared_ptr<Parameter> parameter = Parameter::create(
            name, type, targetValue, soundFile, soundEnabled, description);

        if (parameter) {
            loadedParameters.append(parameter);
        } else {
             qWarning() << "ParameterModel::loadParameters: Failed to create parameter" << name << "from file:" << filename;
             loadSuccess = false; // Ошибка создания параметра
        }
    }

    // Если загрузка прошла успешно (или частично успешно), обновляем модель
    if (!loadedParameters.isEmpty() || loadSuccess) { // Обновляем даже если были ошибки, но что-то загрузилось
         {
             std::lock_guard<std::mutex> lock(m_mutex);
             m_parameters = loadedParameters; // Заменяем старый список новым
         }
         qDebug() << "ParameterModel: Loaded" << loadedParameters.size() << "parameters from" << filename;
         // Оповещаем UI о полной перезагрузке модели
         emit modelReset(); // Сигнал для полного обновления представлений
    }


    return loadSuccess; // Возвращаем true, если не было критических ошибок парсинга JSON
}


} // namespace ParamControl
