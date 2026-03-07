#pragma once

#include <QDateTime>
#include <optional>

/**
 * @brief 历史趋势聚合数据（对应服务端 VitalsTrendDto 的 C++ 映射）
 *
 * 每个实例对应一个 time_bucket 聚合时间窗口的结果。
 */
struct VitalsTrendData {

    // ── 基础生命体征 ─────────────────────────────────────────────
    struct BasicVitals {
        std::optional<double> hrAvg;   ///< 心率均值（bpm）
        std::optional<double> brAvg;   ///< 呼吸率均值（Hz）
        std::optional<double> sqiAvg;  ///< 信号质量指数均值
    };

    // ── HRV 时域（中位数） ────────────────────────────────────────
    struct HrvTimeDomain {
        std::optional<double> sdnnMedian;  ///< SDNN 中位数（ms）
        std::optional<double> rmssdMedian; ///< RMSSD 中位数（ms）
        std::optional<double> sdsdMedian;  ///< SDSD 中位数（ms）
        std::optional<double> pnn50Median; ///< pNN50 中位数
        std::optional<double> pnn20Median; ///< pNN20 中位数
    };

    // ── HRV 频域（均值） ─────────────────────────────────────────
    struct HrvFreqDomain {
        std::optional<double> lfHfRatio; ///< LF/HF 比值（自主神经平衡指标）
        std::optional<double> hfAvg;     ///< HF 均值（副交感活动）
        std::optional<double> lfAvg;     ///< LF 均值（交感 + 副交感活动）
        std::optional<double> vlfAvg;    ///< VLF 均值
        std::optional<double> tpAvg;     ///< 总功率均值
    };

    QDateTime    bucketTime;     ///< 时间桶起始时刻（UTC）
    BasicVitals  basicVitals;
    HrvTimeDomain  hrvTimeDomain;
    HrvFreqDomain  hrvFreqDomain;
};

