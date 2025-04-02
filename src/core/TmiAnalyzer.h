#pragma once

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include <memory>
#include <deque>
#include <mutex>

namespace ParamControl {

/**
 * @brief Структура для хранения истории значений параметра СЕК
 */
struct SekHistoryEntry {
    QDateTime timestamp;  ///< Время получения значения
    int value;            ///< Значение параметра СЕК
    bool valid;           ///< Признак валидности значения
};

/**
 * @brief Класс для анализа телеметрии и обнаружения аномалий в параметре СЕК
 * 
 * Этот класс отвечает за эвристический анализ значений параметра СЕК для
 * обнаружения сбоев в поступлении телеметрии. Основные методы анализа:
 * 1. Проверка скорости изменения СЕК (слишком быстро или слишком медленно)
 * 2. Обнаружение "застревания" (значение не изменяется в течение длительного времени)
 * 3. Проверка на аномальные паттерны в последовательности значений
 */
class TmiAnalyzer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param parent Родительский объект
     */
    explicit TmiAnalyzer(QObject* parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~TmiAnalyzer();
    
    /**
     * @brief Инициализация анализатора
     * @param anomalyDetectionThreshold Порог обнаружения аномалий (количество аномальных значений)
     * @param historySize Размер истории значений
     * @param maxDelta Максимально допустимая дельта между значениями СЕК
     * @param maxNoChangeCount Максимальное количество подряд отсутствия изменений
     */
    void initialize(int anomalyDetectionThreshold = 3, 
                    int historySize = 10, 
                    int maxDelta = 10,
                    int maxNoChangeCount = 12);
    
    /**
     * @brief Анализ нового значения параметра СЕК
     * @param sekValue Строковое представление значения СЕК
     * @return true, если телеметрия в норме, false - при обнаружении аномалии
     */
    bool analyzeSek(const QString& sekValue);
    
    /**
     * @brief Получение текущего статуса телеметрии
     * @return true, если телеметрия в норме, false - при наличии аномалии
     */
    bool isTmiOk() const;
    
    /**
     * @brief Сброс состояния анализатора
     */
    void reset();

signals:
    /**
     * @brief Сигнал изменения статуса телеметрии
     * @param status true, если телеметрия в норме, false - при наличии аномалии
     */
    void tmiStatusChanged(bool status);
    
    /**
     * @brief Сигнал обнаружения аномалии
     * @param type Тип аномалии (1 - застревание, 2 - слишком большая дельта, 3 - неправильный формат)
     * @param message Описание аномалии
     */
    void anomalyDetected(int type, const QString& message);

private:
    std::deque<SekHistoryEntry> m_sekHistory;        ///< История значений параметра СЕК
    int m_anomalyCount;                              ///< Счетчик обнаруженных аномалий
    int m_anomalyDetectionThreshold;                 ///< Порог обнаружения аномалий
    int m_historySize;                               ///< Размер истории значений
    int m_maxDelta;                                  ///< Максимально допустимая дельта между значениями СЕК
    int m_maxNoChangeCount;                          ///< Максимальное количество подряд отсутствия изменений
    int m_sameValueCount;                            ///< Счетчик подряд одинаковых значений
    std::atomic<bool> m_tmiOk;                       ///< Флаг нормального состояния телеметрии
    int m_lastSekValue;                              ///< Последнее значение параметра СЕК
    bool m_firstCheck;                               ///< Флаг первой проверки
    
    mutable std::mutex m_mutex;                      ///< Мьютекс для защиты доступа к данным
    
    /**
     * @brief Проверка на застревание значения СЕК
     * @param value Текущее значение СЕК
     * @return true, если обнаружено застревание
     */
    bool checkForStuck(int value);
    
    /**
     * @brief Проверка на слишком большую дельту между значениями
     * @param value Текущее значение СЕК
     * @return true, если обнаружена слишком большая дельта
     */
    bool checkForLargeDelta(int value);
    
    /**
     * @brief Обновление состояния телеметрии
     * @param status Новый статус телеметрии
     */
    void updateTmiStatus(bool status);
};

} // namespace ParamControl
