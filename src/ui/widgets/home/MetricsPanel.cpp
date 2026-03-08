#include "MetricsPanel.h"
#include <QVBoxLayout>
#include <QLabel>

#include "../../../util/StyleLoader.h"

namespace {
    double truncateDecimal(const double value, const int precision) {
        return std::floor(value * std::pow(10, precision)) / std::pow(10, precision);
    }

    QString formatDecimal(const double value, const int precision) {
        return QString::number(truncateDecimal(value, precision), 'f', precision);
    }

    constexpr double kSqiLowThreshold = 0.5;
}

MetricsPanel::MetricsPanel(QWidget *parent) : QWidget(parent) {
    this->setObjectName("metricsPanel");
    StyleLoader::apply(this, QStringLiteral(":/styles/metrics_panel.qss"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *titleLabel = new QLabel(QStringLiteral("实时信号"), this);
    titleLabel->setObjectName("metricsTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto *cardContainer = new QWidget(this);
    cardContainer->setObjectName("metricsContainer");

    m_listLayout = new QVBoxLayout(cardContainer);
    m_listLayout->setContentsMargins(15, 10, 15, 10);
    m_listLayout->setSpacing(15);

    // HR 卡片：主题色红，弹性 Y 轴（初始 0-100），无文字警告
    addMetricCard("HR", QStringLiteral("心率"), ":/icons/Heartbeat.png",
                  QColor(0xFF, 0x52, 0x52), MetricChart::AxisMode::ElasticFrom100, false);
    // SQI 卡片：主题色紫，固定 0-1 Y 轴，显示文字警告
    addMetricCard("SQI", QStringLiteral("信号质量"), ":/icons/Sqi.png",
                  QColor(0x7E, 0x57, 0xC2), MetricChart::AxisMode::Fixed01, true);

    mainLayout->addWidget(cardContainer, 1);
}

void MetricsPanel::updateData(const MetricsData &data) {
    // ── 低质量判断：SQI 缺失或低于阈值时为低质量 ──
    const bool lowQuality = !data.sqi.has_value()
                            || data.sqi.value() < kSqiLowThreshold;

    // ── HR ──
    if (m_cards.contains("HR")) {
        if (data.hr.has_value()) {
            m_cards["HR"]->setValue(QString("%1 bpm").arg(formatDecimal(data.hr.value(), 0)));
        } else {
            m_cards["HR"]->setValue("-- bpm");
        }
        // 将当前时刻的低质量状态随数据点一起传入，图表按时间段精确着色
        m_cards["HR"]->addDataPoint(data.hr, lowQuality);
    }

    // ── SQI ──
    if (m_cards.contains("SQI")) {
        if (data.sqi.has_value()) {
            m_cards["SQI"]->setValue(formatDecimal(data.sqi.value(), 3));
        } else {
            m_cards["SQI"]->setValue("--");
        }
        // SQI 图表：同样按实际低质量时刻标记，而非 SQI < 阈值（两者等价但保持一致）
        m_cards["SQI"]->addDataPoint(data.sqi, lowQuality);
    }

    // ── 警告标签：状态变化时更新 ──
    if (lowQuality != m_lastLowQuality) {
        m_lastLowQuality = lowQuality;
        if (m_cards.contains("SQI")) m_cards["SQI"]->setLowQualityWarning(lowQuality);
    }
}

void MetricsPanel::addMetricCard(const QString &key, const QString &title, const QString &icon,
                                 const QColor chartColor, const MetricChart::AxisMode axisMode,
                                 const bool showLowQualityWarning) {
    auto *card = new MetricCard(title, icon, chartColor,
                                axisMode, showLowQualityWarning, this);
    card->setObjectName("Card" + key);
    m_listLayout->addWidget(card, 1);
    m_cards.insert(key, card);
}