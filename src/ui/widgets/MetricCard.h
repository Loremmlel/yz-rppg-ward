#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief 指标卡片组件
 * 用于显示单一健康指标，包含图标、标题和数值。
 */
class MetricCard : public QFrame {
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param title 指标标题
     * @param icon 图标（Emoji或图片路径，如 ":/icons/hr.svg"）
     * @param parent 父组件
     */
    explicit MetricCard(const QString& title, const QString& icon, QWidget *parent = nullptr);

    /**
     * @brief 设置显示数值
     * @param value 数值字符串
     */
    void setValue(const QString& value);

    /**
     * @brief 设置图标内容
     * @param iconStr 图标字符串，可以是 Emoji 也可以是图片路径 (PNG, SVG, etc.)
     */
    void setIcon(const QString& iconStr);

private:
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
};
