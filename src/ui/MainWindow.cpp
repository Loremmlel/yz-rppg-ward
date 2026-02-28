#include "MainWindow.h"
#include "../util/StyleLoader.h"
#include "widgets/ToastManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("病房端监控终端"));
    resize(1280, 720);
    initUI();

    // Toast 管理器绑定到主窗口
    ToastManager::instance()->setParentWidget(this);

    // 全局样式 + 导航栏样式
    setStyleSheet(StyleLoader::loadMultiple({
        QStringLiteral(":/styles/global.qss"),
        QStringLiteral(":/styles/main_window.qss")
    }));
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

    m_homePage     = new HomePage(m_stackedWidget);
    m_settingsPage = new SettingsPage(m_stackedWidget);

    m_stackedWidget->addWidget(m_homePage);     // index 0
    m_stackedWidget->addWidget(m_settingsPage); // index 1

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

    auto *homeBtn     = createNavButton(QStringLiteral("主页"));
    auto *settingsBtn = createNavButton(QStringLiteral("设置"));

    m_navGroup->addButton(homeBtn, 0);
    m_navGroup->addButton(settingsBtn, 1);

    navLayout->addWidget(homeBtn);
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
