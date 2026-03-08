#include "TrendChart.h"

#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <algorithm>
#include <cmath>
#include <limits>

// ── 构造 ─────────────────────────────────────────────────────────────────────
TrendChart::TrendChart(QColor lineColor, QWidget *parent)
    : QWidget(parent)
    , m_lineColor(lineColor)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(160);

    m_chart = new QChart();
    m_chart->setBackgroundVisible(false);
    m_chart->setBackgroundRoundness(0);
    // 左边距留足给 Y 轴标签，底部留给 X 轴时间标签
    m_chart->setMargins(QMargins(4, 6, 10, 2));
    m_chart->legend()->hide();
    m_chart->setAnimationOptions(QChart::NoAnimation);

    // ── X 轴（QDateTimeAxis，显示时间刻度） ──
    m_axisX = new QDateTimeAxis(m_chart);
    m_axisX->setTickCount(5);
    m_axisX->setFormat(QStringLiteral("HH:mm"));
    m_axisX->setLabelsColor(QColor(130, 130, 130));
    QFont xFont;
    xFont.setPointSize(7);
    m_axisX->setLabelsFont(xFont);
    m_axisX->setGridLineColor(QColor(220, 220, 220, 80));
    m_axisX->setLinePen(Qt::NoPen);
    // 初始范围随意设置，setData 时会更新
    m_axisX->setRange(QDateTime::currentDateTime().addSecs(-3600),
                      QDateTime::currentDateTime());

    // ── Y 轴 ──
    m_axisY = new QValueAxis(m_chart);
    m_axisY->setTickCount(5);
    m_axisY->setLabelFormat(QStringLiteral("%.4g"));
    m_axisY->setLabelsColor(QColor(130, 130, 130));
    QFont yFont;
    yFont.setPointSize(7);
    m_axisY->setLabelsFont(yFont);
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
void TrendChart::setData(const QList<QDateTime>             &timestamps,
                         const QList<std::optional<double>> &points,
                         std::optional<double>               refValue,
                         const QDateTime                    &axisStart,
                         const QDateTime                    &axisEnd)
{
    m_timestamps = timestamps;
    m_points     = points;
    m_refValue   = refValue;
    m_axisStart  = axisStart;
    m_axisEnd    = axisEnd;
    rebuildSeries();
}

void TrendChart::clearData() {
    m_timestamps.clear();
    m_points.clear();
    m_refValue = std::nullopt;
    rebuildSeries();
}

// ── 内部：重建所有 Series ─────────────────────────────────────────────────────
void TrendChart::rebuildSeries() {
    m_chart->removeAllSeries();

    const int n = m_points.size();

    if (n == 0 || m_timestamps.size() != n) {
        m_axisX->setRange(QDateTime::currentDateTime().addSecs(-3600),
                          QDateTime::currentDateTime());
        m_axisY->setRange(0.0, 1.0);
        m_axisY->setTickType(QValueAxis::TicksFixed);
        m_axisY->setTickCount(5);
        return;
    }

    // ── X 轴范围：优先使用外部指定的起止时间，否则退回首尾时间戳 ──
    const QDateTime xStart = m_axisStart.isValid() ? m_axisStart : m_timestamps.first();
    const QDateTime xEnd   = m_axisEnd.isValid()   ? m_axisEnd   : m_timestamps.last();
    m_axisX->setRange(xStart, xEnd);

    // 跨度超过 23 小时显示 HH:mm + 日期，否则仅显示 HH:mm
    const qint64 spanSecs = xStart.secsTo(xEnd);
    if (spanSecs > 23 * 3600) {
        m_axisX->setFormat(QStringLiteral("MM-dd HH:mm"));
    } else {
        m_axisX->setFormat(QStringLiteral("HH:mm"));
    }

    auto [yMin, yMax] = calcYRange();

    // ── 1. 折线：按 null 断点切出连续有效段，X 坐标用 msecsSinceEpoch ──
    QLineSeries *current = nullptr;
    for (int i = 0; i < n; ++i) {
        if (m_points[i].has_value()) {
            if (!current) {
                current = new QLineSeries();
                QPen pen(m_lineColor, 2.0);
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);
                current->setPen(pen);
                m_chart->addSeries(current);
            }
            current->append(
                static_cast<qreal>(m_timestamps[i].toMSecsSinceEpoch()),
                m_points[i].value());
        } else {
            current = nullptr; // null → 断开，下一个有效点开启新段
        }
    }

    // attach 所有折线到坐标轴
    for (auto *s : m_chart->series()) {
        s->attachAxis(m_axisX);
        s->attachAxis(m_axisY);
    }

    // ── 2. 参考线（y = refValue）水平虚线 ──
    if (m_refValue.has_value()) {
        const double rv = m_refValue.value();
        const qreal  rxStart = static_cast<qreal>(xStart.toMSecsSinceEpoch());
        const qreal  rxEnd   = static_cast<qreal>(xEnd.toMSecsSinceEpoch());

        auto *refLine = new QLineSeries();
        QPen dashPen(m_lineColor.lighter(170), 1.6);
        dashPen.setStyle(Qt::CustomDashLine);
        dashPen.setDashPattern({6.0, 4.0});
        refLine->setPen(dashPen);
        refLine->append(rxStart, rv);
        refLine->append(rxEnd,   rv);

        m_chart->addSeries(refLine);
        refLine->attachAxis(m_axisX);
        refLine->attachAxis(m_axisY);

        // ── Y 轴：TicksDynamic，以 refValue 为锚点，步距 = span/4
        //   确保 refValue 必然出现在 Y 轴刻度上 ──
        const double step4 = (yMax - yMin) / 4.0;
        m_axisY->setTickType(QValueAxis::TicksDynamic);
        m_axisY->setTickAnchor(rv);
        m_axisY->setTickInterval(step4 > 0 ? step4 : 1.0);
    } else {
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
    if (m_refValue.has_value()) {
        dMin = std::min(dMin, m_refValue.value());
        dMax = std::max(dMax, m_refValue.value());
    }

    if (dMin > dMax) return {0.0, 1.0};

    double span = dMax - dMin;
    if (span < 1e-9) span = std::max(1.0, std::abs(dMin) * 0.1);

    constexpr double kPad = 0.15;
    return {dMin - span * kPad, dMax + span * kPad};
}

