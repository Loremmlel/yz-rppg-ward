#pragma once

#include <QLabel>

/**
 * @brief 单项生命体征可视化卡片
 *
 * 固定布局：左侧图标 + 指标名，右侧大字数值。
 * 图标支持 qrc 路径（PNG/SVG）和纯文本字符串（Emoji 等），由 setIcon 自动判别。
 */
class VitalCard : public QFrame {
    Q_OBJECT

public:
    explicit VitalCard(const QString &title, const QString &icon, QWidget *parent = nullptr);

    void setValue(const QString &value) const;
    void setIcon(const QString &iconStr) const;

private:
    QLabel *m_iconLabel  {nullptr};
    QLabel *m_titleLabel {nullptr};
    QLabel *m_valueLabel {nullptr};
};
