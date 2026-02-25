#pragma once
#include <QSplitter>

#include "../widgets/VitalsWidget.h"
#include "../widgets/VideoWidget.h"

/**
 * @brief 主页视图
 * 负责展示视频流和生命体征数据的分栏布局。
 */
class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

    /**
     * @brief 提供对指标监控区的访问
     */
    [[nodiscard]] VitalsWidget *getVitalsWidget() const { return m_vitalsWidget; }

    /**
     * @brief 提供对监控显示区的访问
     */
    [[nodiscard]] VideoWidget *getVideoWidget() const { return m_videoWidget; }

private:
    /**
     * @brief 初始化UI布局
     */
    void initUI();

    QSplitter *m_mainSplitter;
    VideoWidget *m_videoWidget;
    VitalsWidget *m_vitalsWidget;
};

