#include "TmiAnalyzer.h"
#include <QDebug>

namespace ParamControl {

TmiAnalyzer::TmiAnalyzer(QObject* parent)
    : QObject(parent)
    , m_anomalyCount(0)
    , m_anomalyDetectionThreshold(3)
    , m_historySize(10)
    , m_maxDelta(10)
    , m_maxNoChangeCount(12)
    , m_sameValueCount(0)
    , m_tmiOk(true)
    , m_lastSekValue(0)
    , m_firstCheck(true)
{
}

TmiAnalyzer::~TmiAnalyzer() {
}

void TmiAnalyzer::initialize(int anomalyDetectionThreshold, int historySize, 
                             int maxDelta, int maxNoChangeCount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_anomalyDetectionThreshold = anomalyDetectionThreshold;
    m_historySize = historySize;
    m_maxDelta = maxDelta;
    m_maxNoChangeCount = maxNoChangeCount;
    
    reset();
}

bool TmiAnalyzer::analyzeSek(const QString& sekValue) {
    bool sekOk = true;
    int sekInt = 0;
    
    // Попытка преобразования строкового значения в целое число
    try {
        sekInt = sekValue.toInt(&sekOk);
    } catch (...) {
        sekOk = false;
    }
    
    // Если преобразование не удалось, считаем это аномалией
    if (!sekOk) {
        m_anomalyCount++;
        emit anomalyDetected(3, "Неверный формат значения СЕК: " + sekValue);
        
        // Добавляем невалидное значение в историю
        SekHistoryEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.value = -1;
        entry.valid = false;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_sekHistory.push_back(entry);
            if (m_sekHistory.size() > static_cast<size_t>(m_historySize)) {
                m_sekHistory.pop_front();
            }
        }
        
        // Если количество аномалий превысило порог, обновляем статус телеметрии
        if (m_anomalyCount >= m_anomalyDetectionThreshold) {
            updateTmiStatus(false);
        }
        
        return false;
    }
    
    // Для первой проверки просто запоминаем значение
    if (m_firstCheck) {
        m_firstCheck = false;
        m_lastSekValue = sekInt;
        
        SekHistoryEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.value = sekInt;
        entry.valid = true;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_sekHistory.push_back(entry);
        }
        
        return true;
    }
    
    // Проверяем на застревание значения
    bool stuckDetected = checkForStuck(sekInt);
    
    // Проверяем на слишком большую дельту
    bool largeDeltaDetected = checkForLargeDelta(sekInt);
    
    // Добавляем значение в историю
    SekHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.value = sekInt;
    entry.valid = true;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sekHistory.push_back(entry);
        if (m_sekHistory.size() > static_cast<size_t>(m_historySize)) {
            m_sekHistory.pop_front();
        }
    }
    
    // Обновляем последнее значение
    m_lastSekValue = sekInt;
    
    // Если обнаружены аномалии, увеличиваем счетчик
    if (stuckDetected || largeDeltaDetected) {
        m_anomalyCount++;
    } else {
        // Если аномалий нет, сбрасываем счетчик
        m_anomalyCount = 0;
    }
    
    // Если количество аномалий превысило порог, обновляем статус телеметрии
    if (m_anomalyCount >= m_anomalyDetectionThreshold) {
        updateTmiStatus(false);
    } else if (m_anomalyCount == 0 && !m_tmiOk) {
        // Если аномалий нет, а телеметрия считалась сбойной, обновляем статус
        updateTmiStatus(true);
    }
    
    return !stuckDetected && !largeDeltaDetected;
}

bool TmiAnalyzer::isTmiOk() const {
    return m_tmiOk;
}

void TmiAnalyzer::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_sekHistory.clear();
    m_anomalyCount = 0;
    m_sameValueCount = 0;
    m_lastSekValue = 0;
    m_firstCheck = true;
    
    updateTmiStatus(true);
}

bool TmiAnalyzer::checkForStuck(int value) {
    // Если значение не изменилось, увеличиваем счетчик
    if (value == m_lastSekValue) {
        m_sameValueCount++;
        
        // Если значение не изменяется слишком долго, считаем это аномалией
        if (m_sameValueCount >= m_maxNoChangeCount) {
            emit anomalyDetected(1, QString("Значение СЕК не изменяется в течение %1 проверок").arg(m_sameValueCount));
            return true;
        }
    } else {
        // Если значение изменилось, сбрасываем счетчик
        m_sameValueCount = 0;
    }
    
    return false;
}

bool TmiAnalyzer::checkForLargeDelta(int value) {
    int delta = 0;
    
    // Вычисляем дельту между текущим и предыдущим значениями
    if (value > m_lastSekValue) {
        delta = value - m_lastSekValue;
    } else if (value < m_lastSekValue) {
        // Если значение меньше предыдущего, предполагаем, что произошел переход через минуту
        delta = value + 60 - m_lastSekValue;
    }
    
    // Если дельта слишком большая, считаем это аномалией
    if (delta >= m_maxDelta) {
        emit anomalyDetected(2, QString("Слишком большое изменение значения СЕК: %1 -> %2 (дельта: %3)")
                            .arg(m_lastSekValue).arg(value).arg(delta));
        return true;
    }
    
    return false;
}

void TmiAnalyzer::updateTmiStatus(bool status) {
    // Если статус изменился, эмитируем сигнал
    if (status != m_tmiOk) {
        m_tmiOk = status;
        emit tmiStatusChanged(status);
    }
}

} // namespace ParamControl
