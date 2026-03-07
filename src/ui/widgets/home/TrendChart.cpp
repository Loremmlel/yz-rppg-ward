#include "TrendChart.h"

#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

// ── 构造 ─────────────────────────────────────────────────────────────────────
TrendChart::TrendChart(QColor lineColor, QWidget *parent)
    : QWidget(parent)
    , m_lineColor(lineColor)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(90);

    m_chart = new QChart();
    m_chart->setBackgroundVisible(false);
    m_chart->setBackgroundRoundness(0);
    m_chart->setMargins(QMargins(0, 4, 8, 0));
    m_chart->legend()->hide();
    m_chart->setAnimationOptions(QChart::NoAnimation);

    // ── X 轴（隐藏，仅作索引） ──
    m_axisX = new QValueAxis(m_chart);
    m_axisX->setRange(0.0, 1.0);
    m_axisX->setVisible(false);

    // ── Y 轴 ──
    m_axisY = new QValueAxis(m_chart);
    m_axisY->setTickCount(5);
    m_axisY->setLabelFormat("%.4g");   // 自适应精度
    m_axisY->setLabelsColor(QColor(140, 140, 140));
    QFont axisFont;
    axisFont.setPointSize(8);
    m_axisY->setLabelsFont(axisFont);
    m_axisY->setGridLineColor(QColor(220, 220, 220, 100));
    m_axisY->setMinorGridLineVisible(false);
    m_axisY->setLinePen(Qt::NoPen);
    m_axisY->setRange(0.0, 1.0);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setFrameShape(QFrame::NoFrame);
    m_chartView->setBackgroundBrush(Qt::NoBrush);
    m_chartView->viewport()->setAutoFillBackground(false);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_chartView);
}

// ── 公开接口 ─────────────────────────────────────────────────────────────────
void TrendChart::setData(const QList<std::optional<double>> &points,
                         std::optional<double> refValue)
{
    m_points   = points;
    m_refValue = refValue;
    rebuildSeries();
}

void TrendChart::clearData() {
    m_points.clear();
    m_refValue = std::nullopt;
    rebuildSeries();
}

// ── 内部：重建所有 Series ─────────────────────────────────────────────────────
void TrendChart::rebuildSeries() {
    m_chart->removeAllSeries();

    if (m_points.isEmpty()) {
        m_axisX->setRange(0.0, 1.0);
        m_axisY->setRange(0.0, 1.0);
        m_axisY->setTickCount(5);
        return;
    }

    const int n = m_points.size();
    m_axisX->setRange(0.0, static_cast<double>(n - 1));

    auto [yMin, yMax] = calcYRange();

    // ── 1. 折线：按 null 断点切出连续有效段，每段一条 QLineSeries ──
    QLineSeries *current = nullptr;
    for (int i = 0; i < n; ++i) {
        if (m_points[i].has_value()) {
            if (!current) {
                current = new QLineSeries();
                QPen pen(m_lineColor, 1.8);
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);
                current->setPen(pen);
                m_chart->addSeries(current);
            }
            current->append(static_cast<double>(i), m_points[i].value());
        } else {
            current = nullptr; // null 点 → 断开，下一个有效点开启新 series
        }
    }

    // attach 所有折线 series 到坐标轴
    for (auto *s : m_chart->series()) {
        s->attachAxis(m_axisX);
        s->attachAxis(m_axisY);
    }

    // ── 2. 参考线（y = refValue），贯穿整个 X 范围的水平虚线 ──
    if (m_refValue.has_value()) {
        const double rv = m_refValue.value();

        auto *refLine = new QLineSeries();
        QPen dashPen(m_lineColor.lighter(160), 1.4);
        dashPen.setStyle(Qt::CustomDashLine);
        dashPen.setDashPattern({5.0, 4.0});
        refLine->setPen(dashPen);
        refLine->append(0.0,                        rv);
        refLine->append(static_cast<double>(n - 1), rv);

        m_chart->addSeries(refLine);
        refLine->attachAxis(m_axisX);
        refLine->attachAxis(m_axisY);

        // ── Y 轴刻度：使用 TicksDynamic，以 refValue 为锚点，
        //   步距 = span/4（使可见范围内约显示 5 个刻度）。
        //   refValue 必然是一条刻度线，自然在 Y 轴上产生对应标注。 ──
        const double step4 = (yMax - yMin) / 4.0;
        m_axisY->setTickType(QValueAxis::TicksDynamic);
        m_axisY->setTickAnchor(rv);
        m_axisY->setTickInterval(step4);
    } else {
        // 无参考线，正常 5 刻度
        m_axisY->setTickType(QValueAxis::TicksFixed);
        m_axisY->setTickCount(5);
    }

    m_axisY->setRange(yMin, yMax);
}

// ── 内部：Y 轴范围计算 ────────────────────────────────────────────────────────
std::pair<double, double> TrendChart::calcYRange() const {
    double dMin = std::numeric_limits<double>::max();
    double dMax = std::numeric_limits<double>::lowest();

    for (const auto &pt : m_points) {
        if (pt.has_value()) {
            dMin = std::min(dMin, pt.value());
            dMax = std::max(dMax, pt.value());
        }
    }

    // 把 refValue 也纳入范围，确保参考线可见
    if (m_refValue.has_value()) {
        dMin = std::min(dMin, m_refValue.value());
        dMax = std::max(dMax, m_refValue.value());
    }

    if (dMin > dMax) return {0.0, 1.0}; // 无有效数据

    double span = dMax - dMin;
    if (span < 1e-9) span = std::max(1.0, std::abs(dMin) * 0.1);

    constexpr double kPad = 0.15;
    return {dMin - span * kPad, dMax + span * kPad};
}


