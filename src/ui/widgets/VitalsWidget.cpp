#include "VitalsWidget.h"
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QScrollArea>

VitalsWidget::VitalsWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("VitalsWidget");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* titleLabel = new QLabel(QStringLiteral("实时健康指标监控"), this);
    titleLabel->setObjectName("VitalsTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 配置滚动区域以兼容未来更多指标项的扩展显示
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_container = new QWidget(m_scrollArea);
    m_container->setObjectName("VitalsContainer");

    m_listLayout = new QVBoxLayout(m_container);
    m_listLayout->setContentsMargins(15, 10, 15, 10);
    m_listLayout->setSpacing(15);
    // 强制顶部对齐，维持卡片布局的垂向紧凑性
    m_listLayout->setAlignment(Qt::AlignTop);

    // 批量初始化各监测维度卡片，预设高度以固定显示比例
    m_cardHR = new VitalCard(QStringLiteral("心率"), ":/icons/Heartbeat.png", m_container);
    m_cardHR->setObjectName("CardHR");
    m_cardHR->setFixedHeight(110); // 固定高度，长方形效果

    m_cardSpO2 = new VitalCard(QStringLiteral("血氧"), ":/icons/SpO2.png", m_container);
    m_cardSpO2->setObjectName("CardSpO2");
    m_cardSpO2->setFixedHeight(110);

    m_cardRR = new VitalCard(QStringLiteral("呼吸率"), ":/icons/RespiratoryRate.png", m_container);
    m_cardRR->setObjectName("CardRR");
    m_cardRR->setFixedHeight(110);

    m_listLayout->addWidget(m_cardHR);
    m_listLayout->addWidget(m_cardSpO2);
    m_listLayout->addWidget(m_cardRR);

    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea);

    // 启用 1s 周期的定时器，驱动 UI 数据层的实时跃动
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &VitalsWidget::updateVitals);
    m_timer->start(1000);
}

/**
 * @brief 驱动数值更新逻辑
 * 目前采用随机数模拟真实链路的数据反馈。
 */
void VitalsWidget::updateVitals() {
    int hr = QRandomGenerator::global()->bounded(60, 100);
    int sp = QRandomGenerator::global()->bounded(95, 100);
    int rr = QRandomGenerator::global()->bounded(12, 22);

    m_cardHR->setValue(QString("%1 bpm").arg(hr));
    m_cardSpO2->setValue(QString("%1 %").arg(sp));
    m_cardRR->setValue(QString("%1 rpm").arg(rr));
}
