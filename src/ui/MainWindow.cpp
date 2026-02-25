#include "MainWindow.h"
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle(QStringLiteral("病房端监控终端"));
    this->resize(1280, 720);

    initUI();
    initConnections();
}

void MainWindow::initUI() {
    // 1. 创建中心部件和主布局
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    auto *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 2. 创建顶部导航栏
    m_topBar = new QFrame(m_centralWidget);
    m_topBar->setObjectName("TopBar");
    m_topBar->setFixedHeight(50); // 设置固定高度

    auto *topBarLayout = new QHBoxLayout(m_topBar);
    topBarLayout->setContentsMargins(10, 0, 10, 0);
    topBarLayout->setSpacing(0);
    topBarLayout->setAlignment(Qt::AlignBottom);

    // 创建导航按钮
    m_homeBtn = new QPushButton(QStringLiteral("主页"), m_topBar);
    m_homeBtn->setObjectName("NavButton");
    m_homeBtn->setCheckable(true);
    m_homeBtn->setChecked(true); // 默认选中
    m_homeBtn->setCursor(Qt::PointingHandCursor);
    m_homeBtn->setFixedSize(120, 50);

    m_settingsBtn = new QPushButton(QStringLiteral("设置"), m_topBar);
    m_settingsBtn->setObjectName("NavButton");
    m_settingsBtn->setCheckable(true);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setFixedSize(120, 50);

    // 使用按钮组管理互斥状态
    auto *navGroup = new QButtonGroup(this);
    navGroup->addButton(m_homeBtn, 0);
    navGroup->addButton(m_settingsBtn, 1);
    // 连接按钮组信号
    connect(navGroup, &QButtonGroup::idClicked, this, &MainWindow::onTabChanged);

    // 布局：主页按钮 - 弹簧 - 设置按钮
    topBarLayout->addWidget(m_homeBtn);
    topBarLayout->addStretch();
    topBarLayout->addWidget(m_settingsBtn);

    mainLayout->addWidget(m_topBar);

    // 3. 创建堆叠窗口
    m_stackedWidget = new QStackedWidget(m_centralWidget);
    mainLayout->addWidget(m_stackedWidget);

    // --- 页面1：主页 ---
    m_homePage = new HomePage();
    m_stackedWidget->addWidget(m_homePage);

    // --- 页面2：设置 ---
    m_settingsPage = new SettingsPage();
    m_stackedWidget->addWidget(m_settingsPage);
}

void MainWindow::initConnections() {
    // 可以在这里添加其他连接
}

void MainWindow::onTabChanged(const int index) const {
    m_stackedWidget->setCurrentIndex(index);
}
