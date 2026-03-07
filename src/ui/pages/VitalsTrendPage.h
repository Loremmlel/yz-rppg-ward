#pragma once

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QVBoxLayout>
#include <QList>

#include "../../model/VitalsTrendData.h"
#include "../widgets/home/TrendCard.h"

/**
 * @brief 患者生命体征历史趋势查询页面
 *
 * 布局（QVBoxLayout）：
 *  1. 顶部控制面板：时间粒度下拉框 + 快捷时间按钮组 + 自定义时间选择器 + 查询按钮
 *  2. 中间指标区域（可滚动）：
 *     - 基础生命体征（hrAvg / brAvg / sqiAvg）
 *     - HRV 时域中位数（sdnn / rmssd / sdsd / pnn50 / pnn20）
 *     - HRV 频域均值（lfHfRatio / hf / lf / vlf / tp）
 */
class VitalsTrendPage : public QWidget {
    Q_OBJECT

public:
    explicit VitalsTrendPage(QWidget *parent = nullptr);

private slots:
    void onQueryClicked();
    void onShortcutClicked(int hours) const;

private:
    void initUI();
    void setupControlPanel();
    void setupMetricsArea();
    void buildGroupSection(QVBoxLayout *layout,
                           const QString &groupTitle,
                           const QList<TrendCard *> &cards);

    void fetchTrend(const QDateTime &start, const QDateTime &end, const QString &interval) const;
    void applyData(const QList<VitalsTrendData> &records) const;
    void setLoading(bool loading) const;
    void showError(const QString &message) const;

    // ── 控制面板控件 ──
    QComboBox      *m_intervalCombo{nullptr};
    QDateTimeEdit  *m_startEdit{nullptr};
    QDateTimeEdit  *m_endEdit{nullptr};
    QPushButton    *m_queryBtn{nullptr};
    QLabel         *m_statusLabel{nullptr};

    // ── 基础生命体征卡片 ──
    TrendCard *m_cardHrAvg{nullptr};
    TrendCard *m_cardBrAvg{nullptr};
    TrendCard *m_cardSqiAvg{nullptr};

    // ── HRV 时域卡片 ──
    TrendCard *m_cardSdnn{nullptr};
    TrendCard *m_cardRmssd{nullptr};
    TrendCard *m_cardSdsd{nullptr};
    TrendCard *m_cardPnn50{nullptr};
    TrendCard *m_cardPnn20{nullptr};

    // ── HRV 频域卡片 ──
    TrendCard *m_cardLfHfRatio{nullptr};
    TrendCard *m_cardHf{nullptr};
    TrendCard *m_cardLf{nullptr};
    TrendCard *m_cardVlf{nullptr};
    TrendCard *m_cardTp{nullptr};
};


