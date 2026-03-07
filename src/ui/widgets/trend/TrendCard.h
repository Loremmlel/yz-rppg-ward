#pragma once

#include <QFrame>
#include <QLabel>
#include <QList>
#include <QDateTime>
#include <optional>

#include "TrendChart.h"

/**
 * @brief 历史趋势页面中的指标卡片（含折线图）
 *
 * 布局：
 *  - 顶部行：名称 + ⓘ 提示按钮，右侧最新聚合数值
 *  - 下部：TrendChart 折线图（含 X 时间轴和参考线）
 *
 * 点击 ⓘ 弹出 AppDialog 显示该指标的中文名、含义、正常范围及异常含义。
 */
class TrendCard : public QFrame {
    Q_OBJECT

public:
    /** 指标解释信息（用于 ⓘ 弹窗） */
    struct MetricInfo {
        QString name;        ///< 中文全称
        QString meaning;     ///< 生理含义
        QString normalRange; ///< 正常数值范围
        QString abnormal;    ///< 异常时可能代表什么
    };

    /**
     * @param title       简短名称（显示在卡片上）
     * @param info        详细解释信息（点击 ⓘ 弹出）
     * @param unit        单位字符串，如 "bpm"；为空则不显示
     * @param accentColor 左侧色带 & 折线颜色
     * @param parent      父控件
     */
    explicit TrendCard(const QString    &title,
                       const MetricInfo &info,
                       const QString    &unit,
                       const QColor     &accentColor,
                       QWidget          *parent = nullptr);

    /**
     * @brief 用完整数据列表刷新卡片
     */
    void setData(const QList<QDateTime>             &timestamps,
                 const QList<std::optional<double>> &points,
                 std::optional<double>               refValue = std::nullopt) const;

    /** @brief 清空数据，显示 "--" */
    void clearData() const;

private:
    static QString formatValue(double v, int precision);
    static QString buildDialogContent(const MetricInfo &info);

    QLabel     *m_titleLabel{nullptr};
    QLabel     *m_valueLabel{nullptr};
    TrendChart *m_chart{nullptr};

    QString   m_unit;
    QColor    m_accentColor;
};

