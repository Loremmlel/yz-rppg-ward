#include "MainWindow.h"
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle(QStringLiteral("病房端监控终端"));
    this->resize(1280, 720);

    initUI();
    initConnections();
}

void MainWindow::initUI() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    auto *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_topBar = new QFrame(m_centralWidget);
    m_topBar->setObjectName("topBar");
    m_topBar->setFixedHeight(50);

    auto *topBarLayout = new QHBoxLayout(m_topBar);
    topBarLayout->setContentsMargins(10, 0, 10, 0);
    topBarLayout->setSpacing(0);
    topBarLayout->setAlignment(Qt::AlignBottom);

    m_homeBtn = new QPushButton(QStringLiteral("主页"), m_topBar);
    m_homeBtn->setObjectName("homeNavButton");
    m_homeBtn->setProperty("navButton", true);
    m_homeBtn->setCheckable(true);
    m_homeBtn->setChecked(true);
    m_homeBtn->setCursor(Qt::PointingHandCursor);
    m_homeBtn->setFixedSize(120, 50);

    m_settingsBtn = new QPushButton(QStringLiteral("设置"), m_topBar);
    m_settingsBtn->setObjectName("settingsNavButton");
    m_settingsBtn->setProperty("navButton", true);
    m_settingsBtn->setCheckable(true);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setFixedSize(120, 50);

    // QButtonGroup 保证导航按钮互斥，idClicked 传递的索引与 QStackedWidget 页面索引一一对应
    auto *navGroup = new QButtonGroup(this);
    navGroup->addButton(m_homeBtn, 0);
    navGroup->addButton(m_settingsBtn, 1);
    connect(navGroup, &QButtonGroup::idClicked, this, &MainWindow::onTabChanged);

    topBarLayout->addWidget(m_homeBtn);
    topBarLayout->addStretch();
    topBarLayout->addWidget(m_settingsBtn);

    mainLayout->addWidget(m_topBar);

    m_stackedWidget = new QStackedWidget(m_centralWidget);
    mainLayout->addWidget(m_stackedWidget);

    m_homePage = new HomePage();
    m_stackedWidget->addWidget(m_homePage);

    m_settingsPage = new SettingsPage();
    m_stackedWidget->addWidget(m_settingsPage);
}

void MainWindow::initConnections() {
}

void MainWindow::onTabChanged(const int index) const {
    m_stackedWidget->setCurrentIndex(index);
}
