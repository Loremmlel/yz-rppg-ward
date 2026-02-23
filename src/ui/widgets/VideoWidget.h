#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaCaptureSession>

#include "../../service/VideoService.h"


/**
 * @brief 实时视频监控组件
 * 封装并管理摄像头硬件交互、流采集以及画面展示逻辑。
 */
class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget() override;

    /**
     * @brief 注入视频处理服务
     */
    void setVideoService(VideoService* service);

private slots:
    /**
     * @brief 拦截并处理摄像头的每一帧画面
     */
    void processVideoFrame(const QVideoFrame &frame);
private:
    /**
     * @brief 尝试设置摄像头的最佳硬件分辨率 (720p 30fps)
     */
    void setupCameraFormat() const;

    QCamera* m_camera;
    QMediaCaptureSession* m_captureSession;

    QVideoSink* m_videoSink;
    QLabel* m_displayLabel;
    QLabel* m_warningLabel;

    VideoService* m_videoService = nullptr;

    const int TARGET_WIDTH = 1280;
    const int TARGET_HEIGHT = 720;
};

