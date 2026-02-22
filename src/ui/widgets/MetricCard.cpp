#include "MetricCard.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

MetricCard::MetricCard(const QString& title, const QString& emoji, const QString& color, QWidget *parent)
    : QFrame(parent), m_baseColor(color) {

    // Set object name for styling
    this->setObjectName("MetricCard");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->setFrameShape(QFrame::StyledPanel); // Important to ensure styling applies correctly

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
    m_valueLabel->setProperty("baseColor", m_baseColor);
    // Explicitly set the color for now, but QSS should handle it based on a custom property
    m_valueLabel->setStyleSheet(QString("color: %1;").arg(color));
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addLayout(leftContainer);
    layout->addStretch();
    layout->addWidget(m_valueLabel);
}

void MetricCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}

void MetricCard::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
