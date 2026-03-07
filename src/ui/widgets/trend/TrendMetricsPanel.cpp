#include "TrendMetricsPanel.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QStyle>

// ── 指标信息定义 ─────────────────────────────────────────────────────────────
namespace {
using MI = TrendCard::MetricInfo;

const MI kInfoHrAvg = {
    "心率均值（HR Avg）",
    "统计周期内每分钟心跳次数的平均值，反映心脏泵血频率。",
    "成人静息：60–100 bpm",
    "< 60 bpm（心动过缓）可能提示心脏传导阻滞、迷走神经亢进；"
    "> 100 bpm（心动过速）可能提示发热、贫血、心律失常或应激反应。"
};
const MI kInfoBrAvg = {
    "呼吸率均值（BR Avg）",
    "统计周期内每秒呼吸频率的平均值，反映通气功能。",
    "成人静息：0.20–0.35 Hz（约 12–21 次/分）",
    "< 0.20 Hz（呼吸过缓）可能提示镇静过度、颅内压增高；"
    "> 0.35 Hz（呼吸急促）可能提示肺部感染、心衰或代谢性酸中毒。"
};
const MI kInfoSqiAvg = {
    "信号质量指数均值（SQI Avg）",
    "传感器采集信号的质量评分，0（最差）到 1（最佳），反映数据可信程度。",
    "可信区间：≥ 0.5（建议以 ≥ 0.7 为高质量）",
    "持续低于 0.5 表明信号受干扰，相关生命体征数值可靠性下降，需检查传感器佩戴或患者体动情况。"
};
const MI kInfoSdnn = {
    "SDNN 中位数",
    "Normal-to-Normal 间期（相邻两次正常心跳的时间间隔）的标准差中位数，"
    "综合反映交感与副交感神经对心率的调节能力。",
    "健康成人 5 分钟记录：SDNN 50–100 ms；24 小时：> 100 ms",
    "< 50 ms 提示自主神经调节功能显著下降，见于心衰、糖尿病自主神经病变、心肌梗死后等高风险状态。"
};
const MI kInfoRmssd = {
    "RMSSD 中位数",
    "相邻 NN 间期差值的均方根中位数，主要反映副交感（迷走）神经活性，"
    "对短时心率变异性最敏感。",
    "健康成人静息：20–50 ms",
    "< 20 ms 提示副交感活性低下，常见于过度疲劳、慢性压力、心脏疾病；"
    "> 80 ms 可能提示迷走神经高度活跃，运动员中常见。"
};
const MI kInfoSdsd = {
    "SDSD 中位数",
    "相邻 NN 间期差值的标准差中位数，与 RMSSD 含义接近，侧重短时迷走调节变化量。",
    "参考值与 RMSSD 类似：20–50 ms",
    "持续偏低提示短时心率变异性受抑制，通常与 RMSSD 一同解读。"
};
const MI kInfoPnn50 = {
    "pNN50 中位数",
    "相邻 NN 间期差值超过 50 ms 的比例中位数，高度敏感地反映副交感神经活性。",
    "健康成人静息：3%–20%（随年龄增长下降）",
    "< 3% 强烈提示迷走张力下降，常见于心脏自主神经功能障碍；"
    "运动员或深度放松时可高达 30%–50%。"
};
const MI kInfoPnn20 = {
    "pNN20 中位数",
    "相邻 NN 间期差值超过 20 ms 的比例中位数，比 pNN50 对轻度变化更敏感。",
    "健康成人静息：10%–50%",
    "异常偏低同 pNN50，可更早提示自主神经功能轻度受损。"
};
const MI kInfoLfHf = {
    "LF/HF 比值",
    "低频功率（0.04–0.15 Hz）与高频功率（0.15–0.40 Hz）之比，"
    "被用作交感/副交感平衡的粗略指标。",
    "健康成人静息：0.5–2.0",
    "> 2.0 提示交感神经相对占优（应激、疼痛、焦虑）；"
    "< 0.5 提示副交感占优或总体心率变异性极低。该比值受体位、呼吸等影响，需结合临床解读。"
};
const MI kInfoHf = {
    "HF 均值（高频功率）",
    "心率变异性频谱中 0.15–0.40 Hz 频段的功率均值，主要反映呼吸性窦性心律不齐，"
    "是副交感（迷走）神经活性的直接指标。",
    "短时记录（5 min）健康成人：约 500–2000 ms²（因年龄、体位差异大）",
    "持续偏低提示迷走张力下降；急性心梗、心衰、糖尿病自主神经病变患者常见 HF 明显下降。"
};
const MI kInfoLf = {
    "LF 均值（低频功率）",
    "心率变异性频谱中 0.04–0.15 Hz 频段的功率均值，反映交感与副交感共同调节，"
    "受压力感受器反射影响显著。",
    "健康成人 5 分钟：约 500–2000 ms²",
    "明显升高提示交感激活增强；极度降低提示整体自主神经调节受损。"
};
const MI kInfoVlf = {
    "VLF 均值（极低频功率）",
    "心率变异性频谱中 0.003–0.04 Hz 极低频段的功率均值，"
    "机制复杂，与体温调节、肾素-血管紧张素系统及神经体液调节相关。",
    "需在长时记录（≥ 24 小时）中解读；极低或缺失提示自主神经调节严重受损。",
    "短时记录下 VLF 极低提示植物神经功能重度障碍，与全因死亡率升高相关。"
};
const MI kInfoTp = {
    "总功率均值（Total Power）",
    "心率变异性频谱全频段（≤ 0.40 Hz）功率之和的均值，综合反映整体自主神经调节水平。",
    "健康成人 5 分钟参考值：约 1000–4000 ms²（因年龄和体位差异大）",
    "总功率显著下降是自主神经调节功能全面受损的标志，常见于严重心脏病、终末期心衰或全身性疾病。"
};
} // namespace

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
    m_cardHrAvg  = new TrendCard(QStringLiteral("心率均值"),     kInfoHrAvg,  QStringLiteral("bpm"), QColor(0xFF, 0x52, 0x52), scrollContent);
    m_cardBrAvg  = new TrendCard(QStringLiteral("呼吸率均值"),   kInfoBrAvg,  QStringLiteral("Hz"),  QColor(0x21, 0x96, 0xF3), scrollContent);
    m_cardSqiAvg = new TrendCard(QStringLiteral("信号质量均值"), kInfoSqiAvg, QString{},             QColor(0x7E, 0x57, 0xC2), scrollContent);

    m_cardSdnn  = new TrendCard(QStringLiteral("SDNN 中位数"),  kInfoSdnn,  QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), scrollContent);
    m_cardRmssd = new TrendCard(QStringLiteral("RMSSD 中位数"), kInfoRmssd, QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), scrollContent);
    m_cardSdsd  = new TrendCard(QStringLiteral("SDSD 中位数"),  kInfoSdsd,  QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), scrollContent);
    m_cardPnn50 = new TrendCard(QStringLiteral("pNN50 中位数"), kInfoPnn50, QString{},            QColor(0x00, 0x96, 0x88), scrollContent);
    m_cardPnn20 = new TrendCard(QStringLiteral("pNN20 中位数"), kInfoPnn20, QString{},            QColor(0x00, 0x96, 0x88), scrollContent);

    m_cardLfHfRatio = new TrendCard(QStringLiteral("LF/HF 比值"), kInfoLfHf, QString{}, QColor(0xFF, 0x98, 0x00), scrollContent);
    m_cardHf        = new TrendCard(QStringLiteral("HF 均值"),    kInfoHf,   QString{}, QColor(0x66, 0xBB, 0x6A), scrollContent);
    m_cardLf        = new TrendCard(QStringLiteral("LF 均值"),    kInfoLf,   QString{}, QColor(0xFF, 0xB3, 0x00), scrollContent);
    m_cardVlf       = new TrendCard(QStringLiteral("VLF 均值"),   kInfoVlf,  QString{}, QColor(0xEF, 0x53, 0x50), scrollContent);
    m_cardTp        = new TrendCard(QStringLiteral("总功率均值"), kInfoTp,   QString{}, QColor(0x78, 0x90, 0x9C), scrollContent);

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

    auto apply = [](TrendCard *card, const VitalsTrendService::MetricSeries &s) {
        card->setData(s.timestamps, s.points, s.refValue);
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
void TrendMetricsPanel::setStatus(const QString &message, bool isError) {
    m_statusLabel->setText(message);
    m_statusLabel->setProperty("state", isError ? "error" : "loading");
    m_statusLabel->setVisible(!message.isEmpty());
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

// ── 槽：清空 ─────────────────────────────────────────────────────────────────
void TrendMetricsPanel::clearAll() const {
    for (auto *card : {
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


