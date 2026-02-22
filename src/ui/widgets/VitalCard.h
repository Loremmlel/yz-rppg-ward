#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief 健康指标可视化卡片
 * 该组件采用水平布局，左侧展示图标与指标名称，右侧显著展示实时采集的数值。
 */
class VitalCard : public QFrame {
    Q_OBJECT
public:
    /**
     * @brief 构建一个带标题指引与视觉图标的卡片
     * @param title 指标的可读名称（如“心率”）
     * @param icon 图标路径或字符标识
     * @param parent 挂载的父容器
     */
    explicit VitalCard(const QString& title, const QString& icon, QWidget *parent = nullptr);

    /**
     * @brief 更新卡片内显示的数值文本
     * @param value 待显示的测量数值
     */
    void setValue(const QString& value);

    /**
     * @brief 设置图标内容
     * @param iconStr 图标字符串，可以是 Emoji 也可以是图片路径 (PNG, SVG, etc.)
     */
    void setIcon(const QString& iconStr);

private:
    /**
     * @brief 指标状态图标容器
     */
    QLabel* m_iconLabel;
    /**
     * @brief 指标名称文本容器
     */
    QLabel* m_titleLabel;
    /**
     * @brief 测量结果主数值容器
     */
    QLabel* m_valueLabel;
};
