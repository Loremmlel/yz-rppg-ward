#pragma once
#include <QMainWindow>
#include <QSplitter>

#include "widgets/VitalsWidget.h"
#include "widgets/VideoWidget.h"

/**
 * @brief 应用程序主视图窗口
 * 构建系统的主框架，采用分栏布局展示监控画面和关键健康体征数据。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief 构造函数，构建界面分栏框架
     * @param parent 窗口的父级容器
     */
    explicit MainWindow(QWidget *parent = nullptr);
private:
    /**
     * @brief 分栏器，支持用户动态拖放调整监控和数据的显示权重
     */
    QSplitter* m_mainSplitter;
    /**
     * @brief 视频流实时显示区域
     */
    VideoWidget* m_videoWidget;
    /**
     * @brief 多指标聚合看板组件
     */
    VitalsWidget* m_vitalsWidget;
};