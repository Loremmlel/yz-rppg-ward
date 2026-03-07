#include "TrendCard.h"
#include "TrendChart.h"
#include "../AppDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <cmath>

TrendCard::TrendCard(const QString    &title,
                     const MetricInfo &info,
                     const QString    &unit,
                     const QColor     &accentColor,
                     QWidget          *parent)
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
    mainLayout->setContentsMargins(14, 10, 10, 10);
    mainLayout->setSpacing(6);

    // ── 顶部行：[名称 ⓘ]   |   [数值] ──
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(6);

    // 名称
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("trendCardTitle");
    m_titleLabel->setStyleSheet(
        "font-size: 12px; color: #495057; font-weight: 600; background: transparent;");

    // ⓘ 提示按钮
    auto *infoBtn = new QPushButton(QStringLiteral("ⓘ"), this);
    infoBtn->setObjectName("trendCardInfoBtn");
    infoBtn->setFixedSize(18, 18);
    infoBtn->setCursor(Qt::PointingHandCursor);
    infoBtn->setToolTip(QStringLiteral("查看指标说明"));
    infoBtn->setStyleSheet(
        "QPushButton#trendCardInfoBtn {"
        "  background: transparent; border: none;"
        "  color: #ADB5BD; font-size: 13px; padding: 0;"
        "}"
        "QPushButton#trendCardInfoBtn:hover  { color: #4A90D9; }"
        "QPushButton#trendCardInfoBtn:pressed { color: #357ABD; }"
    );

    // 捕获 info 副本，点击时弹出 AppDialog
    connect(infoBtn, &QPushButton::clicked, this, [info, title] {
        AppDialog::instance()->showInfo(title, buildDialogContent(info));
    });

    // 数值
    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setObjectName("trendCardValue");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_valueLabel->setStyleSheet(QString(
        "font-size: 20px; font-weight: bold; color: %1;"
        "font-family: \"Segoe UI\", \"PingFang SC\", sans-serif;"
        "background: transparent;"
    ).arg(accentColor.name()));

    topRow->addWidget(m_titleLabel);
    topRow->addWidget(infoBtn);
    topRow->addStretch();
    topRow->addWidget(m_valueLabel);
    mainLayout->addLayout(topRow);

    // ── 下部：折线图 ──
    m_chart = new TrendChart(accentColor, this);
    m_chart->setMinimumHeight(160);
    mainLayout->addWidget(m_chart, 1);
}

// ── 公开接口 ──────────────────────────────────────────────────────────────────
void TrendCard::setData(const QList<QDateTime>             &timestamps,
                        const QList<std::optional<double>> &points,
                        const std::optional<double>         refValue) const
{
    std::optional<double> lastValid;
    for (int i = points.size() - 1; i >= 0; --i) {
        if (points[i].has_value()) { lastValid = points[i]; break; }
    }

    if (lastValid.has_value()) {
        int prec = 2;
        if (std::abs(*lastValid) >= 100.0) prec = 0;
        else if (std::abs(*lastValid) >= 10.0) prec = 1;
        const QString txt = formatValue(*lastValid, prec);
        m_valueLabel->setText(m_unit.isEmpty() ? txt : txt + " " + m_unit);
    } else {
        m_valueLabel->setText("--");
    }

    m_chart->setData(timestamps, points, refValue);
}

void TrendCard::clearData() const {
    m_valueLabel->setText("--");
    m_chart->clearData();
}

// ── 私有工具 ──────────────────────────────────────────────────────────────────
QString TrendCard::formatValue(double v, int precision) {
    const double factor = std::pow(10.0, precision);
    return QString::number(std::floor(v * factor) / factor, 'f', precision);
}

QString TrendCard::buildDialogContent(const MetricInfo &info) {
    // 用 HTML 构建带样式的多节内容
    return QString(
        "<p style='margin:0 0 10px 0;'>"
        "  <span style='color:#212529;font-weight:bold;'>%1</span>"
        "</p>"
        "<p style='margin:0 0 6px 0;color:#6C757D;font-size:12px;'>📖 含义</p>"
        "<p style='margin:0 0 12px 0;color:#343A40;'>%2</p>"
        "<p style='margin:0 0 6px 0;color:#6C757D;font-size:12px;'>✅ 正常范围</p>"
        "<p style='margin:0 0 12px 0;color:#343A40;'>%3</p>"
        "<p style='margin:0 0 6px 0;color:#6C757D;font-size:12px;'>⚠️ 异常提示</p>"
        "<p style='margin:0;color:#343A40;'>%4</p>"
    )
    .arg(info.name, info.meaning, info.normalRange, info.abnormal);
}
