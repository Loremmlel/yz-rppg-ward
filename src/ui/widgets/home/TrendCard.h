#pragma once

#include <QFrame>
#include <QLabel>

/**
 * @brief 历史趋势页面中的紧凑指标卡片
 *
 * 仅展示指标名称与最新聚合数值，不含折线图，体积比 MetricCard 小。
 * 布局：左侧图标 + 名称标签，右侧数值标签。
 */
class TrendCard : public QFrame {
    Q_OBJECT

public:
    /**
     * @param title   指标中文名，如 "心率均值"
     * @param icon    emoji 或资源路径
     * @param unit    单位字符串，如 "bpm"；为空则不显示单位
     * @param accentColor 左侧色带颜色
     * @param parent  父控件
     */
    explicit TrendCard(const QString &title,
                       const QString &icon,
                       const QString &unit,
                       const QColor  &accentColor,
                       QWidget       *parent = nullptr);

    /** @brief 更新显示的数值；传空字符串显示 "--" */
    void setValue(const QString &value) const;

    /** @brief 便捷接口：传入 double 按指定精度格式化，并附加单位 */
    void setValue(double value, int precision = 1) const;

    /** @brief 标记为"无数据"状态 */
    void clearValue() const;

private:
    QLabel   *m_iconLabel{nullptr};
    QLabel   *m_titleLabel{nullptr};
    QLabel   *m_valueLabel{nullptr};

    QString   m_unit;
};

