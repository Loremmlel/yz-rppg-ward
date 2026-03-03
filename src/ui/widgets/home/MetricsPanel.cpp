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

    // HR 卡片：主题色红，无自动阈值填充，无文字警告（由全局 overlay 控制）
    addMetricCard("HR",  QStringLiteral("心率"),     ":/icons/Heartbeat.png",
                  QColor(0xFF, 0x52, 0x52), false, kSqiLowThreshold, false);
    // SQI 卡片：主题色紫，启用按阈值自动红色填充，显示文字警告
    addMetricCard("SQI", QStringLiteral("信号质量"), ":/icons/Sqi.png",
                  QColor(0x7E, 0x57, 0xC2), true,  kSqiLowThreshold, true);

    mainLayout->addWidget(cardContainer, 1);
}

void MetricsPanel::updateData(const MetricsData &data) {
    // ── HR ──
    if (m_cards.contains("HR")) {
        if (data.hr.has_value()) {
            m_cards["HR"]->setValue(QString("%1 bpm").arg(formatDecimal(data.hr.value(), 0)));
        } else {
            m_cards["HR"]->setValue("-- bpm");
        }
        m_cards["HR"]->addDataPoint(data.hr);
    }

    // ── SQI ──
    if (m_cards.contains("SQI")) {
        if (data.sqi.has_value()) {
            m_cards["SQI"]->setValue(formatDecimal(data.sqi.value(), 3));
        } else {
            m_cards["SQI"]->setValue("--");
        }
        m_cards["SQI"]->addDataPoint(data.sqi);
    }

    // ── 低质量判断：SQI 缺失或低于阈值时触发 ──
    const bool lowQuality = !data.sqi.has_value()
                            || data.sqi.value() < kSqiLowThreshold;

    if (lowQuality != m_lastLowQuality) {
        m_lastLowQuality = lowQuality;
        // HR 卡触发全局 overlay（整个图表变红）
        if (m_cards.contains("HR"))  m_cards["HR"]->setLowQuality(lowQuality);
        // SQI 卡显示文字警告（图表自身已按阈值分段染色，无需 overlay）
        if (m_cards.contains("SQI")) m_cards["SQI"]->setLowQuality(lowQuality);
    }
}

void MetricsPanel::addMetricCard(const QString &key, const QString &title, const QString &icon,
                                 QColor chartColor, bool enableLowQualityFill,
                                 double lowQualityThreshold, bool showLowQualityWarning) {
    auto *card = new MetricCard(title, icon, chartColor,
                                enableLowQualityFill, lowQualityThreshold,
                                showLowQualityWarning, this);
    card->setObjectName("Card" + key);
    m_listLayout->addWidget(card, 1);
    m_cards.insert(key, card);
}

