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
                         bool enableLowQualityFill,
                         double lowQualityThreshold,
                         QWidget *parent)
    : QWidget(parent)
    , m_lineColor(lineColor)
    , m_enableLowQualityFill(enableLowQualityFill)
    , m_lowQualityThreshold(lowQualityThreshold)
{
    setMinimumHeight(50);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ── Chart ──
    m_chart = new QChart();
    m_chart->setBackgroundVisible(false);       // 背景透明，由 QSS 控制
    m_chart->setBackgroundRoundness(0);
    m_chart->setMargins(QMargins(0, 0, 0, 0));
    m_chart->legend()->hide();
    m_chart->setAnimationOptions(QChart::NoAnimation);

    // ── 坐标轴 ──
    m_axisX = new QValueAxis(m_chart);
    m_axisX->setRange(0, kMaxPoints - 1);
    m_axisX->setVisible(false);               // 不显示 X 轴标签 / 网格线

    m_axisY = new QValueAxis(m_chart);
    m_axisY->setRange(0, 1);
    m_axisY->setTickCount(3);
    m_axisY->setLabelFormat("%.1f");
    m_axisY->setLabelsColor(QColor(150, 150, 150));
    QFont axisFont;
    axisFont.setPointSize(8);
    m_axisY->setLabelsFont(axisFont);
    m_axisY->setGridLineColor(QColor(220, 220, 220, 80));
    m_axisY->setLinePen(Qt::NoPen);

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
void MetricChart::addDataPoint(std::optional<double> value) {
    // 时间戳：始终填满 [0, kMaxPoints-1] 窗口
    double newX = m_points.isEmpty() ? 0.0 : m_points.back().x + 1.0;
    m_points.append({newX, value});

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

void MetricChart::setLowQualityOverlay(bool on) {
    if (m_lowQualityOverlay == on) return;
    m_lowQualityOverlay = on;
    rebuildSeries();
}

// ── 内部：重建所有 series ─────────────────────────────────────
void MetricChart::rebuildSeries() {
    // 移除旧 series（QChart 会 delete 它们）
    m_chart->removeAllSeries();

    if (m_points.isEmpty()) {
        m_axisY->setRange(0, 1);
        return;
    }

    // ── 更新 Y 轴 ──
    auto [yMin, yMax] = calcYRange();
    m_axisY->setRange(yMin, yMax);

    // ── 辅助：构建一条从起始到结束的 QLineSeries（单段） ──
    // 返回值的所有权交给调用者（再 addSeries 给 chart）
    auto makeSegmentSeries = [&](const int from, const int to) -> QLineSeries * {
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

    // ── 辅助：为一段折线创建红色渐变 area ──
    auto makeRedArea = [&](QLineSeries *upper) -> QAreaSeries * {
        auto *lower = new QLineSeries(); // X 轴基线（y = yMin）
        for (const QPointF &pt : upper->points()) {
            lower->append(pt.x(), yMin);
        }
        auto *area = new QAreaSeries(upper, lower);
        area->setOpacity(1.0);
        // 渐变：从折线顶部（半透明红）到底部（透明）
        // 使用 ObjectMode（Qt6 等价于 ObjectBoundingBoxMode）
        QLinearGradient grad(0, 0, 0, 1);
        grad.setCoordinateMode(QGradient::ObjectMode);
        grad.setColorAt(0.0, QColor(255, 60, 60, 110));
        grad.setColorAt(1.0, QColor(255, 60, 60, 0));
        area->setBrush(grad);
        area->setPen(Qt::NoPen);
        return area;
    };

    // ── 1. 将数据按 null 断点切分成连续段 ──
    QList<QPair<int, int>> segments; // [from, to] inclusive
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
    if (segStart >= 0) {
        segments.append({segStart, m_points.size() - 1});
    }

    // ── 2. 按段绘制折线，并在需要时叠加红色渐变 area ──
    for (const auto &[from, to] : segments) {
        // ── 2a. 对 SQI 卡：找出段内低质量子段并着色 ──
        if (m_enableLowQualityFill && !m_lowQualityOverlay) {
            // 在当前段内按阈值切出更细的子段
            int subStart = -1;
            for (int i = from; i <= to; ++i) {
                bool low = m_points[i].y.has_value()
                           && m_points[i].y.value() < m_lowQualityThreshold;
                if (low) {
                    if (subStart < 0) subStart = i;
                } else {
                    if (subStart >= 0) {
                        auto *upper = makeSegmentSeries(subStart, i - 1);
                        m_chart->addSeries(upper);
                        upper->attachAxis(m_axisX);
                        upper->attachAxis(m_axisY);
                        auto *area = makeRedArea(upper);
                        m_chart->addSeries(area);
                        area->attachAxis(m_axisX);
                        area->attachAxis(m_axisY);
                        subStart = -1;
                    }
                }
            }
            if (subStart >= 0) {
                auto *upper = makeSegmentSeries(subStart, to);
                m_chart->addSeries(upper);
                upper->attachAxis(m_axisX);
                upper->attachAxis(m_axisY);
                auto *area = makeRedArea(upper);
                m_chart->addSeries(area);
                area->attachAxis(m_axisX);
                area->attachAxis(m_axisY);
            }
        }

        // ── 2b. 全局低质量叠加（HR 卡跟随 SQI 状态） ──
        if (m_lowQualityOverlay) {
            auto *upper = makeSegmentSeries(from, to);
            m_chart->addSeries(upper);
            upper->attachAxis(m_axisX);
            upper->attachAxis(m_axisY);
            auto *area = makeRedArea(upper);
            m_chart->addSeries(area);
            area->attachAxis(m_axisX);
            area->attachAxis(m_axisY);
        }

        // ── 2c. 主折线（总是绘制，覆盖在渐变上方） ──
        auto *line = makeSegmentSeries(from, to);
        m_chart->addSeries(line);
        line->attachAxis(m_axisX);
        line->attachAxis(m_axisY);
    }
}

// ── 内部：Y 轴范围计算 ────────────────────────────────────────
std::pair<double, double> MetricChart::calcYRange() const {
    double dMin = std::numeric_limits<double>::max();
    double dMax = std::numeric_limits<double>::lowest();

    for (const auto &pt : m_points) {
        if (pt.y.has_value()) {
            dMin = std::min(dMin, pt.y.value());
            dMax = std::max(dMax, pt.y.value());
        }
    }

    if (dMin > dMax) return {0.0, 1.0};

    double span = dMax - dMin;
    constexpr double kMinSpan = 1e-6;
    if (span < kMinSpan) span = kMinSpan;

    constexpr double kPad = 0.15;
    return {dMin - span * kPad, dMax + span * kPad};
}
