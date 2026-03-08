#pragma once

#include <QDateTime>
#include <optional>

#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QScatterSeries>

/**
 * @brief 历史趋势折线图（批量数据，Qt Charts 实现）
 *
 * 特性：
 *  - 接受 (bucketTime, y_value) 列表，null 值令折线断开
 *  - Y 轴显示 5 个刻度（有参考线时用 TicksDynamic 使参考值恰好落在刻度上）
 *  - X 轴显示约 5 个时间刻度（HH:mm 格式）
 *  - 参考线：y = refValue 水平虚线，Y 轴上额外标注该值
 *  - yMaxHint：Y 轴软上限；数据全部在 [0, hint] 内时固定使用此范围，
 *              有数据超出时退回动态计算。
 */
class TrendChart : public QWidget {
    Q_OBJECT

public:
    /**
     * @param lineColor  折线 & 散点颜色
     * @param yMaxHint   Y 轴软上限（下限固定为 0）；≤0 表示完全动态计算
     */
    explicit TrendChart(QColor lineColor, double yMaxHint = 0.0, QWidget *parent = nullptr);

    /**
     * @brief 用新数据集全量替换图表内容
     *
     * @param timestamps    每个数据点对应的 bucket 时刻（本地时间）
     * @param points        对应的数值，nullopt 表示该 bucket 缺失
     * @param refValue      参考线值（均值或中位数），nullopt 则不绘制参考线
     * @param axisStart     X 轴起点（用户选择的查询开始时间），无效则取首个时间戳
     * @param axisEnd       X 轴终点（用户选择的查询结束时间），无效则取最后时间戳
     * @param intervalSecs  时间粒度（秒）；相邻有效点间距 > 此值时断开折线，单独绘点
     */
    void setData(const QList<QDateTime>              &timestamps,
                 const QList<std::optional<double>>  &points,
                 std::optional<double>                refValue     = std::nullopt,
                 const QDateTime                     &axisStart    = QDateTime{},
                 const QDateTime                     &axisEnd      = QDateTime{},
                 qint64                               intervalSecs = 0);

    /** @brief 清空图表 */
    void clearData();

private:
    void rebuildSeries();
    [[nodiscard]] std::pair<double, double> calcYRange() const;

    // ── 数据 ──
    QList<QDateTime>             m_timestamps;
    QList<std::optional<double>> m_points;
    std::optional<double>        m_refValue;
    QDateTime                    m_axisStart;    ///< X 轴强制起点（无效则自动取首个时间戳）
    QDateTime                    m_axisEnd;      ///< X 轴强制终点（无效则自动取最后时间戳）
    qint64                       m_intervalSecs{0}; ///< 时间粒度（秒），0 表示不做间隔检查

    // ── 配置 ──
    QColor  m_lineColor;
    double  m_yMaxHint{0.0}; ///< Y 轴软上限（≤0 = 动态）

    // ── Qt Charts 对象 ──
    QChart        *m_chart{nullptr};
    QChartView    *m_chartView{nullptr};
    QDateTimeAxis *m_axisX{nullptr};
    QValueAxis    *m_axisY{nullptr};
};

