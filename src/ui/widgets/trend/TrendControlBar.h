#pragma once

#include <QFrame>
#include <QComboBox>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QLabel>
#include <QDateTime>
#include "../common/TimeRangeControlBar.h"

/**
 * @brief 历史趋势页顶部控制栏（兼容层）
 *
 * 实际实现复用通用 TimeRangeControlBar。
 */
class TrendControlBar : public TimeRangeControlBar {
    Q_OBJECT

public:
    explicit TrendControlBar(QWidget *parent = nullptr): TimeRangeControlBar(parent) {};
};