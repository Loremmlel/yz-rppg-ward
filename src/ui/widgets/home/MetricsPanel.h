#pragma once

#include <QVBoxLayout>
#include <QMap>
#include <QColor>

#include "MetricCard.h"
#include "../../../model/MetricsData.h"

/**
 * @brief 监测指标面板
 *
 * 管理一组 MetricCard，通过 updateData 槽接收 MetricsService 分发的数据快照。
 * 当前显示 HR（心率）与 SQI（信号质量指数）两张卡片，纵向均分填满面板。
 */
class MetricsPanel : public QWidget {
    Q_OBJECT

public:
    explicit MetricsPanel(QWidget *parent = nullptr);

public slots:
    void updateData(const MetricsData &data);

private:
    void addMetricCard(const QString &key, const QString &title, const QString &icon,
                       QColor chartColor,
                       MetricChart::AxisMode axisMode = MetricChart::AxisMode::ElasticFrom100,
                       bool showLowQualityWarning = false);

    QVBoxLayout *m_listLayout{nullptr};
    QMap<QString, MetricCard *> m_cards;

    bool m_lastLowQuality{false}; ///< 上一帧 SQI 低质量状态，避免重复触发警告标签更新
};
