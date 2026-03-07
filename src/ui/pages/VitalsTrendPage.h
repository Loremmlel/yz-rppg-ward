#pragma once

#include <QWidget>

#include "../widgets/trend/TrendControlBar.h"
#include "../widgets/trend/TrendMetricsPanel.h"
#include "../../service/VitalsTrendService.h"

/**
 * @brief 患者生命体征历史趋势查询页面
 *
 * 职责：仅组装子控件并连接信号槽，不包含业务逻辑。
 *
 *  ┌──────────────────────────────┐
 *  │  TrendControlBar             │ ← 发出 queryRequested
 *  ├──────────────────────────────┤
 *  │  TrendMetricsPanel           │ ← 接收 resultReady / errorOccurred
 *  │  （可滚动，三组 TrendCard）    │
 *  └──────────────────────────────┘
 *
 * 数据流：
 *   TrendControlBar::queryRequested
 *     → VitalsTrendPage::onQueryRequested（校验床位配置）
 *       → VitalsTrendService::query
 *         → VitalsTrendService::resultReady  → TrendMetricsPanel::applyResult
 *         → VitalsTrendService::errorOccurred → TrendMetricsPanel::setStatus
 *         → VitalsTrendService::loadingChanged → TrendControlBar::setLoading
 *                                              → TrendMetricsPanel::setStatus
 */
class VitalsTrendPage : public QWidget {
    Q_OBJECT

public:
    explicit VitalsTrendPage(QWidget *parent = nullptr);

private slots:
    void onQueryRequested(const QDateTime &start,
                          const QDateTime &end,
                          const QString   &interval) const;

private:
    TrendControlBar  *m_controlBar{nullptr};
    TrendMetricsPanel *m_metricsPanel{nullptr};
};

