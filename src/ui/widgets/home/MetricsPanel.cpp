#include "MetricsPanel.h"
#include <QVBoxLayout>
#include <QLabel>

#include "../../../util/StyleLoader.h"

MetricsPanel::MetricsPanel(QWidget *parent) : QWidget(parent) {
    this->setObjectName("metricsPanel");
    StyleLoader::apply(this, QStringLiteral(":/styles/metrics_panel.qss"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *titleLabel = new QLabel(QStringLiteral("实时监测指标"), this);
    titleLabel->setObjectName("metricsTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 卡片容器
    auto *cardContainer = new QWidget(this);
    cardContainer->setObjectName("metricsContainer");

    m_listLayout = new QVBoxLayout(cardContainer);
    m_listLayout->setContentsMargins(15, 10, 15, 10);
    m_listLayout->setSpacing(15);

    addMetricCard("HR",  QStringLiteral("心率"),       ":/icons/Heartbeat.png");
    addMetricCard("SQI", QStringLiteral("信号质量"),    QStringLiteral("📶"));

    mainLayout->addWidget(cardContainer, 1); // stretch=1 让卡片区域填满
}

void MetricsPanel::updateData(const MetricsData &data) {
    if (m_cards.contains("HR"))
        m_cards["HR"]->setValue(QString("%1 bpm").arg(data.heartRate));
    if (m_cards.contains("SQI"))
        m_cards["SQI"]->setValue(QString::number(data.sqi));
}

void MetricsPanel::addMetricCard(const QString &key, const QString &title, const QString &icon) {
    auto *card = new MetricCard(title, icon, this);
    card->setObjectName("Card" + key);
    m_listLayout->addWidget(card, 1); // stretch=1 让每张卡片均分高度
    m_cards.insert(key, card);
}

