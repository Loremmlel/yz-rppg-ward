#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaCaptureSession>

#include  <opencv2/opencv.hpp>
#include <opencv2//objdetect.hpp>


/**
 * @brief 实时视频监控组件
 * 封装并管理摄像头硬件交互、流采集以及画面展示逻辑。实现人脸检测
 */
class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget() override = default;
private slots:
    /**
     * @brief 拦截并处理摄像头的每一帧画面
     */
    void processVideoFrame(const QVideoFrame &frame);
private:
    /**
     * @brief 尝试设置摄像头的最佳硬件分辨率 (720p 30fps)
     */
    void setupCameraFormat();


    static QString loadModel(const QString& modelName);
private:
    QCamera* m_camera;
    QMediaCaptureSession* m_captureSession;

    QVideoSink* m_videoSink;
    QLabel* m_displayLabel;
    QLabel* m_warningLabel;

    cv::Ptr<cv::FaceDetectorYN> m_faceDetector;

    const int TARGET_WIDTH = 1280;
    const int TARGET_HEIGHT = 720;
};