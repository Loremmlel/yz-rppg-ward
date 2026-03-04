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
 */
class MetricChart : public QWidget {
    Q_OBJECT

public:
    enum class AxisMode {
        ElasticFrom100, ///< 初始 [0,100]，数据超出时弹性扩展 (HR)
        Fixed01         ///< 固定 [0,1] (SQI)
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
    /** 重新根据 m_points 构建所有 series，并更新 Y 轴 */
    void rebuildSeries();

    /** 计算 Y 轴范围，返回 {yMin, yMax} */
    [[nodiscard]] std::pair<double, double> calcYRange() const;

    static constexpr int kMaxPoints = 60;

    // ── 数据 ──
    struct Point {
        double x{0.0};
        std::optional<double> y;
        bool lowQuality{false}; ///< 该时刻信号质量不佳
    };
    QList<Point> m_points;

    // ── 配置 ──
    QColor   m_lineColor;
    AxisMode m_axisMode;

    // ── Qt Charts 对象 ──
    QChart      *m_chart{nullptr};
    QChartView  *m_chartView{nullptr};
    QValueAxis  *m_axisX{nullptr};
    QValueAxis  *m_axisY{nullptr};
};
