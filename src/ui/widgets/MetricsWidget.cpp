#include "MetricsWidget.h"
#include <QVBoxLayout>
#include <QRandomGenerator>
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
    m_scrollArea->setWidgetResizable(true); // 让内部 widget 自动调整宽度
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_container = new QWidget(m_scrollArea);
    m_container->setObjectName("MetricsContainer");

    // 使用垂直布局，无需手动计算 Grid 位置
    m_listLayout = new QVBoxLayout(m_container);
    m_listLayout->setContentsMargins(15, 10, 15, 10);
    m_listLayout->setSpacing(15);
    // 顶部对齐，防止卡片数量少时分散
    m_listLayout->setAlignment(Qt::AlignTop);

    // 初始化指标卡片并设置对象名称，用于 QSS 样式定制
    m_cardHR = new MetricCard(QStringLiteral("心率"), "❤️", m_container);
    m_cardHR->setObjectName("CardHR");
    m_cardHR->setFixedHeight(110); // 固定高度，长方形效果

    m_cardSpO2 = new MetricCard(QStringLiteral("血氧"), "🩸", m_container);
    m_cardSpO2->setObjectName("CardSpO2");
    m_cardSpO2->setFixedHeight(110);

    m_cardRR = new MetricCard(QStringLiteral("呼吸率"), "🫁", m_container);
    m_cardRR->setObjectName("CardRR");
    m_cardRR->setFixedHeight(110);

    m_listLayout->addWidget(m_cardHR);
    m_listLayout->addWidget(m_cardSpO2);
    m_listLayout->addWidget(m_cardRR);

    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MetricsWidget::updateMetrics);
    m_timer->start(1000);
}


void MetricsWidget::updateMetrics() {
    int hr = QRandomGenerator::global()->bounded(60, 100);
    int sp = QRandomGenerator::global()->bounded(95, 100);
    int rr = QRandomGenerator::global()->bounded(12, 22);

    m_cardHR->setValue(QString("%1 bpm").arg(hr));
    m_cardSpO2->setValue(QString("%1 %").arg(sp));
    m_cardRR->setValue(QString("%1 rpm").arg(rr));
}
