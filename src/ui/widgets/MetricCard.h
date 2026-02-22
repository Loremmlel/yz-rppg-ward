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
     * @param emoji 图标（Emoji）
     * @param parent 父组件
     */
    explicit MetricCard(const QString& title, const QString& emoji, QWidget *parent = nullptr);

    /**
     * @brief 设置显示数值
     * @param value 数值字符串
     */
    void setValue(const QString& value);

private:
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
};
