#pragma once

#include <QWidget>
#include <QList>
#include <optional>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

/**
 * @brief 历史趋势折线图（批量数据，Qt Charts 实现）
 *
 * 特性：
 *  - 接受 (x_index, y_value) 列表，null 值令折线断开
 *  - Y 轴显示 5 个刻度
 *  - 可选：绘制 y = refValue 的水平虚线，并在 Y 轴额外标注该值
 *  - X 轴仅作索引，不显示
 */
class TrendChart : public QWidget {
    Q_OBJECT

public:
    /**
     * @param lineColor 折线颜色
     * @param parent    父控件
     */
    explicit TrendChart(QColor lineColor, QWidget *parent = nullptr);

    /**
     * @brief 用新数据集全量替换图表内容
     *
     * @param points    数据点列表，index 即为 X 轴序号
     * @param refValue  参考线值（均值或中位数），nullopt 则不绘制参考线
     */
    void setData(const QList<std::optional<double>> &points,
                 std::optional<double> refValue = std::nullopt);

    /** @brief 清空图表 */
    void clearData();

private:
    void rebuildSeries();

    [[nodiscard]] std::pair<double, double> calcYRange() const;

    // ── 数据 ──
    QList<std::optional<double>> m_points;
    std::optional<double>        m_refValue;

    // ── 配置 ──
    QColor m_lineColor;

    // ── Qt Charts 对象 ──
    QChart     *m_chart{nullptr};
    QChartView *m_chartView{nullptr};
    QValueAxis *m_axisX{nullptr};
    QValueAxis *m_axisY{nullptr};
};

