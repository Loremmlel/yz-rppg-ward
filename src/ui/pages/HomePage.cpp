#include "HomePage.h"

HomePage::HomePage(QWidget *parent) : QWidget(parent) {
    initUI();
}

void HomePage::initUI() {
    // 设置主布局（虽然 Splitter 自己也是布局管理者，但放在 QWidget 中最好有一个 Layout）
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 利用 QSplitter 构建左右可伸缩布局，默认视频居左，指标居右
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    m_videoWidget = new VideoWidget(m_mainSplitter);
    m_vitalsWidget = new VitalsWidget(m_mainSplitter);

    m_mainSplitter->addWidget(m_videoWidget);
    m_mainSplitter->addWidget(m_vitalsWidget);

    // 设置初始拉伸因子，使两者平分可用空间
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 1);

    // 限制各组件的最小展示宽度，保证必要的交互体验
    m_videoWidget->setMinimumWidth(400);
    m_vitalsWidget->setMinimumWidth(300);

    mainLayout->addWidget(m_mainSplitter);
}

