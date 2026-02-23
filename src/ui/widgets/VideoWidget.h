#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>

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

signals:
    /**
     * @brief 向外抛出原始视频帧
     */
    void frameCaptured(const QImage &image);

public slots:
    /**
     * @brief 接收传回来的检测结果，画框框
     */
    void updateFaceDetection(const QRect &rect, bool hasFace);

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

    QCamera *m_camera;
    QMediaCaptureSession *m_captureSession;
    QVideoSink *m_videoSink;
    QLabel *m_displayLabel;
    QLabel *m_warningLabel;

    QRect m_currentFaceRect;
    bool m_hasFace = false;

    const int TARGET_WIDTH = 1280;
    const int TARGET_HEIGHT = 720;
};
