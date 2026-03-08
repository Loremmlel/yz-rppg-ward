#include "TrendControlBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {
    struct IntervalItem {
        QString label;
        QString apiValue;
    };

    const QList<IntervalItem> kIntervals = {
        {QStringLiteral("1 分钟"), QStringLiteral("1m")},
        {QStringLiteral("5 分钟"), QStringLiteral("5m")},
        {QStringLiteral("10 分钟"), QStringLiteral("10m")},
        {QStringLiteral("15 分钟"), QStringLiteral("15m")},
        {QStringLiteral("30 分钟"), QStringLiteral("30m")},
        {QStringLiteral("1 小时"), QStringLiteral("1h")},
        {QStringLiteral("6 小时"), QStringLiteral("6h")},
        {QStringLiteral("12 小时"), QStringLiteral("12h")},
        {QStringLiteral("1 天"), QStringLiteral("1d")},
    };
}

TrendControlBar::TrendControlBar(QWidget *parent) : QFrame(parent) {
    setObjectName("trendControlPanel");
    initUI();
}

void TrendControlBar::initUI() {
    auto *panelLayout = new QVBoxLayout(this);
    panelLayout->setContentsMargins(20, 12, 20, 12);
    panelLayout->setSpacing(10);

    // ── 第一行：时间粒度 + 快捷时间按钮 ──
    auto *row1 = new QHBoxLayout();
    row1->setSpacing(8);

    auto *intervalLabel = new QLabel(QStringLiteral("时间粒度："), this);
    intervalLabel->setObjectName("trendLabel");

    m_intervalCombo = new QComboBox(this);
    m_intervalCombo->setObjectName("TrendCombo");
    m_intervalCombo->setFixedWidth(110);
    for (const auto &item: kIntervals)
        m_intervalCombo->addItem(item.label, item.apiValue);
    m_intervalCombo->setCurrentIndex(1); // 默认 5 分钟

    row1->addWidget(intervalLabel);
    row1->addWidget(m_intervalCombo);
    row1->addSpacing(16);

    auto *shortcutLabel = new QLabel(QStringLiteral("快捷时间："), this);
    shortcutLabel->setObjectName("trendLabel");
    row1->addWidget(shortcutLabel);

    const QList<QPair<QString, int> > shortcuts = {
        {QStringLiteral("最近 1 小时"), 1},
        {QStringLiteral("最近 6 小时"), 6},
        {QStringLiteral("最近 24 小时"), 24},
    };
    for (const auto &[btnText, hours]: shortcuts) {
        auto *btn = new QPushButton(btnText, this);
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

    auto *startLabel = new QLabel(QStringLiteral("开始时间："), this);
    startLabel->setObjectName("trendLabel");

    m_startEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    m_startEdit->setObjectName("TrendDateTimeEdit");
    m_startEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_startEdit->setCalendarPopup(true);
    m_startEdit->setFixedWidth(180);

    auto *endLabel = new QLabel(QStringLiteral("结束时间："), this);
    endLabel->setObjectName("trendLabel");

    m_endEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_endEdit->setObjectName("TrendDateTimeEdit");
    m_endEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_endEdit->setCalendarPopup(true);
    m_endEdit->setFixedWidth(180);

    m_queryBtn = new QPushButton(QStringLiteral("查询"), this);
    m_queryBtn->setObjectName("PrimaryButton");
    m_queryBtn->setFixedSize(80, 32);
    m_queryBtn->setCursor(Qt::PointingHandCursor);
    connect(m_queryBtn, &QPushButton::clicked, this, &TrendControlBar::onQueryClicked);

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

// ── 公开访问器 ────────────────────────────────────────────────────────────────
QString TrendControlBar::interval() const {
    return m_intervalCombo->currentData().toString();
}

QDateTime TrendControlBar::startTime() const {
    return m_startEdit->dateTime();
}

QDateTime TrendControlBar::endTime() const {
    return m_endEdit->dateTime();
}

void TrendControlBar::setLoading(bool loading) const {
    m_queryBtn->setEnabled(!loading);
}

// ── 槽 ───────────────────────────────────────────────────────────────────────
void TrendControlBar::onQueryClicked() {
    emit queryRequested(m_startEdit->dateTime(),
                        m_endEdit->dateTime(),
                        m_intervalCombo->currentData().toString());
}

void TrendControlBar::onShortcutClicked(const int hours) const {
    const QDateTime now = QDateTime::currentDateTime();
    m_endEdit->setDateTime(now);
    m_startEdit->setDateTime(now.addSecs(-static_cast<qint64>(hours) * 3600));

    // 根据时间范围自动选择合适粒度
    QString autoInterval;
    if (hours <= 1) autoInterval = QStringLiteral("1m");
    else if (hours <= 6) autoInterval = QStringLiteral("5m");
    else autoInterval = QStringLiteral("15m");

    for (int i = 0; i < m_intervalCombo->count(); ++i) {
        if (m_intervalCombo->itemData(i).toString() == autoInterval) {
            m_intervalCombo->setCurrentIndex(i);
            break;
        }
    }
}