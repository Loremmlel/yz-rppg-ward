#include "MetricCard.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

MetricCard::MetricCard(const QString& title, const QString& emoji, QWidget *parent)
    : QFrame(parent) {

    // 设置对象名称以便于 QSS 样式控制
    // 注意：具体实例的颜色样式将在 QSS 中通过 ID 选择器配置
    this->setObjectName("MetricCard");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->setFrameShape(QFrame::StyledPanel); // 确保样式表生效

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(15);

    auto* leftContainer = new QVBoxLayout();
    leftContainer->setSpacing(5);

    m_iconLabel = new QLabel(emoji, this);
    m_iconLabel->setObjectName("MetricIcon");

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("MetricTitle");

    leftContainer->addWidget(m_iconLabel, 0, Qt::AlignLeft);
    leftContainer->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setObjectName("MetricValue");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addLayout(leftContainer);
    layout->addStretch();
    layout->addWidget(m_valueLabel);
}

void MetricCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}

