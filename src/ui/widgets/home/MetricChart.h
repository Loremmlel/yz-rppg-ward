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
 * - Y 轴范围随数据动态扩展（含 15% padding）
 * - 可选：当值低于阈值时，折线与 X 轴之间填充红色渐变（用于 SQI < 0.5）
 * - 当 setLowQualityMode(true) 时，所有分段折线下方叠加红色渐变 area
 */
class MetricChart : public QWidget {
    Q_OBJECT

public:
    explicit MetricChart(QColor lineColor,
                         bool enableLowQualityFill = false,
                         double lowQualityThreshold = 0.5,
                         QWidget *parent = nullptr);

    /** 追加一个新数据点；超过 60 个时自动丢弃最旧的 */
    void addDataPoint(std::optional<double> value);

    /** 清空所有历史数据并重置图表 */
    void clearData();

    /**
     * @brief 全局低质量叠加（外部统一控制）
     *
     * 当 on=true 时，整个图表绘图区下方叠加红色渐变 area（无论具体值）。
     * 用于 HR 卡片跟随 SQI 状态。
     */
    void setLowQualityOverlay(bool on);

private:
    /** 重新根据 m_points 构建所有 series，并更新 Y 轴 */
    void rebuildSeries();

    /** 计算 Y 轴范围，返回 {yMin, yMax} */
    [[nodiscard]] std::pair<double, double> calcYRange() const;

    static constexpr int kMaxPoints = 60;

    // ── 数据 ──
    struct Point {
        double x{0.0};             ///< 时间戳（0..kMaxPoints-1）
        std::optional<double> y;
    };
    QList<Point> m_points;

    // ── 颜色 / 策略 ──
    QColor  m_lineColor;
    bool    m_enableLowQualityFill;   ///< 是否按阈值自动标红
    double  m_lowQualityThreshold;
    bool    m_lowQualityOverlay{false}; ///< 外部全局低质量叠加开关

    // ── Qt Charts 对象 ──
    QChart      *m_chart{nullptr};
    QChartView  *m_chartView{nullptr};
    QValueAxis  *m_axisX{nullptr};
    QValueAxis  *m_axisY{nullptr};
};
