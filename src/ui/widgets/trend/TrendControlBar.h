#pragma once

#include <QFrame>
#include <QComboBox>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QLabel>
#include <QDateTime>

/**
 * @brief 趋势页顶部控制栏
 *
 * 包含：时间粒度下拉框、快捷时间按钮组、自定义时间范围选择器、查询按钮。
 * 用户点击查询时发出 queryRequested 信号，由 Page 层连接到 Service。
 */
class TrendControlBar : public QFrame {
    Q_OBJECT

public:
    explicit TrendControlBar(QWidget *parent = nullptr);

    /** 当前选中的时间粒度 API 值，如 "5m"、"1h" */
    [[nodiscard]] QString interval() const;

    /** 当前选中的开始时间（本地时间） */
    [[nodiscard]] QDateTime startTime() const;

    /** 当前选中的结束时间（本地时间） */
    [[nodiscard]] QDateTime endTime() const;

    /** 设置加载中状态（禁用/启用查询按钮） */
    void setLoading(bool loading) const;

signals:
    /**
     * @brief 用户请求查询
     * @param startTime 开始时间（本地时间）
     * @param endTime   结束时间（本地时间）
     * @param interval  时间粒度，如 "5m"
     */
    void queryRequested(const QDateTime &startTime,
                        const QDateTime &endTime,
                        const QString   &interval);

private slots:
    void onQueryClicked();
    void onShortcutClicked(int hours) const;

private:
    void initUI();

    QComboBox     *m_intervalCombo{nullptr};
    QDateTimeEdit *m_startEdit{nullptr};
    QDateTimeEdit *m_endEdit{nullptr};
    QPushButton   *m_queryBtn{nullptr};
};

