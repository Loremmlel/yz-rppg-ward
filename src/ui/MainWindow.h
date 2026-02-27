#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>

#include "pages/HomePage.h"
#include "pages/SettingsPage.h"

/**
 * @brief 应用主窗口
 *
 * 职责：
 *  - 顶部导航栏（选项卡按钮）
 *  - QStackedWidget 管理页面切换
 *  - 仅管理页面，不直接操作子 Widget
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    [[nodiscard]] VitalsWidget *getVitalsWidget() const { return m_homePage->getVitalsWidget(); }
    [[nodiscard]] VideoWidget  *getVideoWidget()  const { return m_homePage->getVideoWidget(); }

    /**
     * @brief 获取主页指针（供 Controller 连接信号）
     */
    [[nodiscard]] HomePage     *homePage()     const { return m_homePage; }

    /**
     * @brief 获取设置页指针
     */
    [[nodiscard]] SettingsPage *settingsPage() const { return m_settingsPage; }

private:
    void initUI();
    void setupNavBar();

    /**
     * @brief 创建一个导航按钮
     */
    QPushButton *createNavButton(const QString &text);

    QStackedWidget *m_stackedWidget = nullptr;
    QButtonGroup   *m_navGroup      = nullptr;

    HomePage       *m_homePage      = nullptr;
    SettingsPage   *m_settingsPage  = nullptr;
};
