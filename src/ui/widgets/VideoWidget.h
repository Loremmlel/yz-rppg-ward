#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaCaptureSession>


/**
 * @brief 实时视频监控组件
 * 封装并管理摄像头硬件交互、流采集以及画面展示逻辑。
 */
class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

private:
    /**
     * @brief 视频流渲染载体
     */
    QVideoWidget* m_videoOutput;
    /**
     * @brief 摄像头硬件抽象实例
     */
    QCamera* m_camera;
    /**
     * @brief 捕捉会话控制器，连接摄像头源与输出窗口
     */
    QMediaCaptureSession* m_captureSession;
};