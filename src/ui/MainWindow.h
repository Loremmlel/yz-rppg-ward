#pragma once
#include <QMainWindow>
#include <QSplitter>

#include "widgets/MetricsWidget.h"
#include "widgets/VideoWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    QSplitter* m_mainSplitter;
    VideoWidget* m_videoWidget;
    MetricsWidget* m_metricsWidget;
};