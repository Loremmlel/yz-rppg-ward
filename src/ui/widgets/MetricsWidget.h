#pragma once

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "MetricCard.h"

class MetricsWidget : public QWidget {
    Q_OBJECT
public:
    explicit MetricsWidget(QWidget *parent = nullptr);

private slots:
    void updateMetrics();

private:
    MetricCard* m_cardHR;
    MetricCard* m_cardSpO2;
    MetricCard* m_cardRR;

    QVBoxLayout* m_listLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_container;
    QTimer* m_timer;
};