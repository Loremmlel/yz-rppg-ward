#include "TrendCard.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>

TrendCard::TrendCard(const QString &title,
                     const QString &icon,
                     const QString &unit,
                     const QColor  &accentColor,
                     QWidget       *parent)
    : QFrame(parent)
    , m_unit(unit)
{
    setObjectName("TrendCard");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFrameShape(StyledPanel);

    // 左侧色带通过动态属性控制（QSS 无法直接使用 QColor 变量，改为 inline style）
    setStyleSheet(QString(
        "TrendCard {"
        "  background-color: #FFFFFF;"
        "  border: 1px solid #E0E4E8;"
        "  border-radius: 10px;"
        "  border-left: 4px solid %1;"
        "}"
        "TrendCard:hover {"
        "  background-color: #FAFBFC;"
        "  border-color: #CED4DA;"
        "  border-left: 4px solid %1;"
        "}"
    ).arg(accentColor.name()));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(10);

    // ── 左侧：图标 + 名称 ──
    auto *leftCol = new QVBoxLayout();
    leftCol->setSpacing(2);

    m_iconLabel = new QLabel(icon, this);
    m_iconLabel->setObjectName("trendCardIcon");
    m_iconLabel->setStyleSheet("font-size: 20px;");

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("trendCardTitle");
    m_titleLabel->setStyleSheet("font-size: 12px; color: #6C757D; font-weight: 500;");

    leftCol->addWidget(m_iconLabel);
    leftCol->addWidget(m_titleLabel);

    // ── 右侧：数值 ──
    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setObjectName("trendCardValue");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_valueLabel->setStyleSheet(QString(
        "font-size: 22px;"
        "font-weight: bold;"
        "color: %1;"
        "font-family: \"Segoe UI\", \"PingFang SC\", sans-serif;"
    ).arg(accentColor.name()));

    layout->addLayout(leftCol);
    layout->addStretch();
    layout->addWidget(m_valueLabel);
}

void TrendCard::setValue(const QString &value) const {
    if (value.isEmpty()) {
        m_valueLabel->setText("--");
    } else {
        m_valueLabel->setText(m_unit.isEmpty() ? value : value + " " + m_unit);
    }
}

void TrendCard::setValue(double value, int precision) const {
    const double factor = std::pow(10.0, precision);
    const double truncated = std::floor(value * factor) / factor;
    setValue(QString::number(truncated, 'f', precision));
}

void TrendCard::clearValue() const {
    m_valueLabel->setText("--");
}

