#include "MetricChart.h"

#include <QVBoxLayout>
#include <QLinearGradient>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <algorithm>
#include <cmath>
#include <limits>

// ── 构造 ─────────────────────────────────────────────────────
MetricChart::MetricChart(QColor lineColor,
                         AxisMode axisMode,
                         QWidget *parent)
    : QWidget(parent)
    , m_lineColor(lineColor)
    , m_axisMode(axisMode)
{
    setMinimumHeight(50);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ── Chart ──
    m_chart = new QChart();
    m_chart->setBackgroundVisible(false);
    m_chart->setBackgroundRoundness(0);
    m_chart->setMargins(QMargins(0, 0, 0, 0));
    m_chart->legend()->hide();
    m_chart->setAnimationOptions(QChart::NoAnimation);

    // ── 坐标轴 ──
    m_axisX = new QValueAxis(m_chart);
    m_axisX->setRange(0, kMaxPoints - 1);
    m_axisX->setVisible(false);

    m_axisY = new QValueAxis(m_chart);
    m_axisY->setTickCount(3);
    m_axisY->setLabelFormat("%.3f");
    m_axisY->setLabelsColor(QColor(150, 150, 150));
    QFont axisFont;
    axisFont.setPointSize(8);
    m_axisY->setLabelsFont(axisFont);
    m_axisY->setGridLineColor(QColor(220, 220, 220, 80));
    m_axisY->setLinePen(Qt::NoPen);

    if (m_axisMode == AxisMode::Fixed01) {
        m_axisY->setRange(0.0, 1.0);
        m_cachedYMin = 0.0;
        m_cachedYMax = 1.0;
    } else {
        m_axisY->setRange(0.0, 100.0);
        m_cachedYMin = 0.0;
        m_cachedYMax = 100.0;
    }

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // ── ChartView ──
    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setFrameShape(QFrame::NoFrame);
    m_chartView->setBackgroundBrush(Qt::NoBrush);
    m_chartView->viewport()->setAutoFillBackground(false);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_chartView);
}

// ── 公开接口 ─────────────────────────────────────────────────
void MetricChart::addDataPoint(std::optional<double> value, bool lowQuality) {
    // 使用单调递增 x，不再每帧重对齐所有点
    m_points.append({m_xCounter, value, lowQuality});
    m_xCounter += 1.0;

    if (m_points.size() > kMaxPoints) {
        m_points.removeFirst();
    }

    // 滑动 X 轴窗口：始终显示最近 kMaxPoints 个点
    double xEnd   = m_points.back().x;
    double xStart = xEnd - (kMaxPoints - 1);
    m_axisX->setRange(xStart, xEnd);

    // 计算新的 Y 轴范围
    auto [yMin, yMax] = calcYRange();

    // 计算当前拓扑
    Topology topo = calcTopology();

    if (!m_poolInitialized || topo != m_lastTopology) {
        // 拓扑变化（或首次）：重建 series 池
        rebuildSeriesPool(topo, yMin);
        m_lastTopology = topo;
        m_poolInitialized = true;
    } else {
        // 拓扑不变：仅用 replace() 更新点坐标（性能热路径）
        updateSeriesData(topo, yMin);
    }

    updateYAxis(yMin, yMax);
}

void MetricChart::clearData() {
    m_points.clear();
    // xCounter 不重置，保持单调性（也可重置，无副作用）
    m_xCounter = 0.0;
    m_poolInitialized = false;
    m_lastTopology = {};

    // 移除所有 series，清空池
    m_chart->removeAllSeries();
    m_lineSeries.clear();
    m_areaSeries.clear();

    // 复位 X 轴
    m_axisX->setRange(0, kMaxPoints - 1);

    // 复位 Y 轴
    if (m_axisMode == AxisMode::Fixed01) {
        m_cachedYMin = 0.0; m_cachedYMax = 1.0;
        m_axisY->setRange(0.0, 1.0);
    } else {
        m_cachedYMin = 0.0; m_cachedYMax = 100.0;
        m_axisY->setRange(0.0, 100.0);
    }
}

// ── 内部：计算分段拓扑 ────────────────────────────────────────
MetricChart::Topology MetricChart::calcTopology() const {
    Topology topo;

    // 1. 有效（非null）连续段，用 x 坐标记录边界
    int segStart = -1;
    for (int i = 0; i < m_points.size(); ++i) {
        if (m_points[i].y.has_value()) {
            if (segStart < 0) segStart = i;
        } else {
            if (segStart >= 0) {
                topo.validSegs.append({m_points[segStart].x, m_points[i - 1].x});
                segStart = -1;
            }
        }
    }
    if (segStart >= 0)
        topo.validSegs.append({m_points[segStart].x, m_points.back().x});

    // 2. lowQuality 连续子段，用 x 坐标记录边界
    int lqStart = -1;
    for (int i = 0; i < m_points.size(); ++i) {
        const bool isValid = m_points[i].y.has_value();
        if (isValid && m_points[i].lowQuality) {
            if (lqStart < 0) lqStart = i;
        } else {
            if (lqStart >= 0) {
                topo.lowQualitySegs.append({m_points[lqStart].x, m_points[i - 1].x});
                lqStart = -1;
            }
        }
    }
    if (lqStart >= 0)
        topo.lowQualitySegs.append({m_points[lqStart].x, m_points.back().x});

    return topo;
}

// ── 辅助：配置一个 QAreaSeries 的样式 ────────────────────────
static void configureAreaSeries(QAreaSeries *area) {
    area->setOpacity(1.0);
    QLinearGradient grad(0, 0, 0, 1);
    grad.setCoordinateMode(QGradient::ObjectMode);
    grad.setColorAt(0.0, QColor(255, 60, 60, 110));
    grad.setColorAt(1.0, QColor(255, 60, 60, 0));
    area->setBrush(grad);
    area->setPen(Qt::NoPen);
}

// ── 内部：重建 series 池（拓扑变化时）────────────────────────
void MetricChart::rebuildSeriesPool(const Topology &topo, double yMin) {
    m_chart->removeAllSeries();
    m_lineSeries.clear();
    m_areaSeries.clear();

    QPen linePen(m_lineColor, 2.0);
    linePen.setCapStyle(Qt::RoundCap);
    linePen.setJoinStyle(Qt::RoundJoin);

    // ── 为每个 lowQuality 子段创建 QAreaSeries ──
    for (const auto &seg : topo.lowQualitySegs) {
        auto *upper = new QLineSeries();
        auto *lower = new QLineSeries();
        for (const auto &pt : m_points) {
            if (pt.y.has_value() && pt.x >= seg.xFrom && pt.x <= seg.xTo) {
                upper->append(pt.x, pt.y.value());
                lower->append(pt.x, yMin);
            }
        }
        auto *area = new QAreaSeries(upper, lower);
        configureAreaSeries(area);
        m_areaSeries.append(area);
        m_chart->addSeries(area);
        area->attachAxis(m_axisX);
        area->attachAxis(m_axisY);
    }

    // ── 为每个有效段创建 QLineSeries（覆盖在 area 上方）──
    for (const auto &seg : topo.validSegs) {
        auto *line = new QLineSeries();
        line->setPen(linePen);
        for (const auto &pt : m_points) {
            if (pt.y.has_value() && pt.x >= seg.xFrom && pt.x <= seg.xTo) {
                line->append(pt.x, pt.y.value());
            }
        }
        m_lineSeries.append(line);
        m_chart->addSeries(line);
        line->attachAxis(m_axisX);
        line->attachAxis(m_axisY);
    }
}

// ── 内部：仅更新 series 点数据（拓扑不变，热路径）────────────
void MetricChart::updateSeriesData(const Topology &topo, double yMin) {
    // ── 更新 lowQuality area series ──
    for (int si = 0; si < topo.lowQualitySegs.size(); ++si) {
        const auto &seg = topo.lowQualitySegs[si];
        QAreaSeries *area = m_areaSeries[si];
        QLineSeries *upper = area->upperSeries();
        QLineSeries *lower = area->lowerSeries();

        QList<QPointF> upperPts, lowerPts;
        for (const auto &pt : m_points) {
            if (pt.y.has_value() && pt.x >= seg.xFrom && pt.x <= seg.xTo) {
                upperPts.append({pt.x, pt.y.value()});
                lowerPts.append({pt.x, yMin});
            }
        }
        upper->replace(upperPts);
        lower->replace(lowerPts);
    }

    // ── 更新主折线 series ──
    for (int si = 0; si < topo.validSegs.size(); ++si) {
        const auto &seg = topo.validSegs[si];
        QLineSeries *line = m_lineSeries[si];

        QList<QPointF> pts;
        for (const auto &pt : m_points) {
            if (pt.y.has_value() && pt.x >= seg.xFrom && pt.x <= seg.xTo) {
                pts.append({pt.x, pt.y.value()});
            }
        }
        line->replace(pts);
    }
}

// ── 内部：更新 Y 轴（滞后抑制）────────────────────────────────
void MetricChart::updateYAxis(double yMin, double yMax) {
    if (std::abs(yMin - m_cachedYMin) > kYHysteresis ||
        std::abs(yMax - m_cachedYMax) > kYHysteresis)
    {
        m_cachedYMin = yMin;
        m_cachedYMax = yMax;
        m_axisY->setRange(yMin, yMax);
    }
}

// ── 内部：Y 轴范围计算 ────────────────────────────────────────
std::pair<double, double> MetricChart::calcYRange() const {
    if (m_axisMode == AxisMode::Fixed01) {
        return {0.0, 1.0};
    }

    double dMin = std::numeric_limits<double>::max();
    double dMax = std::numeric_limits<double>::lowest();

    for (const auto &pt : m_points) {
        if (pt.y.has_value()) {
            dMin = std::min(dMin, pt.y.value());
            dMax = std::max(dMax, pt.y.value());
        }
    }

    if (dMin > dMax) return {0.0, 100.0};

    double lo = 0.0;
    double hi = std::max(100.0, dMax);
    double span = hi - lo;
    constexpr double kPad = 0.10;
    return {std::max(0.0, lo - span * kPad), hi + span * kPad};
}
