#pragma once
#include <QMetaType>
#include <optional>

/**
 * @brief 监测指标数据快照（值对象）
 *
 * 包含服务器通过 WebSocket 推送的 HR（心率）和 SQI（信号质量指数）。
 * 在 Service、Controller 与 View 之间传递，不持有状态，不触发副作用。
 */
struct MetricsData {
    std::optional<double> hr;   ///< 心率，单位 bpm
    std::optional<double> sqi;  ///< 信号质量指数，0-1

    MetricsData() = default;

    MetricsData(std::optional<double> h, std::optional<double> s)
        : hr(h), sqi(s)
    {}
};

Q_DECLARE_METATYPE(MetricsData)

