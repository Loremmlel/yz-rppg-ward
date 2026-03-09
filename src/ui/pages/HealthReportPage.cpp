#include "HealthReportPage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

#include "../../service/ConfigService.h"
#include "../../util/StyleLoader.h"

HealthReportPage::HealthReportPage(QWidget *parent) : QWidget(parent) {
    setObjectName("HealthReportPage");
    initUI();

    auto *svc = HealthReportService::instance();

    connect(m_controlBar, &TimeRangeControlBar::queryRequested,
            this, &HealthReportPage::onQueryRequested);

    connect(svc, &HealthReportService::loadingChanged,
            m_controlBar, &TimeRangeControlBar::setLoading);

    connect(svc, &HealthReportService::loadingChanged,
            this, [this](const bool loading) {
                if (loading) {
                    setStatus(QStringLiteral("⏳ 正在生成报告..."), false);
                }
            });

    connect(svc, &HealthReportService::reportReady,
            this, [this](const QString &html, const QString &) {
                showHtml(html);
                setStatus(QStringLiteral("✅ 报告已生成"), false);
                refreshHistory();
            });

    connect(svc, &HealthReportService::errorOccurred,
            this, [this](const QString &msg) {
                setStatus(msg, true);
            });

    refreshHistory();
    StyleLoader::apply(this, QStringLiteral(":/styles/health_report_page.qss"));
}

void HealthReportPage::initUI() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_controlBar = new TimeRangeControlBar(this);
    layout->addWidget(m_controlBar);

    auto *contentRow = new QWidget(this);
    contentRow->setObjectName("reportContentRow");
    auto *contentRowLayout = new QHBoxLayout(contentRow);
    contentRowLayout->setContentsMargins(10, 0, 10, 10);
    contentRowLayout->setSpacing(10);

    auto *historyPanel = new QFrame(contentRow);
    historyPanel->setObjectName("reportHistoryPanel");
    auto *historyLayout = new QVBoxLayout(historyPanel);
    historyLayout->setContentsMargins(12, 10, 12, 10);
    historyLayout->setSpacing(8);

    auto *historyHeader = new QHBoxLayout();
    auto *title = new QLabel(QStringLiteral("历史报告"), historyPanel);
    title->setObjectName("reportSectionTitle");

    auto *refreshBtn = new QPushButton(QStringLiteral("刷新"), historyPanel);
    refreshBtn->setObjectName("TrendShortcutButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    historyHeader->addWidget(title);
    historyHeader->addStretch();
    historyHeader->addWidget(refreshBtn);

    m_historyList = new QListWidget(historyPanel);
    m_historyList->setObjectName("reportHistoryList");

    historyLayout->addLayout(historyHeader);
    historyLayout->addWidget(m_historyList, 1);

    auto *rightPanel = new QFrame(contentRow);
    rightPanel->setObjectName("reportRightPanel");
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(12, 10, 12, 10);
    rightLayout->setSpacing(8);

    m_statusLabel = new QLabel(rightPanel);
    m_statusLabel->setObjectName("reportStatusLabel");
    m_statusLabel->setText(QStringLiteral("请选择参数并点击查询生成报告"));
    rightLayout->addWidget(m_statusLabel);

    auto *htmlContainer = new QFrame(rightPanel);
    htmlContainer->setObjectName("reportHtmlContainer");
    auto *htmlLayout = new QVBoxLayout(htmlContainer);
    htmlLayout->setContentsMargins(8, 8, 8, 8);

    auto *scrollArea = new QScrollArea(htmlContainer);
    scrollArea->setWidgetResizable(true);
    scrollArea->setObjectName("reportHtmlScroll");

    auto *content = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    m_htmlLabel = new QLabel(content);
    m_htmlLabel->setObjectName("reportHtmlLabel");
    m_htmlLabel->setWordWrap(true);
    m_htmlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    m_htmlLabel->setOpenExternalLinks(true);
    m_htmlLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_htmlLabel->setText(QStringLiteral("<html><body><h3>暂无报告</h3><p>生成后将在此显示。</p></body></html>"));

    contentLayout->addWidget(m_htmlLabel);
    contentLayout->addStretch();

    scrollArea->setWidget(content);
    htmlLayout->addWidget(scrollArea);

    rightLayout->addWidget(htmlContainer, 1);

    contentRowLayout->addWidget(historyPanel, 3);
    contentRowLayout->addWidget(rightPanel, 7);

    layout->addWidget(contentRow, 1);

    connect(refreshBtn, &QPushButton::clicked, this, &HealthReportPage::refreshHistory);
    connect(m_historyList, &QListWidget::currentRowChanged, this, &HealthReportPage::onHistorySelected);
}

void HealthReportPage::onQueryRequested(const QDateTime &start,
                                        const QDateTime &end,
                                        const QString &interval) const {
    const AppConfig cfg = ConfigService::instance()->config();
    if (!cfg.hasBed()) {
        setStatus(QStringLiteral("⚠ 请先在设置页面配置床位信息"), true);
        return;
    }
    if (start >= end) {
        setStatus(QStringLiteral("⚠ 开始时间必须早于结束时间"), true);
        return;
    }
    HealthReportService::instance()->generate(cfg.bedId, start, end, interval);
}

void HealthReportPage::onHistorySelected(const int row) {
    if (row < 0 || row >= m_historyItems.size()) {
        return;
    }

    const QString html = HealthReportService::readHistoryHtml(m_historyItems.at(row).filePath);
    if (html.isEmpty()) {
        setStatus(QStringLiteral("⚠ 读取历史报告失败"), true);
        return;
    }

    showHtml(html);
    setStatus(QStringLiteral("📖 已加载历史报告：") + m_historyItems.at(row).fileName, false);
}

void HealthReportPage::refreshHistory() {
    m_historyItems = HealthReportService::listHistory();

    m_historyList->clear();
    for (const auto &item: m_historyItems) {
        const QString text = QStringLiteral("%1  |  %2")
                .arg(item.generatedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")), item.fileName);
        m_historyList->addItem(text);
    }

    if (m_historyItems.isEmpty()) {
        m_historyList->addItem(QStringLiteral("暂无历史报告（将在可执行目录 report/ 下保存）"));
        m_historyList->setCurrentRow(-1);
    }
}

void HealthReportPage::setStatus(const QString &text, const bool isError) const {
    m_statusLabel->setProperty("error", isError);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    m_statusLabel->setText(text);
}

void HealthReportPage::showHtml(const QString &html) const {
    m_htmlLabel->setTextFormat(Qt::RichText);
    m_htmlLabel->setText(html);
}
