#include "XmlParser.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <stdexcept>

XmlParser::XmlParser(QObject* parent)
    : QObject(parent)
{
}

XmlParser::~XmlParser() {
}

QByteArray XmlParser::createParameterRequest(const SotmRequestParams& params) const {
    // Создаем XML-запрос
    QString xmlRequest = createXmlRequest(params);
    
    // Преобразуем в UTF-8 байты
    return xmlRequest.toUtf8();
}

QString XmlParser::createXmlRequest(const SotmRequestParams& params) const {
    QString xmlRequest;
    QXmlStreamWriter writer(&xmlRequest);
    
    // Настройки для красивого форматирования
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);
    
    // Начало документа
    writer.writeStartDocument();
    
    // Корневой элемент
    writer.writeStartElement("SotmDialog");
    writer.writeAttribute("BodyType", "Query");
    
    // Номер КА
    writer.writeTextElement("Ka", QString::number(params.kaNumber));
    
    // Разбираем номер ЗС
    if (params.zsNumber < 1000) {
        // Трехзначный номер ЗС
        QString zsStr = QString::number(params.zsNumber).rightJustified(3, '0');
        writer.writeTextElement("Nip", QString(zsStr[0]));
        
        if (params.zsNumber == 0) {
            writer.writeTextElement("Kts", "00");
        } else {
            writer.writeTextElement("Kts", QString(zsStr[1]) + QString(zsStr[2]));
        }
    } else {
        // Четырехзначный номер ЗС (для имитаторов)
        QString zsStr = QString::number(params.zsNumber).rightJustified(4, '0');
        writer.writeTextElement("Nip", QString(zsStr[0]) + QString(zsStr[1]));
        writer.writeTextElement("Kts", QString(zsStr[2]) + QString(zsStr[3]));
    }
    
    // Начало блока с параметрами
    writer.writeStartElement("Params");
    writer.writeAttribute("ValueType", "Last");
    writer.writeAttribute("Interval", QString::number(params.updateIntervalMs));
    writer.writeAttribute("FindNameBehaviour", "1");
    
    // Список параметров
    for (const auto& paramName : params.parameterNames) {
        writer.writeStartElement("Item");
        writer.writeAttribute("Index", paramName);
        writer.writeEndElement(); // Item
    }
    
    // Завершение блока с параметрами
    writer.writeEndElement(); // Params
    
    // Завершение документа
    writer.writeEndElement(); // SotmDialog
    writer.writeEndDocument();
    
    return xmlRequest;
}

QVector<ParameterValue> XmlParser::parseParameterResponse(const QByteArray& response) const {
    QVector<ParameterValue> result;
    
    QXmlStreamReader reader(response);
    
    try {
        // Проверка на правильный формат XML
        if (!reader.readNextStartElement() || reader.name() != "SotmDialog") {
            throw std::runtime_error("Неверный формат XML: ожидался SotmDialog");
        }
        
        // Проверка атрибута BodyType
        QStringRef bodyType = reader.attributes().value("BodyType");
        if (bodyType != "Answer") {
            throw std::runtime_error("Неверный тип ответа: " + bodyType.toString().toStdString());
        }
        
        // Переходим к элементу Params
        bool foundParams = false;
        while (reader.readNextStartElement()) {
            if (reader.name() == "Params") {
                foundParams = true;
                break;
            } else {
                reader.skipCurrentElement();
            }
        }
        
        if (!foundParams) {
            throw std::runtime_error("Элемент Params не найден");
        }
        
        // Читаем элементы Item
        while (reader.readNextStartElement()) {
            if (reader.name() == "Item") {
                // Получаем имя параметра из атрибута Index
                QString paramName = reader.attributes().value("Index").toString();
                QString paramValue = "Не сформирован";
                
                // Ищем Value внутри Item
                while (reader.readNextStartElement()) {
                    if (reader.name() == "Value") {
                        // Проверяем атрибут State
                        QStringRef state = reader.attributes().value("State");
                        if (state == "-1") {
                            // Параметр не сформирован
                            reader.skipCurrentElement();
                        } else {
                            // Читаем значение
                            paramValue = reader.readElementText();
                        }
                    } else {
                        reader.skipCurrentElement();
                    }
                }
                
                // Добавляем параметр в результат
                ParameterValue pv;
                pv.name = paramName;
                pv.value = paramValue;
                result.append(pv);
            } else {
                reader.skipCurrentElement();
            }
        }
    } catch (const std::exception& e) {
        // Логируем ошибку и прокидываем исключение дальше
        emit parsingError(QString("Ошибка при разборе XML: %1").arg(e.what()));
        throw;
    }
    
    // Проверка на ошибки парсера
    if (reader.hasError()) {
        QString errorMsg = QString("Ошибка при разборе XML: %1").arg(reader.errorString());
        emit parsingError(errorMsg);
        throw std::runtime_error(errorMsg.toStdString());
    }
    
    return result;
}
