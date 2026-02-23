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

    addVitalCard("HR", QStringLiteral("心率"), ":/icons/Heartbeat.png");
    addVitalCard("SpO2", QStringLiteral("血氧"), ":/icons/SpO2.png");
    addVitalCard("RR", QStringLiteral("呼吸率"), ":/icons/RespiratoryRate.png");

    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea);
}

/**
 * @brief 更新体征指标视图
 * 接收来自核心业务层的结构化数据快照并解包到对应卡片组件。
 */
void VitalsWidget::updateData(const VitalData& data) {
    if (m_cards.contains("HR")) m_cards["HR"]->setValue(QString("%1 bpm").arg(data.heartRate));
    if (m_cards.contains("SpO2")) m_cards["SpO2"]->setValue(QString("%1 %").arg(data.SpO2));
    if (m_cards.contains("RR")) m_cards["RR"]->setValue(QString("%1 rpm").arg(data.respirationRate));
}

void VitalsWidget::addVitalCard(const QString &key, const QString &title, const QString &iconPath) {
    auto card = new VitalCard(title, iconPath, m_container);
    card->setObjectName("Card_" + key);
    card->setFixedHeight(110);
    m_listLayout->addWidget(card);
    m_cards.insert(key, card);
}
