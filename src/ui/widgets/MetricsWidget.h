#pragma once

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include "MetricCard.h"

class MetricsWidget : public QWidget {
    Q_OBJECT
public:
    explicit MetricsWidget(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateMetrics();

private:
    void rearrangeLayout();

    MetricCard* m_cardHR;
    MetricCard* m_cardSpO2;
    MetricCard* m_cardRR;

    QGridLayout* m_gridLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_container;
    QTimer* m_timer;
};