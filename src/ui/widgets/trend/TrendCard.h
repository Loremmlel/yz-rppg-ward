#pragma once

#include <QLabel>
#include <optional>

#include "TrendChart.h"

/**
 * @brief 历史趋势页面中的指标卡片（含折线图）
 *
 * 布局：
 *  - 顶部行：左侧图标 + 名称，右侧最新聚合数值
 *  - 下部：TrendChart 折线图（含 X 时间轴和参考线）
 */
class TrendCard : public QFrame {
    Q_OBJECT

public:
    /**
     * @param title       指标中文名，如 "心率均值"
     * @param icon        emoji 或资源路径
     * @param unit        单位字符串，如 "bpm"；为空则不显示
     * @param accentColor 左侧色带 & 折线颜色
     * @param parent      父控件
     */
    explicit TrendCard(const QString &title,
                       const QString &icon,
                       const QString &unit,
                       const QColor  &accentColor,
                       QWidget       *parent = nullptr);

    /**
     * @brief 用完整数据列表刷新卡片
     *
     * @param timestamps  每个 bucket 对应的时刻（本地时间）
     * @param points      每个 bucket 的数值，nullopt 表示缺失
     * @param refValue    参考线值（均值/中位数），nullopt 则不绘参考线
     */
    void setData(const QList<QDateTime>             &timestamps,
                 const QList<std::optional<double>> &points,
                 std::optional<double>               refValue = std::nullopt) const;

    /** @brief 清空数据，显示 "--" */
    void clearData() const;

private:
    static QString formatValue(double v, int precision);

    QLabel      *m_iconLabel{nullptr};
    QLabel      *m_titleLabel{nullptr};
    QLabel      *m_valueLabel{nullptr};  ///< 最新（末尾）有效值
    TrendChart  *m_chart{nullptr};

    QString      m_unit;
    QColor       m_accentColor;
};

