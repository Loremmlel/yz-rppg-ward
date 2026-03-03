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
                        bool enableLowQualityFill = false,
                        double lowQualityThreshold = 0.5,
                        bool showLowQualityWarning = false,
                        QWidget *parent = nullptr);

    void setValue(const QString &value) const;
    void setIcon(const QString &iconStr) const;

    /** 向趋势图追加一个数据点；nullopt 表示数据缺失（折线断开） */
    void addDataPoint(std::optional<double> value) const;

    /**
     * @brief 设置"信号质量不佳"状态
     *
     * - 触发图表红色渐变叠加
     * - 若 showLowQualityWarning=true，在卡片中央显示/隐藏警告文字
     */
    void setLowQuality(bool low) const;

private:
    QLabel      *m_iconLabel{nullptr};
    QLabel      *m_titleLabel{nullptr};
    QLabel      *m_valueLabel{nullptr};
    QLabel      *m_warningLabel{nullptr};  ///< "信号质量不佳" 警告（可为 nullptr）
    MetricChart *m_chart{nullptr};

    bool m_showWarning{false};
};
