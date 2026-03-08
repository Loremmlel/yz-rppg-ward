#include "TrendChart.h"

#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <algorithm>
#include <cmath>
#include <limits>

// ── 构造 ─────────────────────────────────────────────────────────────────────
TrendChart::TrendChart(const QColor lineColor, QWidget *parent)
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
                         const QDateTime                    &axisEnd,
                         qint64                              intervalSecs)
{
    m_timestamps   = timestamps;
    m_points       = points;
    m_refValue     = refValue;
    m_axisStart    = axisStart;
    m_axisEnd      = axisEnd;
    m_intervalSecs = intervalSecs;
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

    // ── 1. 折线 & 散点：
    //   规则：相邻两个有效数据点，若它们的时间戳之差 ≤ 1.5 × intervalSecs，
    //   则连线；否则各自作为孤立点用散点绘制。
    //   当 intervalSecs == 0（未提供粒度）时，退化为原有行为：全部连线。
    // ──────────────────────────────────────────────────────────────────────
    // 先收集所有有效点的下标
    QList<int> validIdx;
    for (int i = 0; i < n; ++i)
        if (m_points[i].has_value()) validIdx.append(i);

    // 判断第 k 个有效点（validIdx[k]）是否与下一个有效点相邻（可连线）
    // "相邻"：时间差 ≤ 1.5 × intervalSecs
    auto isAdjacentToNext = [&](int k) -> bool {
        if (m_intervalSecs <= 0) return true; // 无粒度信息 → 全连线
        if (k + 1 >= validIdx.size()) return false;
        const qint64 gap = m_timestamps[validIdx[k]].secsTo(
                               m_timestamps[validIdx[k + 1]]);
        return gap <= m_intervalSecs * 3 / 2; // 1.5 倍容差
    };

    // 用于创建（并注册）一条新折线段的 helper
    auto newLineSeries = [&]() -> QLineSeries * {
        auto *ls = new QLineSeries();
        QPen pen(m_lineColor, 2.0);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        ls->setPen(pen);
        m_chart->addSeries(ls);
        return ls;
    };

    // 用于创建（并注册）一个散点系列的 helper
    auto newDotSeries = [&]() -> QScatterSeries * {
        auto *ss = new QScatterSeries();
        ss->setColor(m_lineColor);
        ss->setBorderColor(Qt::transparent);
        ss->setMarkerSize(6.0);
        ss->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        m_chart->addSeries(ss);
        return ss;
    };

    // 遍历有效点，按相邻关系切分为折线段或孤立散点
    int k = 0;
    while (k < validIdx.size()) {
        const int idx = validIdx[k];
        const qreal x = static_cast<qreal>(m_timestamps[idx].toMSecsSinceEpoch());
        const double y = m_points[idx].value();

        if (isAdjacentToNext(k)) {
            // 开始一条新折线段，持续追加直到断开
            auto *ls = newLineSeries();
            ls->append(x, y);
            while (isAdjacentToNext(k)) {
                ++k;
                const int ni = validIdx[k];
                ls->append(static_cast<qreal>(m_timestamps[ni].toMSecsSinceEpoch()),
                           m_points[ni].value());
            }
        } else {
            // 孤立点 → 散点
            auto *ss = newDotSeries();
            ss->append(x, y);
        }
        ++k;
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

