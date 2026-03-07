#include "TrendCard.h"
#include "TrendChart.h"

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
    , m_accentColor(accentColor)
{
    setObjectName("TrendCard");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setFrameShape(QFrame::StyledPanel);

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

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 10, 14, 10);
    mainLayout->setSpacing(6);

    // ── 顶部行：图标+名称  |  数值 ──
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    auto *leftCol = new QVBoxLayout();
    leftCol->setSpacing(2);

    m_iconLabel = new QLabel(icon, this);
    m_iconLabel->setObjectName("trendCardIcon");
    m_iconLabel->setStyleSheet("font-size: 18px;");

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("trendCardTitle");
    m_titleLabel->setStyleSheet("font-size: 11px; color: #6C757D; font-weight: 500;");

    leftCol->addWidget(m_iconLabel);
    leftCol->addWidget(m_titleLabel);

    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setObjectName("trendCardValue");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_valueLabel->setStyleSheet(QString(
        "font-size: 20px;"
        "font-weight: bold;"
        "color: %1;"
        "font-family: \"Segoe UI\", \"PingFang SC\", sans-serif;"
    ).arg(accentColor.name()));

    topRow->addLayout(leftCol);
    topRow->addStretch();
    topRow->addWidget(m_valueLabel);
    mainLayout->addLayout(topRow);

    // ── 下部：折线图 ──
    m_chart = new TrendChart(accentColor, this);
    m_chart->setMinimumHeight(80);
    mainLayout->addWidget(m_chart, 1);
}

// ── 公开接口 ──────────────────────────────────────────────────────────────────
void TrendCard::setData(const QList<std::optional<double>> &points,
                        const std::optional<double> refValue) const {
    // 找最后一个有效值作为顶部展示数值
    std::optional<double> lastValid;
    for (int i = points.size() - 1; i >= 0; --i) {
        if (points[i].has_value()) { lastValid = points[i]; break; }
    }

    if (lastValid.has_value()) {
        // 用 refValue 的精度作为显示精度（粗略）；无 refValue 时用 2 位
        int prec = 2;
        if (refValue.has_value() && std::abs(*refValue) >= 10.0)  prec = 1;
        if (refValue.has_value() && std::abs(*refValue) >= 100.0) prec = 0;
        const QString txt = formatValue(*lastValid, prec);
        m_valueLabel->setText(m_unit.isEmpty() ? txt : txt + " " + m_unit);
    } else {
        m_valueLabel->setText("--");
    }

    m_chart->setData(points, refValue);
}

void TrendCard::clearData() const {
    m_valueLabel->setText("--");
    m_chart->clearData();
}

// ── 私有工具 ──────────────────────────────────────────────────────────────────
QString TrendCard::formatValue(const double v, const int precision) {
    const double factor = std::pow(10.0, precision);
    const double truncated = std::floor(v * factor) / factor;
    return QString::number(truncated, 'f', precision);
}
