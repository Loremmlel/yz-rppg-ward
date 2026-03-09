#include "MainWindow.h"
#include "widgets/ToastManager.h"
#include "widgets/AppDialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("病房端监控终端"));
    resize(1280, 720);
    initUI();

    // Toast 管理器绑定到主窗口
    ToastManager::instance()->setParentWidget(this);
    // 全局对话框绑定到主窗口
    AppDialog::instance()->setParentWidget(this);
}

void MainWindow::initUI() {
    auto *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    auto *rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ——— 页面堆叠区域（先创建，供导航栏连接） ———
    m_stackedWidget = new QStackedWidget(centralWidget);

    m_homePage = new HomePage(m_stackedWidget);
    m_settingsPage = new SettingsPage(m_stackedWidget);
    m_vitalsTrendPage = new VitalsTrendPage(m_stackedWidget);
    m_healthReportPage = new HealthReportPage(m_stackedWidget);

    m_stackedWidget->addWidget(m_homePage); // index 0
    m_stackedWidget->addWidget(m_settingsPage); // index 1
    m_stackedWidget->addWidget(m_vitalsTrendPage); // index 2
    m_stackedWidget->addWidget(m_healthReportPage); // index 3

    // ——— 顶部导航栏 ———
    setupNavBar();
    rootLayout->addWidget(findChild<QFrame *>("topBar"));

    // ——— 状态栏（导航栏下方，所有页面可见） ———
    m_statusBar = new StatusBar(centralWidget);
    rootLayout->addWidget(m_statusBar);

    rootLayout->addWidget(m_stackedWidget, 1);
}

void MainWindow::setupNavBar() {
    auto *topBar = new QFrame(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(40);

    auto *navLayout = new QHBoxLayout(topBar);
    navLayout->setContentsMargins(8, 0, 8, 0);
    navLayout->setSpacing(4);

    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);

    auto *homeBtn = createNavButton(QStringLiteral("主页"));
    auto *trendBtn = createNavButton(QStringLiteral("历史趋势"));
    auto *reportBtn = createNavButton(QStringLiteral("健康报告"));
    auto *settingsBtn = createNavButton(QStringLiteral("设置"));

    m_navGroup->addButton(homeBtn, 0);
    m_navGroup->addButton(trendBtn, 2);
    m_navGroup->addButton(reportBtn, 3);
    m_navGroup->addButton(settingsBtn, 1);

    navLayout->addWidget(homeBtn);
    navLayout->addWidget(trendBtn);
    navLayout->addWidget(reportBtn);
    navLayout->addWidget(settingsBtn);
    navLayout->addStretch();

    // 默认选中主页
    homeBtn->setChecked(true);

    // 导航按钮切换页面
    connect(m_navGroup, &QButtonGroup::idClicked,
            m_stackedWidget, &QStackedWidget::setCurrentIndex);
}

QPushButton *MainWindow::createNavButton(const QString &text) {
    auto *btn = new QPushButton(text, this);
    btn->setProperty("navButton", true);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(36);
    return btn;
}