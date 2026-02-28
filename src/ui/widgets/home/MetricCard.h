#pragma once

#include <QFrame>
#include <QLabel>

/**
 * @brief 单项监测指标可视化卡片
 *
 * 布局分为上下两部分：
 *  - 上部：左侧图标 + 指标名，右侧大字数值
 *  - 下部：预留趋势折线图区域（占据剩余高度）
 *
 * 图标支持 qrc 路径（PNG/SVG）和纯文本字符串（Emoji 等），由 setIcon 自动判别。
 */
class MetricCard : public QFrame {
    Q_OBJECT

public:
    explicit MetricCard(const QString &title, const QString &icon, QWidget *parent = nullptr);

    void setValue(const QString &value) const;
    void setIcon(const QString &iconStr) const;

private:
    QLabel *m_iconLabel{nullptr};
    QLabel *m_titleLabel{nullptr};
    QLabel *m_valueLabel{nullptr};
    QWidget *m_trendArea{nullptr};  ///< 预留趋势图区域
};

