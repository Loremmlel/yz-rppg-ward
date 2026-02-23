#pragma once
#include <QMetaType>

/**
 * @brief 核心生命体征数据模型
 * 定义基本的健康参数结构，用于在 Service、Controller 与 View 之间传递。
 */
struct VitalData {
    int heartRate; ///< 心率 (bpm)
    int SpO2; ///< 血氧饱和度 (%)
    int respirationRate; ///< 呼吸频率 (rpm)

    VitalData() : heartRate(0), SpO2(0), respirationRate(0) {
    }

    VitalData(int hr, int sp, int rr)
        : heartRate(hr), SpO2(sp), respirationRate(rr) {
    }
};

Q_DECLARE_METATYPE(VitalData)
