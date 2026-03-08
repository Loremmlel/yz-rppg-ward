#pragma once

#include <QFrame>
#include <QLabel>
#include <optional>

#include "MetricChart.h"

/**
 * @brief 单项监测指标可视化卡片
 *
 * 布局分为上下两部分：
 *  - 上部：左侧图标 + 指标名，右侧大字数值；可选居中警告标签
 *  - 下部：近 60 秒趋势折线图（MetricChart）
 */
class MetricCard : public QFrame {
    Q_OBJECT

public:
    explicit MetricCard(const QString &title, const QString &icon,
                        QColor chartColor,
                        MetricChart::AxisMode axisMode = MetricChart::AxisMode::ElasticFrom100,
                        bool showLowQualityWarning = false,
                        QWidget *parent = nullptr);

    void setValue(const QString &value) const;

    void setIcon(const QString &iconStr) const;

    /**
     * @brief 向趋势图追加一个数据点
     * @param value      数值（nullopt 表示缺失，折线断开）
     * @param lowQuality 该时刻信号质量不佳，折线下方填充红色渐变
     */
    void addDataPoint(std::optional<double> value, bool lowQuality = false) const;

    /**
     * @brief 设置"信号质量不佳"警告标签的显示状态
     *
     * 仅在 showLowQualityWarning=true 时有效。
     * 图表着色已通过 addDataPoint 的 lowQuality 参数逐点控制，无需此接口操作图表。
     */
    void setLowQualityWarning(bool show) const;

private:
    QLabel *m_iconLabel{nullptr};
    QLabel *m_titleLabel{nullptr};
    QLabel *m_valueLabel{nullptr};
    QLabel *m_warningLabel{nullptr}; ///< "信号质量不佳" 警告（可为 nullptr）

    MetricChart *m_chart{nullptr};

    bool m_showWarning{false};
};