#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>

#include "pages/HomePage.h"
#include "pages/SettingsPage.h"
#include "widgets/StatusBar.h"

/**
 * @brief 应用主窗口
 *
 * 职责：
 *  - 顶部导航栏（选项卡按钮）
 *  - StatusBar 显示常驻通知（所有页面可见）
 *  - QStackedWidget 管理页面切换
 *  - 仅管理页面，不直接操作子 Widget
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    [[nodiscard]] VitalsWidget *getVitalsWidget() const { return m_homePage->getVitalsWidget(); }
    [[nodiscard]] VideoWidget *getVideoWidget() const { return m_homePage->getVideoWidget(); }
    [[nodiscard]] StatusBar *notificationBar() const { return m_statusBar; }

    [[nodiscard]] HomePage *homePage() const { return m_homePage; }
    [[nodiscard]] SettingsPage *settingsPage() const { return m_settingsPage; }

private:
    void initUI();

    void setupNavBar();

    QPushButton *createNavButton(const QString &text);

    QStackedWidget *m_stackedWidget = nullptr;
    QButtonGroup *m_navGroup = nullptr;

    StatusBar *m_statusBar = nullptr;
    HomePage *m_homePage = nullptr;
    SettingsPage *m_settingsPage = nullptr;
};
