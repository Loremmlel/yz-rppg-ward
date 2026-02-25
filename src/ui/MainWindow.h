#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QPushButton>
#include <QFrame>

#include "widgets/VitalsWidget.h"
#include "widgets/VideoWidget.h"

/**
 * @brief 应用程序主视图窗口
 * 构建系统的主框架，顶部包含导航栏，下方为主内容区。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数，构建界面分栏框架
     * @param parent 窗口的父级容器
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 提供对指标监控区的访问，方便控制器进行数据绑定或信号透传
     * @return 成员变量 vitals 看板实例
     */
    VitalsWidget *getVitalsWidget() const { return m_vitalsWidget; }

    /**
     * @brief 提供对监控显示区的访问
     */
    VideoWidget *getVideoWidget() const { return m_videoWidget; }

private slots:
    void onTabChanged(int index) const;

private:
    /**
     * @brief 初始化UI组件
     */
    void initUI();

    /**
     * @brief 初始化信号槽连接
     */
    void initConnections();

    QWidget *m_centralWidget{};
    QStackedWidget *m_stackedWidget{};

    // 顶部导航栏
    QFrame *m_topBar{};
    QPushButton *m_homeBtn{};
    QPushButton *m_settingsBtn{};

    // 页面内容
    QWidget *m_homePage{};
    QWidget *m_settingsPage{};

    /**
     * @brief 分栏器，支持用户动态拖放调整监控和数据的显示权重
     */
    QSplitter *m_mainSplitter{};
    /**
     * @brief 视频流实时显示区域
     */
    VideoWidget *m_videoWidget{};
    /**
     * @brief 多指标聚合看板组件
     */
    VitalsWidget *m_vitalsWidget{};
};
