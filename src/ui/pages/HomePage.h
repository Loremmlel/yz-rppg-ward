#pragma once
#include <QSplitter>

#include "../widgets/home/MetricsPanel.h"
#include "../widgets/home/VideoWidget.h"

/**
 * @brief 主页视图
 *
 * 通过 QSplitter 构建左右可伸缩布局，持有子组件的访问器供 AppController 绑定数据。
 */
class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

    [[nodiscard]] MetricsPanel *getMetricsPanel() const { return m_metricsPanel; }
    [[nodiscard]] VideoWidget *getVideoWidget() const { return m_videoWidget; }

private:
    void initUI();

    QSplitter *m_mainSplitter{nullptr};
    VideoWidget *m_videoWidget{nullptr};
    MetricsPanel *m_metricsPanel{nullptr};
};
