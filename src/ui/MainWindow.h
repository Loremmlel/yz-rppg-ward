#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QFrame>

#include "pages/HomePage.h"
#include "pages/SettingsPage.h"

/**
 * @brief 应用程序主窗口
 *
 * 管理顶部导航栏与 QStackedWidget 页面切换。
 * 对外暴露子组件的访问器，供 AppController 完成数据绑定，
 * 窗口本身不持有任何业务逻辑。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    [[nodiscard]] VitalsWidget *getVitalsWidget() const { return m_homePage->getVitalsWidget(); }
    [[nodiscard]] VideoWidget  *getVideoWidget()  const { return m_homePage->getVideoWidget(); }

private slots:
    void onTabChanged(int index) const;

private:
    void initUI();
    void initConnections();

    QWidget        *m_centralWidget  {};
    QStackedWidget *m_stackedWidget  {};

    QFrame         *m_topBar         {};
    QPushButton    *m_homeBtn        {};
    QPushButton    *m_settingsBtn    {};

    HomePage       *m_homePage       {};
    SettingsPage   *m_settingsPage   {};
};
