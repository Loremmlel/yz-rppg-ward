#pragma once

#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFrame>
#include <QLabel>
#include <QPushButton>

/**
 * @brief 通用时间范围查询栏
 *
 * 包含：时间粒度下拉框、快捷时间按钮、自定义时间范围、查询按钮。
 * 两个页面（历史趋势、健康报告）可复用同一套交互与参数定义。
 */
class TimeRangeControlBar : public QFrame {
    Q_OBJECT

public:
    explicit TimeRangeControlBar(QWidget *parent = nullptr);

    [[nodiscard]] QString interval() const;
    [[nodiscard]] QDateTime startTime() const;
    [[nodiscard]] QDateTime endTime() const;

    void setLoading(bool loading) const;

signals:
    void queryRequested(const QDateTime &startTime,
                        const QDateTime &endTime,
                        const QString &interval);

private slots:
    void onQueryClicked();
    void onShortcutClicked(int hours) const;

private:
    void initUI();

    QComboBox *m_intervalCombo{nullptr};
    QDateTimeEdit *m_startEdit{nullptr};
    QDateTimeEdit *m_endEdit{nullptr};
    QPushButton *m_queryBtn{nullptr};
};

