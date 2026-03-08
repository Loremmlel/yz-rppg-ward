#pragma once

#include <QVBoxLayout>

#include "TrendCard.h"
#include "../../../service/VitalsTrendService.h"

/**
 * @brief 趋势页可滚动指标面板
 *
 * 内部按三组 QGroupBox（基础生命体征 / HRV 时域 / HRV 频域）排布 13 张 TrendCard。
 * 通过 applyResult 槽接收 VitalsTrendService 的结果并更新各卡片。
 * 通过 setStatus 显示加载中或错误信息。
 */
class TrendMetricsPanel : public QWidget {
    Q_OBJECT

public:
    explicit TrendMetricsPanel(QWidget *parent = nullptr);

public slots:
    /** 将一次查询结果分发到各卡片 */
    void applyResult(const VitalsTrendService::TrendResult &result) const;

    /** 显示状态信息（加载中 / 错误 / 清除） */
    void setStatus(const QString &message, bool isError = false) const;

    /** 清空所有卡片数据 */
    void clearAll() const;

private:
    static void buildGroup(QVBoxLayout *layout,
                          const QString &title,
                          const QList<TrendCard *> &cards);

    // 基础生命体征
    TrendCard *m_cardHrAvg{nullptr};
    TrendCard *m_cardBrAvg{nullptr};
    TrendCard *m_cardSqiAvg{nullptr};
    // HRV 时域
    TrendCard *m_cardSdnn{nullptr};
    TrendCard *m_cardRmssd{nullptr};
    TrendCard *m_cardSdsd{nullptr};
    TrendCard *m_cardPnn50{nullptr};
    TrendCard *m_cardPnn20{nullptr};
    // HRV 频域
    TrendCard *m_cardLfHfRatio{nullptr};
    TrendCard *m_cardHf{nullptr};
    TrendCard *m_cardLf{nullptr};
    TrendCard *m_cardVlf{nullptr};
    TrendCard *m_cardTp{nullptr};

    QLabel *m_statusLabel{nullptr};
};



