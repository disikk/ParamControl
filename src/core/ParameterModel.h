#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QSet>
#include <QMap>
#include <memory>
#include <mutex>

#include "Parameter.h"

class ParameterModel : public QObject {
    Q_OBJECT

public:
    explicit ParameterModel(QObject* parent = nullptr);
    ~ParameterModel();

    // Работа с параметрами
    bool addParameter(const std::shared_ptr<Parameter>& parameter);
    bool removeParameter(const QString& name, ParameterType type);
    std::shared_ptr<Parameter> getParameter(const QString& name, ParameterType type) const;
    QVector<std::shared_ptr<Parameter>> getAllParameters() const;
    QVector<QString> getAllParameterNames() const;
    
    // Динамическое обновление параметров
    bool updateParameter(const QString& name, ParameterType type, const QVariant& targetValue);
    bool updateParameterSound(const QString& name, ParameterType type, bool enabled, const QString& soundFile = QString());
    
    // Сохранение и загрузка параметров в формате JSON
    bool saveParameters(const QString& filename) const;
    bool loadParameters(const QString& filename);

    // Проверка параметров на соответствие условиям
    void checkParameters(const QVector<ParameterValue>& values);

signals:
    // Сигналы для оповещения о изменениях в модели
    void parameterAdded(const QString& name, ParameterType type);
    void parameterRemoved(const QString& name, ParameterType type);
    void parameterUpdated(const QString& name, ParameterType type);
    void parameterSoundChanged(const QString& name, ParameterType type, bool enabled);
    void parameterStatusChanged(const QString& name, ParameterType type, bool status);
    void parameterValueChanged(const QString& name, const QVariant& value);

private:
    mutable std::mutex m_mutex; // Мьютекс для безопасного доступа из разных потоков
    QVector<std::shared_ptr<Parameter>> m_parameters;
};
