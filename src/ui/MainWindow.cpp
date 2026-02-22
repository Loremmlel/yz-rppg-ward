#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle(QStringLiteral("病房端"));
    this->resize(1280, 720);

    // 核心：使用 QSplitter 实现左右分栏
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 实例化左侧和右侧组件
    m_videoWidget = new VideoWidget(m_mainSplitter);
    m_metricsWidget = new MetricsWidget(m_mainSplitter);

    // 将组件加入 Splitter
    m_mainSplitter->addWidget(m_videoWidget);
    m_mainSplitter->addWidget(m_metricsWidget);

    // 关键配置：设置各占 50% 的比例
    // setStretchFactor(index, stretch)
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 1);

    // 如果不希望用户把某一边完全拖没，可以设置最小宽度
    m_videoWidget->setMinimumWidth(400);
    m_metricsWidget->setMinimumWidth(300);

    // 将 Splitter 设置为主窗口的中心部件
    this->setCentralWidget(m_mainSplitter);
}
