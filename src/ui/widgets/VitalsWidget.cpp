#include "VitalsWidget.h"
#include <QVBoxLayout>
#include <QLabel>
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
}

/**
 * @brief 更新体征指标视图
 * 接收来自核心业务层的结构化数据快照并解包到对应卡片组件。
 */
void VitalsWidget::updateData(const VitalData& data) {
    m_cardHR->setValue(QString("%1 bpm").arg(data.heartRate));
    m_cardSpO2->setValue(QString("%1 %").arg(data.SpO2));
    m_cardRR->setValue(QString("%1 rpm").arg(data.respirationRate));
}
