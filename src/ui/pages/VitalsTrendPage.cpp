#include "VitalsTrendPage.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QFrame>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QDateTime>
#include <algorithm>

#include "../../service/ApiClient.h"
#include "../../service/ConfigService.h"
#include "../../util/StyleLoader.h"

// ── 时间粒度映射 ────────────────────────────────────────────────────────────
struct IntervalItem {
    QString label;    ///< 下拉框显示文本
    QString apiValue; ///< 传给服务端的 interval 参数
};

static const QList<IntervalItem> kIntervalItems = {
    { QStringLiteral("1 分钟"),  QStringLiteral("1m")  },
    { QStringLiteral("5 分钟"),  QStringLiteral("5m")  },
    { QStringLiteral("10 分钟"), QStringLiteral("10m") },
    { QStringLiteral("15 分钟"), QStringLiteral("15m") },
    { QStringLiteral("30 分钟"), QStringLiteral("30m") },
    { QStringLiteral("1 小时"),  QStringLiteral("1h")  },
    { QStringLiteral("6 小时"),  QStringLiteral("6h")  },
    { QStringLiteral("12 小时"), QStringLiteral("12h") },
    { QStringLiteral("1 天"),    QStringLiteral("1d")  },
};

// ── 解析辅助 ────────────────────────────────────────────────────────────────
static std::optional<double> parseOptDouble(const QJsonObject &obj, const QString &key) {
    if (obj.contains(key) && !obj.value(key).isNull())
        return obj.value(key).toDouble();
    return std::nullopt;
}

// ============================================================
VitalsTrendPage::VitalsTrendPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("VitalsTrendPage");
    initUI();
    StyleLoader::apply(this, QStringLiteral(":/styles/vitals_trend_page.qss"));
}

// ── 整体布局 ────────────────────────────────────────────────────────────────
void VitalsTrendPage::initUI() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    setupControlPanel();
    rootLayout->addWidget(findChild<QFrame *>("trendControlPanel"));

    // 状态/错误标签（控制面板下方，数据区上方）
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("trendStatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setVisible(false);
    rootLayout->addWidget(m_statusLabel);

    // 可滚动指标区
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("trendScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName("trendScrollContent");
    setupMetricsArea();     // 在内部构建卡片，稍后重新挂载到 scrollContent
    // 注意：setupMetricsArea 将卡片写入 m_card* 成员，但不挂载到任何 layout，
    // 下面重新构建。
    scrollArea->setWidget(scrollContent);
    rootLayout->addWidget(scrollArea, 1);

    // ── 在 scrollContent 内构建三个分组 ──
    auto *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(20, 16, 20, 20);
    contentLayout->setSpacing(20);

    buildGroupSection(contentLayout,
        QStringLiteral("基础生命体征"),
        { m_cardHrAvg, m_cardBrAvg, m_cardSqiAvg });

    buildGroupSection(contentLayout,
        QStringLiteral("HRV 时域中位数"),
        { m_cardSdnn, m_cardRmssd, m_cardSdsd, m_cardPnn50, m_cardPnn20 });

    buildGroupSection(contentLayout,
        QStringLiteral("HRV 频域均值"),
        { m_cardLfHfRatio, m_cardHf, m_cardLf, m_cardVlf, m_cardTp });

    contentLayout->addStretch();
}

// ── 控制面板 ────────────────────────────────────────────────────────────────
void VitalsTrendPage::setupControlPanel() {
    auto *panel = new QFrame(this);
    panel->setObjectName("trendControlPanel");

    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(20, 12, 20, 12);
    panelLayout->setSpacing(10);

    // ── 第一行：时间粒度 + 快捷时间按钮 ──
    auto *row1 = new QHBoxLayout();
    row1->setSpacing(8);

    auto *intervalLabel = new QLabel(QStringLiteral("时间粒度："), panel);
    intervalLabel->setObjectName("trendLabel");

    m_intervalCombo = new QComboBox(panel);
    m_intervalCombo->setObjectName("TrendCombo");
    m_intervalCombo->setFixedWidth(110);
    for (const auto &item : kIntervalItems)
        m_intervalCombo->addItem(item.label, item.apiValue);
    m_intervalCombo->setCurrentIndex(1); // 默认 5 分钟

    row1->addWidget(intervalLabel);
    row1->addWidget(m_intervalCombo);
    row1->addSpacing(16);

    // 快捷时间按钮
    auto *shortcutLabel = new QLabel(QStringLiteral("快捷时间："), panel);
    shortcutLabel->setObjectName("trendLabel");
    row1->addWidget(shortcutLabel);

    const QList<QPair<QString, int>> shortcuts = {
        { QStringLiteral("最近 1 小时"),  1  },
        { QStringLiteral("最近 6 小时"),  6  },
        { QStringLiteral("最近 24 小时"), 24 },
    };
    for (const auto &[btnText, hours] : shortcuts) {
        auto *btn = new QPushButton(btnText, panel);
        btn->setObjectName("TrendShortcutButton");
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, hours]() {
            onShortcutClicked(hours);
        });
        row1->addWidget(btn);
    }
    row1->addStretch();

    // ── 第二行：自定义时间范围 + 查询按钮 ──
    auto *row2 = new QHBoxLayout();
    row2->setSpacing(8);

    auto *startLabel = new QLabel(QStringLiteral("开始时间："), panel);
    startLabel->setObjectName("trendLabel");

    m_startEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), panel);
    m_startEdit->setObjectName("TrendDateTimeEdit");
    m_startEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_startEdit->setCalendarPopup(true);
    m_startEdit->setFixedWidth(180);

    auto *endLabel = new QLabel(QStringLiteral("结束时间："), panel);
    endLabel->setObjectName("trendLabel");

    m_endEdit = new QDateTimeEdit(QDateTime::currentDateTime(), panel);
    m_endEdit->setObjectName("TrendDateTimeEdit");
    m_endEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_endEdit->setCalendarPopup(true);
    m_endEdit->setFixedWidth(180);

    m_queryBtn = new QPushButton(QStringLiteral("查询"), panel);
    m_queryBtn->setObjectName("PrimaryButton");
    m_queryBtn->setFixedSize(80, 32);
    m_queryBtn->setCursor(Qt::PointingHandCursor);
    connect(m_queryBtn, &QPushButton::clicked, this, &VitalsTrendPage::onQueryClicked);

    row2->addWidget(startLabel);
    row2->addWidget(m_startEdit);
    row2->addSpacing(12);
    row2->addWidget(endLabel);
    row2->addWidget(m_endEdit);
    row2->addSpacing(16);
    row2->addWidget(m_queryBtn);
    row2->addStretch();

    panelLayout->addLayout(row1);
    panelLayout->addLayout(row2);
}

// ── 指标卡片预创建 ────────────────────────────────────────────────────────────
void VitalsTrendPage::setupMetricsArea() {
    // 基础生命体征
    m_cardHrAvg  = new TrendCard(QStringLiteral("心率均值"),     "❤",  QStringLiteral("bpm"), QColor(0xFF, 0x52, 0x52), this);
    m_cardBrAvg  = new TrendCard(QStringLiteral("呼吸率均值"),   "🫁", QStringLiteral("Hz"),  QColor(0x21, 0x96, 0xF3), this);
    m_cardSqiAvg = new TrendCard(QStringLiteral("信号质量均值"), "📶",  QString{},            QColor(0x7E, 0x57, 0xC2), this);

    // HRV 时域
    m_cardSdnn  = new TrendCard(QStringLiteral("SDNN 中位数"),  "📊", QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), this);
    m_cardRmssd = new TrendCard(QStringLiteral("RMSSD 中位数"), "📊", QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), this);
    m_cardSdsd  = new TrendCard(QStringLiteral("SDSD 中位数"),  "📊", QStringLiteral("ms"), QColor(0x26, 0xA6, 0x9A), this);
    m_cardPnn50 = new TrendCard(QStringLiteral("pNN50 中位数"), "📈", QString{},            QColor(0x00, 0x96, 0x88), this);
    m_cardPnn20 = new TrendCard(QStringLiteral("pNN20 中位数"), "📈", QString{},            QColor(0x00, 0x96, 0x88), this);

    // HRV 频域
    m_cardLfHfRatio = new TrendCard(QStringLiteral("LF/HF 比值"), "⚖",  QString{},           QColor(0xFF, 0x98, 0x00), this);
    m_cardHf        = new TrendCard(QStringLiteral("HF 均值"),    "〰",  QString{},           QColor(0x66, 0xBB, 0x6A), this);
    m_cardLf        = new TrendCard(QStringLiteral("LF 均值"),    "〰",  QString{},           QColor(0xFF, 0xB3, 0x00), this);
    m_cardVlf       = new TrendCard(QStringLiteral("VLF 均值"),   "〰",  QString{},           QColor(0xEF, 0x53, 0x50), this);
    m_cardTp        = new TrendCard(QStringLiteral("总功率均值"), "⚡",  QString{},           QColor(0x78, 0x90, 0x9C), this);
}

// ── 构建分组 Box ─────────────────────────────────────────────────────────────
void VitalsTrendPage::buildGroupSection(QVBoxLayout *layout,
                                        const QString &groupTitle,
                                        const QList<TrendCard *> &cards)
{
    auto *group = new QGroupBox(groupTitle);
    group->setObjectName("TrendGroup");

    auto *grid = new QGridLayout(group);
    grid->setContentsMargins(12, 16, 12, 12);
    grid->setSpacing(10);

    // 每行 3 张卡片
    constexpr int kCols = 3;
    for (int i = 0; i < cards.size(); ++i) {
        grid->addWidget(cards[i], i / kCols, i % kCols);
    }
    // 让各列等宽
    for (int col = 0; col < kCols; ++col)
        grid->setColumnStretch(col, 1);

    layout->addWidget(group);
}

// ── 快捷时间按钮响应 ─────────────────────────────────────────────────────────
void VitalsTrendPage::onShortcutClicked(const int hours) const {
    const QDateTime now = QDateTime::currentDateTime();
    m_endEdit->setDateTime(now);
    m_startEdit->setDateTime(now.addSecs(-static_cast<qint64>(hours) * 3600));

    // 根据时间范围自动选择合适粒度
    QString autoInterval;
    if (hours <= 1)        autoInterval = QStringLiteral("1m");
    else if (hours <= 6)   autoInterval = QStringLiteral("5m");
    else                   autoInterval = QStringLiteral("15m");

    for (int i = 0; i < m_intervalCombo->count(); ++i) {
        if (m_intervalCombo->itemData(i).toString() == autoInterval) {
            m_intervalCombo->setCurrentIndex(i);
            break;
        }
    }
}

// ── 查询按钮响应 ─────────────────────────────────────────────────────────────
void VitalsTrendPage::onQueryClicked() {
    const AppConfig cfg = ConfigService::instance()->config();
    if (!cfg.hasBed()) {
        showError(QStringLiteral("⚠ 请先在设置页面配置床位信息"));
        return;
    }

    const QDateTime start = m_startEdit->dateTime();
    const QDateTime end   = m_endEdit->dateTime();
    if (start >= end) {
        showError(QStringLiteral("⚠ 开始时间必须早于结束时间"));
        return;
    }

    const QString interval = m_intervalCombo->currentData().toString();
    fetchTrend(start, end, interval);
}

// ── 发起 HTTP 请求 ───────────────────────────────────────────────────────────
void VitalsTrendPage::fetchTrend(const QDateTime &start,
                                  const QDateTime &end,
                                  const QString   &interval) const {
    setLoading(true);

    const auto cfg = ConfigService::instance()->config();

    // ISO-8601 UTC 格式（服务端要求）
    const QString startStr = start.toUTC().toString(Qt::ISODate);
    const QString endStr   = end.toUTC().toString(Qt::ISODate);

    // 用 QUrlQuery 构建查询参数，Qt 会为每个值独立做百分号编码，
    // 不会误编码 '?' 与 '&' 分隔符
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("bedId"),    QString::number(cfg.bedId));
    query.addQueryItem(QStringLiteral("startTime"), startStr);
    query.addQueryItem(QStringLiteral("endTime"),   endStr);
    query.addQueryItem(QStringLiteral("interval"),  interval);

    ApiClient::instance()->getJson(QStringLiteral("/api/vitals/trend"), query,
        [this](const QJsonDocument &doc) {
            setLoading(false);
            if (!doc.isArray()) {
                showError(QStringLiteral("⚠ 服务端返回数据格式异常"));
                return;
            }

            QList<VitalsTrendData> records;
            const QJsonArray arr = doc.array();
            for (const auto &val : arr) {
                if (!val.isObject()) continue;
                const QJsonObject obj = val.toObject();

                VitalsTrendData rec;
                rec.bucketTime = QDateTime::fromString(
                    obj.value(QStringLiteral("bucketTime")).toString(),
                    Qt::ISODate);

                if (obj.contains(QStringLiteral("basicVitals")) && obj.value(QStringLiteral("basicVitals")).isObject()) {
                    const QJsonObject bv = obj.value(QStringLiteral("basicVitals")).toObject();
                    rec.basicVitals.hrAvg  = parseOptDouble(bv, QStringLiteral("hrAvg"));
                    rec.basicVitals.brAvg  = parseOptDouble(bv, QStringLiteral("brAvg"));
                    rec.basicVitals.sqiAvg = parseOptDouble(bv, QStringLiteral("sqiAvg"));
                }

                if (obj.contains(QStringLiteral("hrvTimeDomain")) && obj.value(QStringLiteral("hrvTimeDomain")).isObject()) {
                    const QJsonObject td = obj.value(QStringLiteral("hrvTimeDomain")).toObject();
                    rec.hrvTimeDomain.sdnnMedian  = parseOptDouble(td, QStringLiteral("sdnnMedian"));
                    rec.hrvTimeDomain.rmssdMedian = parseOptDouble(td, QStringLiteral("rmssdMedian"));
                    rec.hrvTimeDomain.sdsdMedian  = parseOptDouble(td, QStringLiteral("sdsdMedian"));
                    rec.hrvTimeDomain.pnn50Median = parseOptDouble(td, QStringLiteral("pnn50Median"));
                    rec.hrvTimeDomain.pnn20Median = parseOptDouble(td, QStringLiteral("pnn20Median"));
                }

                if (obj.contains(QStringLiteral("hrvFreqDomain")) && obj.value(QStringLiteral("hrvFreqDomain")).isObject()) {
                    const QJsonObject fd = obj.value(QStringLiteral("hrvFreqDomain")).toObject();
                    rec.hrvFreqDomain.lfHfRatio = parseOptDouble(fd, QStringLiteral("lfHfRatio"));
                    rec.hrvFreqDomain.hfAvg     = parseOptDouble(fd, QStringLiteral("hfAvg"));
                    rec.hrvFreqDomain.lfAvg     = parseOptDouble(fd, QStringLiteral("lfAvg"));
                    rec.hrvFreqDomain.vlfAvg    = parseOptDouble(fd, QStringLiteral("vlfAvg"));
                    rec.hrvFreqDomain.tpAvg     = parseOptDouble(fd, QStringLiteral("tpAvg"));
                }

                records.append(rec);
            }

            if (records.isEmpty()) {
                showError(QStringLiteral("📭 该时间段内暂无数据"));
            } else {
                m_statusLabel->setVisible(false);
                applyData(records);
            }
        },
        [this](const QString &err) {
            setLoading(false);
            showError(QStringLiteral("⚠ 请求失败：") + err);
        }
    );
}

// ── 统计辅助 ────────────────────────────────────────────────────────────────

/** 从 records 中提取单个字段的点序列 */
template<typename Extractor>
static QList<std::optional<double>> extractPoints(
    const QList<VitalsTrendData> &records, Extractor extractor)
{
    QList<std::optional<double>> pts;
    pts.reserve(records.size());
    for (const auto &r : records)
        pts.append(extractor(r));
    return pts;
}

/** 计算有效值的算术均值；无有效值返回 nullopt */
static std::optional<double> calcMean(const QList<std::optional<double>> &pts)
{
    double sum = 0.0;
    int cnt = 0;
    for (const auto &p : pts) {
        if (p.has_value()) { sum += *p; ++cnt; }
    }
    return cnt > 0 ? std::optional(sum / cnt) : std::nullopt;
}

/** 计算有效值的中位数；无有效值返回 nullopt */
static std::optional<double> calcMedian(const QList<std::optional<double>> &pts)
{
    QList<double> vals;
    for (const auto &p : pts) if (p.has_value()) vals.append(*p);
    if (vals.isEmpty()) return std::nullopt;
    const int n = vals.size();
    const auto midIter = vals.begin() + n / 2;
    std::ranges::nth_element(vals, midIter);

    if (n % 2 == 1) {
        return *midIter;
    }
    const auto leftMaxIter = std::ranges::max_element(vals.begin(), midIter);
    return std::midpoint(*leftMaxIter, *midIter);
}

// ── 将全量记录展示到各卡片 ───────────────────────────────────────────────────
void VitalsTrendPage::applyData(const QList<VitalsTrendData> &records) const {
    // ── 基础生命体征（参考线 = 均值） ──
    const auto hrPts  = extractPoints(records, [](const VitalsTrendData &r){ return r.basicVitals.hrAvg; });
    const auto brPts  = extractPoints(records, [](const VitalsTrendData &r){ return r.basicVitals.brAvg; });
    const auto sqiPts = extractPoints(records, [](const VitalsTrendData &r){ return r.basicVitals.sqiAvg; });

    m_cardHrAvg ->setData(hrPts,  calcMean(hrPts));
    m_cardBrAvg ->setData(brPts,  calcMean(brPts));
    m_cardSqiAvg->setData(sqiPts, calcMean(sqiPts));

    // ── HRV 时域（后端返回的是各 bucket 的中位数；参考线 = 全局中位数的中位数） ──
    const auto sdnnPts  = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvTimeDomain.sdnnMedian; });
    const auto rmssdPts = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvTimeDomain.rmssdMedian; });
    const auto sdsdPts  = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvTimeDomain.sdsdMedian; });
    const auto pnn50Pts = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvTimeDomain.pnn50Median; });
    const auto pnn20Pts = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvTimeDomain.pnn20Median; });

    m_cardSdnn ->setData(sdnnPts,  calcMedian(sdnnPts));
    m_cardRmssd->setData(rmssdPts, calcMedian(rmssdPts));
    m_cardSdsd ->setData(sdsdPts,  calcMedian(sdsdPts));
    m_cardPnn50->setData(pnn50Pts, calcMedian(pnn50Pts));
    m_cardPnn20->setData(pnn20Pts, calcMedian(pnn20Pts));

    // ── HRV 频域（参考线 = 均值） ──
    const auto lfHfPts = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvFreqDomain.lfHfRatio; });
    const auto hfPts   = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvFreqDomain.hfAvg; });
    const auto lfPts   = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvFreqDomain.lfAvg; });
    const auto vlfPts  = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvFreqDomain.vlfAvg; });
    const auto tpPts   = extractPoints(records, [](const VitalsTrendData &r){ return r.hrvFreqDomain.tpAvg; });

    m_cardLfHfRatio->setData(lfHfPts, calcMean(lfHfPts));
    m_cardHf       ->setData(hfPts,   calcMean(hfPts));
    m_cardLf       ->setData(lfPts,   calcMean(lfPts));
    m_cardVlf      ->setData(vlfPts,  calcMean(vlfPts));
    m_cardTp       ->setData(tpPts,   calcMean(tpPts));
}

// ── 加载状态 ─────────────────────────────────────────────────────────────────
void VitalsTrendPage::setLoading(bool loading) const {
    m_queryBtn->setEnabled(!loading);
    if (loading) {
        m_statusLabel->setText(QStringLiteral("⏳ 正在查询…"));
        m_statusLabel->setProperty("state", "loading");
        m_statusLabel->setVisible(true);
    }
}

void VitalsTrendPage::showError(const QString &message) const {
    m_statusLabel->setText(message);
    m_statusLabel->setProperty("state", "error");
    m_statusLabel->setVisible(true);
    // 刷新 QSS（property 变化需要重新 polish）
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}



