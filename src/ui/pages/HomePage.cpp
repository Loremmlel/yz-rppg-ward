#include "HomePage.h"
#include "../../util/StyleLoader.h"

HomePage::HomePage(QWidget *parent) : QWidget(parent) {
    initUI();
    StyleLoader::apply(this, QStringLiteral(":/styles/home.qss"));
}

void HomePage::initUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    m_videoWidget = new VideoWidget(m_mainSplitter);
    m_metricsPanel = new MetricsPanel(m_mainSplitter);

    m_mainSplitter->addWidget(m_videoWidget);
    m_mainSplitter->addWidget(m_metricsPanel);

    // 初始等分；用户可拖动分隔线，setMinimumWidth 防止任一侧被压到无法交互
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 1);
    m_videoWidget->setMinimumWidth(400);
    m_metricsPanel->setMinimumWidth(300);

    mainLayout->addWidget(m_mainSplitter);
}
