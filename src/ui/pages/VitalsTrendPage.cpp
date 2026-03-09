#include "VitalsTrendPage.h"

#include <QVBoxLayout>

#include "../../service/ConfigService.h"

VitalsTrendPage::VitalsTrendPage(QWidget *parent) : QWidget(parent) {
    setObjectName("VitalsTrendPage");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_controlBar = new TrendControlBar(this);
    m_metricsPanel = new TrendMetricsPanel(this);

    layout->addWidget(m_controlBar);
    layout->addWidget(m_metricsPanel, 1);

    auto *svc = VitalsTrendService::instance();

    // 控制栏发出查询请求 → 本页校验配置后转发给 Service
    connect(m_controlBar, &TrendControlBar::queryRequested,
            this, &VitalsTrendPage::onQueryRequested);

    // Service 结果 → 面板更新
    connect(svc, &VitalsTrendService::resultReady,
            m_metricsPanel, &TrendMetricsPanel::applyResult);

    // Service 错误 → 面板显示错误
    connect(svc, &VitalsTrendService::errorOccurred,
            m_metricsPanel, [this](const QString &msg) {
                m_metricsPanel->setStatus(msg, true);
            });

    // Service 加载状态 → 控制栏禁用按钮 + 面板显示提示
    connect(svc, &VitalsTrendService::loadingChanged,
            m_controlBar, &TrendControlBar::setLoading);
    connect(svc, &VitalsTrendService::loadingChanged,
            m_metricsPanel, [this](const bool loading) {
                if (loading)
                    m_metricsPanel->setStatus(QStringLiteral("⏳ 正在查询…"), false);
                else
                    m_metricsPanel->setStatus(QString{}, false); // 清除加载提示
            });
}

void VitalsTrendPage::onQueryRequested(const QDateTime &start,
                                       const QDateTime &end,
                                       const QString &interval) const {
    const AppConfig cfg = ConfigService::instance()->config();
    if (!cfg.hasBed()) {
        m_metricsPanel->setStatus(QStringLiteral("⚠ 请先在设置页面配置床位信息"), true);
        return;
    }
    if (start >= end) {
        m_metricsPanel->setStatus(QStringLiteral("⚠ 开始时间必须早于结束时间"), true);
        return;
    }
    VitalsTrendService::instance()->query(cfg.bedId, start, end, interval);
}