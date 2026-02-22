#include "MetricsWidget.h"
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QScrollArea>

MetricsWidget::MetricsWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("MetricsWidget");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* titleLabel = new QLabel(QStringLiteral("实时健康指标"), this);
    titleLabel->setObjectName("MetricsTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_container = new QWidget(m_scrollArea);
    m_container->setObjectName("MetricsContainer");

    m_gridLayout = new QGridLayout(m_container);
    m_gridLayout->setContentsMargins(15, 10, 15, 10);
    m_gridLayout->setSpacing(15);

    m_cardHR = new MetricCard(QStringLiteral("心率"), "❤️", "#FF5252", m_container);
    m_cardSpO2 = new MetricCard(QStringLiteral("血氧"), "🩸", "#4CAF50", m_container);
    m_cardRR = new MetricCard(QStringLiteral("呼吸率"), "🫁", "#2196F3", m_container);

    m_gridLayout->addWidget(m_cardHR, 0, 0);
    m_gridLayout->addWidget(m_cardSpO2, 1, 0);
    m_gridLayout->addWidget(m_cardRR, 2, 0);

    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MetricsWidget::updateMetrics);
    m_timer->start(1000);
}

void MetricsWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rearrangeLayout();
}

void MetricsWidget::rearrangeLayout() {
    int spacing = m_gridLayout->spacing();
    QMargins margins = m_gridLayout->contentsMargins();

    // Set a fixed height for rectangular cards
    int cardHeight = 110;

    // Use minimumWidth to ensure it doesn't shrink too much,
    // but don't set fixed size to allow width filling
    m_cardHR->setFixedHeight(cardHeight);
    m_cardSpO2->setFixedHeight(cardHeight);
    m_cardRR->setFixedHeight(cardHeight);
}

void MetricsWidget::updateMetrics() {
    int hr = QRandomGenerator::global()->bounded(60, 100);
    int sp = QRandomGenerator::global()->bounded(95, 100);
    int rr = QRandomGenerator::global()->bounded(12, 22);

    m_cardHR->setValue(QString("%1 bpm").arg(hr));
    m_cardSpO2->setValue(QString("%1 %").arg(sp));
    m_cardRR->setValue(QString("%1 rpm").arg(rr));
}
