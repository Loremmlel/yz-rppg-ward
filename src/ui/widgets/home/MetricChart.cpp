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

    // 初始 Y 轴范围
    if (m_axisMode == AxisMode::Fixed01) {
        m_axisY->setRange(0.0, 1.0);
    } else {
        m_axisY->setRange(0.0, 100.0);
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
    double newX = m_points.isEmpty() ? 0.0 : m_points.back().x + 1.0;
    m_points.append({newX, value, lowQuality});

    if (m_points.size() > kMaxPoints) {
        m_points.removeFirst();
    }

    // 重新对齐 x 坐标到 [0, kMaxPoints-1]
    double offset = m_points.first().x;
    for (auto &pt : m_points) {
        pt.x -= offset;
    }

    rebuildSeries();
}

void MetricChart::clearData() {
    m_points.clear();
    rebuildSeries();
}

// ── 内部：重建所有 series ─────────────────────────────────────
void MetricChart::rebuildSeries() {
    m_chart->removeAllSeries();

    auto [yMin, yMax] = calcYRange();

    // ── 辅助：构建一段折线 ──
    auto makeSegmentSeries = [&](int from, int to) -> QLineSeries * {
        auto *s = new QLineSeries();
        QPen pen(m_lineColor, 2.0);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        s->setPen(pen);
        for (int i = from; i <= to; ++i) {
            if (m_points[i].y.has_value()) {
                s->append(m_points[i].x, m_points[i].y.value());
            }
        }
        return s;
    };

    // ── 辅助：为指定点集创建红色渐变 area（从折线到 yMin）
    //   注意：QAreaSeries 会接管 upper/lower 的所有权，
    //   因此每次都新建独立的 QLineSeries，不与主折线对象共用。 ──
    auto makeRedArea = [&](int from, int to) -> QAreaSeries * {
        auto *upper = new QLineSeries();
        auto *lower = new QLineSeries();
        for (int i = from; i <= to; ++i) {
            if (m_points[i].y.has_value()) {
                double xv = m_points[i].x;
                double yv = m_points[i].y.value();
                upper->append(xv, yv);
                lower->append(xv, yMin);
            }
        }
        auto *area = new QAreaSeries(upper, lower);
        area->setOpacity(1.0);
        QLinearGradient grad(0, 0, 0, 1);
        grad.setCoordinateMode(QGradient::ObjectMode);
        grad.setColorAt(0.0, QColor(255, 60, 60, 110));
        grad.setColorAt(1.0, QColor(255, 60, 60, 0));
        area->setBrush(grad);
        area->setPen(Qt::NoPen);
        return area;
    };

    // ── 1. 按 null 断点切出连续有效段 ──
    QList<QPair<int, int>> segments;
    int segStart = -1;
    for (int i = 0; i < m_points.size(); ++i) {
        if (m_points[i].y.has_value()) {
            if (segStart < 0) segStart = i;
        } else {
            if (segStart >= 0) {
                segments.append({segStart, i - 1});
                segStart = -1;
            }
        }
    }
    if (segStart >= 0) segments.append({segStart, m_points.size() - 1});

    // ── 2. 对每个连续段，按 lowQuality 标记切出子段，分别绘制红色 area 和主折线 ──
    for (const auto &[from, to] : segments) {
        // 2a. 找出段内所有 lowQuality 连续子段，叠加红色渐变 area
        int subStart = -1;
        for (int i = from; i <= to; ++i) {
            if (m_points[i].lowQuality) {
                if (subStart < 0) subStart = i;
            } else {
                if (subStart >= 0) {
                    auto *area = makeRedArea(subStart, i - 1);
                    m_chart->addSeries(area);
                    area->attachAxis(m_axisX);
                    area->attachAxis(m_axisY);
                    subStart = -1;
                }
            }
        }
        if (subStart >= 0) {
            auto *area = makeRedArea(subStart, to);
            m_chart->addSeries(area);
            area->attachAxis(m_axisX);
            area->attachAxis(m_axisY);
        }

        // 2b. 主折线（覆盖在渐变上方，独立新建 series）
        auto *line = makeSegmentSeries(from, to);
        m_chart->addSeries(line);
        line->attachAxis(m_axisX);
        line->attachAxis(m_axisY);
    }

    // 所有 series attach 完毕后设置 Y 轴范围
    m_axisY->setRange(yMin, yMax);
}

// ── 内部：Y 轴范围计算 ────────────────────────────────────────
std::pair<double, double> MetricChart::calcYRange() const {
    if (m_axisMode == AxisMode::Fixed01) {
        return {0.0, 1.0};
    }

    // ElasticFrom100：初始下限 0，上限至少 100，数据超出时弹性扩展
    double dMin = std::numeric_limits<double>::max();
    double dMax = std::numeric_limits<double>::lowest();

    for (const auto &[x, y, lq] : m_points) {
        if (y.has_value()) {
            dMin = std::min(dMin, y.value());
            dMax = std::max(dMax, y.value());
        }
    }

    if (dMin > dMax) return {0.0, 100.0};  // 无数据，保持初始范围

    // 下限固定 0，上限至少 100
    double lo = 0.0;
    double hi = std::max(100.0, dMax);

    double span = hi - lo;
    constexpr double kPad = 0.10;
    // 下限 padding 后 clamp 到 0：心率不可能为负
    return {std::max(0.0, lo - span * kPad), hi + span * kPad};
}
