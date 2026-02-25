#pragma once
#include <QMetaType>

/**
 * @brief 生命体征数据快照（值对象）
 *
 * 在 Service、Controller 与 View 之间传递，不持有状态，不触发副作用。
 */
struct VitalData {
    int heartRate;      ///< 心率，单位 bpm
    int SpO2;           ///< 血氧饱和度，单位 %
    int respirationRate; ///< 呼吸频率，单位 rpm

    VitalData() : heartRate(0), SpO2(0), respirationRate(0) {}

    VitalData(int hr, int sp, int rr)
        : heartRate(hr), SpO2(sp), respirationRate(rr) {}
};

Q_DECLARE_METATYPE(VitalData)
