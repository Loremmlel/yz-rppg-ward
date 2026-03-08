#pragma once

#include <QWidget>
#include <optional>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QValueAxis>

/**
 * @brief 单项指标近 60 秒折线图（Qt Charts 实现）
 *
 * - 保存最近 60 个数据点（每秒一个）
 * - null 值令折线断开（用独立 QLineSeries 分段）
 * - 每个数据点携带 lowQuality 标记，低质量区间折线下方填充红色渐变
 * - AxisMode::ElasticFrom100：Y 轴初始 [0,100]，数据超出时弹性扩展（用于 HR）
 * - AxisMode::Fixed01：Y 轴固定 [0,1]（用于 SQI）
 *
 * 性能优化：
 * - 使用持久化 series 对象池（按需 show/hide），避免每帧 removeAllSeries()+重建
 * - 用单调递增 x + 滑动窗口替代每帧重对齐所有点 x 坐标的遍历
 * - 拓扑不变时只做 replace()，拓扑变化时才重建 series 池
 */
class MetricChart : public QWidget {
    Q_OBJECT

public:
    enum class AxisMode {
        ElasticFrom100, ///< 初始 [0,100]，数据超出时弹性扩展 (HR)
        Fixed01 ///< 固定 [0,1] (SQI)
    };

    explicit MetricChart(QColor lineColor,
                         AxisMode axisMode = AxisMode::ElasticFrom100,
                         QWidget *parent = nullptr);

    /**
     * @brief 追加一个新数据点
     * @param value      数值，nullopt 表示缺失（折线断开）
     * @param lowQuality 该时刻是否信号质量不佳（折线下方填充红色渐变）
     */
    void addDataPoint(std::optional<double> value, bool lowQuality = false);

    /** 清空所有历史数据并重置图表 */
    void clearData();

private:
    // ── 数据结构 ────────────────────────────────────────────
    struct Point {
        double x{0.0};
        std::optional<double> y;
        bool lowQuality{false};
    };

    // 描述当前数据的分段拓扑。
    // 用全局 x 坐标值（非数组下标）描述段边界，
    // 这样 removeFirst() 后下标偏移不会影响拓扑比较。
    struct SegRange {
        double xFrom{0.0};
        double xTo{0.0};

        bool operator==(const SegRange &o) const {
            return xFrom == o.xFrom && xTo == o.xTo;
        }
    };

    struct Topology {
        QList<SegRange> validSegs; ///< 有效（非null）连续段，按 x 坐标描述
        QList<SegRange> lowQualitySegs; ///< lowQuality 连续子段，按 x 坐标描述

        bool operator==(const Topology &o) const {
            return validSegs == o.validSegs && lowQualitySegs == o.lowQualitySegs;
        }

        bool operator!=(const Topology &o) const { return !(*this == o); }
    };

    // ── 核心方法 ─────────────────────────────────────────────
    /** 计算当前数据的分段拓扑 */
    [[nodiscard]] Topology calcTopology() const;

    /** 完全重建 series 池（拓扑变化时调用）*/
    void rebuildSeriesPool(const Topology &topo, double yMin);

    /** 仅更新 series 点数据（拓扑不变时调用，性能路径）*/
    void updateSeriesData(const Topology &topo, double yMin);

    /** 更新 Y 轴范围（带滞后抑制，避免频繁微调触发重绘）*/
    void updateYAxis(double yMin, double yMax);

    /** 计算 Y 轴范围，返回 {yMin, yMax} */
    [[nodiscard]] std::pair<double, double> calcYRange() const;

    static constexpr int kMaxPoints = 60;

    // ── 数据 ─────────────────────────────────────────────────
    QList<Point> m_points;
    double m_xCounter{0.0}; ///< 单调递增 x，避免每帧重对齐

    // ── Y 轴缓存（滞后抑制） ──────────────────────────────────
    double m_cachedYMin{0.0};
    double m_cachedYMax{100.0};
    static constexpr double kYHysteresis = 0.5; ///< 变化小于此值不触发轴更新

    // ── 配置 ─────────────────────────────────────────────────
    QColor m_lineColor;
    AxisMode m_axisMode;

    // ── Qt Charts 对象 ───────────────────────────────────────
    QChart *m_chart{nullptr};
    QChartView *m_chartView{nullptr};
    QValueAxis *m_axisX{nullptr};
    QValueAxis *m_axisY{nullptr};

    // ── Series 持久化池 ──────────────────────────────────────
    // 每个有效段对应一个 QLineSeries
    QList<QLineSeries *> m_lineSeries;
    // 每个 lowQuality 子段对应一个 QAreaSeries（含内部 upper/lower QLineSeries）
    QList<QAreaSeries *> m_areaSeries;

    // 上一帧的拓扑，用于判断是否需要重建
    Topology m_lastTopology;
    bool m_poolInitialized{false};
};