#pragma once

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "MetricCard.h"

/**
 * @brief 指标监控看板
 * 维护并管理一组健康指标卡片（MetricCard），并通过定时器模拟/更新实时数据状态。
 */
class MetricsWidget : public QWidget {
    Q_OBJECT
public:
    explicit MetricsWidget(QWidget *parent = nullptr);

private slots:
    /**
     * @brief 定时更新槽函数，用于刷新看板内所有卡片的数值
     */
    void updateMetrics();

private:
    /**
     * @name 核心指标卡片
     * @{
     */
    MetricCard* m_cardHR;    ///< 心率监测卡片
    MetricCard* m_cardSpO2;  ///< 血氧饱和度监测卡片
    MetricCard* m_cardRR;    ///< 呼吸频率监测卡片
    /** @} */

    QVBoxLayout* m_listLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_container;
    /**
     * @brief 数据刷新定时器，驱动 UI 实时变动
     */
    QTimer* m_timer;
};