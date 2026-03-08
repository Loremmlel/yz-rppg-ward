#include "TrendMetricsPanel.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QStyle>

#include "../../../model/MetricInfoData.h"

using namespace MetricInfoData;

// ── 构造 ─────────────────────────────────────────────────────────────────────
TrendMetricsPanel::TrendMetricsPanel(QWidget *parent) : QWidget(parent) {
    setObjectName("trendMetricsPanel");

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // ── 状态标签 ──
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("trendStatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setVisible(false);
    outerLayout->addWidget(m_statusLabel);

    // ── 可滚动卡片区域 ──
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("trendScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName("trendScrollContent");
    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea, 1);

    // ── 创建所有卡片（无 icon，传 MetricInfo） ──
    m_cardHrAvg  = new TrendCard(QStringLiteral("心率均值"),     kHrAvg,      QStringLiteral("bpm"), QColor(0xFF, 0x52, 0x52), scrollContent);
    m_cardBrAvg  = new TrendCard(QStringLiteral("呼吸率均值"),   kBrAvg,      QStringLiteral("Hz"),  QColor(0x21, 0x96, 0xF3), scrollContent);
    m_cardSqiAvg = new TrendCard(QStringLiteral("信号质量均值"), kSqiAvg,     QString{},             QColor(0x7E, 0x57, 0xC2), scrollContent);

    m_cardSdnn  = new TrendCard(QStringLiteral("SDNN 中位数"),  kSdnnMedian,  QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), scrollContent);
    m_cardRmssd = new TrendCard(QStringLiteral("RMSSD 中位数"), kRmssdMedian, QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), scrollContent);
    m_cardSdsd  = new TrendCard(QStringLiteral("SDSD 中位数"),  kSdsdMedian,  QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), scrollContent);
    m_cardPnn50 = new TrendCard(QStringLiteral("pNN50 中位数"), kPnn50Median, QString{},            QColor(0x00, 0x96, 0x88), scrollContent);
    m_cardPnn20 = new TrendCard(QStringLiteral("pNN20 中位数"), kPnn20Median, QString{},            QColor(0x00, 0x96, 0x88), scrollContent);

    m_cardLfHfRatio = new TrendCard(QStringLiteral("LF/HF 比值"), kLfHfRatio, QString{}, QColor(0xFF, 0x98, 0x00), scrollContent);
    m_cardHf        = new TrendCard(QStringLiteral("HF 均值"),    kHfAvg,     QString{}, QColor(0x66, 0xBB, 0x6A), scrollContent);
    m_cardLf        = new TrendCard(QStringLiteral("LF 均值"),    kLfAvg,     QString{}, QColor(0xFF, 0xB3, 0x00), scrollContent);
    m_cardVlf       = new TrendCard(QStringLiteral("VLF 均值"),   kVlfAvg,    QString{}, QColor(0xEF, 0x53, 0x50), scrollContent);
    m_cardTp        = new TrendCard(QStringLiteral("总功率均值"), kTpAvg,     QString{}, QColor(0x78, 0x90, 0x9C), scrollContent);

    // ── 布局三个分组 ──
    auto *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(20, 16, 20, 20);
    contentLayout->setSpacing(20);

    buildGroup(contentLayout, QStringLiteral("基础生命体征"),
               { m_cardHrAvg, m_cardBrAvg, m_cardSqiAvg });
    buildGroup(contentLayout, QStringLiteral("HRV 时域中位数"),
               { m_cardSdnn, m_cardRmssd, m_cardSdsd, m_cardPnn50, m_cardPnn20 });
    buildGroup(contentLayout, QStringLiteral("HRV 频域均值"),
               { m_cardLfHfRatio, m_cardHf, m_cardLf, m_cardVlf, m_cardTp });

    contentLayout->addStretch();
}

// ── 槽：填充查询结果 ──────────────────────────────────────────────────────────
void TrendMetricsPanel::applyResult(const VitalsTrendService::TrendResult &r) const {
    m_statusLabel->setVisible(false);

    auto apply = [&r](const TrendCard *card, const VitalsTrendService::MetricSeries &s) {
        card->setData(s.timestamps, s.points, s.refValue, r.queryStart, r.queryEnd, r.intervalSecs);
    };

    apply(m_cardHrAvg,      r.hrAvg);
    apply(m_cardBrAvg,      r.brAvg);
    apply(m_cardSqiAvg,     r.sqiAvg);
    apply(m_cardSdnn,       r.sdnnMedian);
    apply(m_cardRmssd,      r.rmssdMedian);
    apply(m_cardSdsd,       r.sdsdMedian);
    apply(m_cardPnn50,      r.pnn50Median);
    apply(m_cardPnn20,      r.pnn20Median);
    apply(m_cardLfHfRatio,  r.lfHfRatio);
    apply(m_cardHf,         r.hfAvg);
    apply(m_cardLf,         r.lfAvg);
    apply(m_cardVlf,        r.vlfAvg);
    apply(m_cardTp,         r.tpAvg);
}

// ── 槽：状态信息 ─────────────────────────────────────────────────────────────
void TrendMetricsPanel::setStatus(const QString &message, bool isError) const {
    m_statusLabel->setText(message);
    m_statusLabel->setProperty("state", isError ? "error" : "loading");
    m_statusLabel->setVisible(!message.isEmpty());
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

// ── 槽：清空 ─────────────────────────────────────────────────────────────────
void TrendMetricsPanel::clearAll() const {
    for (const auto *card : {
        m_cardHrAvg, m_cardBrAvg, m_cardSqiAvg,
        m_cardSdnn, m_cardRmssd, m_cardSdsd, m_cardPnn50, m_cardPnn20,
        m_cardLfHfRatio, m_cardHf, m_cardLf, m_cardVlf, m_cardTp
    }) { card->clearData(); }
}

// ── 私有：构建分组 ────────────────────────────────────────────────────────────
void TrendMetricsPanel::buildGroup(QVBoxLayout *layout,
                                   const QString &title,
                                   const QList<TrendCard *> &cards)
{
    auto *group = new QGroupBox(title);
    group->setObjectName("TrendGroup");

    auto *grid = new QGridLayout(group);
    grid->setContentsMargins(12, 16, 12, 12);
    grid->setSpacing(10);

    constexpr int kCols = 3;
    for (int i = 0; i < cards.size(); ++i)
        grid->addWidget(cards[i], i / kCols, i % kCols);
    for (int col = 0; col < kCols; ++col)
        grid->setColumnStretch(col, 1);

    layout->addWidget(group);
}


