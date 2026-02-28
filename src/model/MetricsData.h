#pragma once
#include <QMetaType>

/**
 * @brief 监测指标数据快照（值对象）
 *
 * 包含服务器通过 WebSocket 推送的 HR（心率）和 SQI（信号质量指数）。
 * 在 Service、Controller 与 View 之间传递，不持有状态，不触发副作用。
 */
struct MetricsData {
    int heartRate;  ///< 心率，单位 bpm
    int sqi;        ///< 信号质量指数，0-100

    MetrMetricsData()
        : heartRate(0), sqi(0)
    {}

    MetricsData(int hr, int sq)
        : heartRate(hr), sqi(sq)
    {}
};

Q_DECLARE_METATYPE(MetricsData)

