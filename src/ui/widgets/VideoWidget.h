#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaCaptureSession>
#include <QFuture>

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
    ~VideoWidget() override;
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

    /**
     * @brief 把模型复制到可执行文件目录下，返回绝对路径
     */
    static QString loadModel(const QString& modelName);

    /**
     * @brief 执行人脸检测，然后更新当前人脸坐标缓存
     */
    void detectAndUpdateRect(cv::Mat mat);

    QCamera* m_camera;
    QMediaCaptureSession* m_captureSession;

    QVideoSink* m_videoSink;
    QLabel* m_displayLabel;
    QLabel* m_warningLabel;

    cv::Ptr<cv::FaceDetectorYN> m_faceDetector;

    std::atomic<bool> m_isProcessing{false};
    QFuture<void> m_processingFuture;

    int m_frameSkipCounter = 0;  // 帧跳过计数器

    std::mutex m_faceRectMutex;
    cv::Rect m_currentFaceRect;

    const int TARGET_WIDTH = 1280;
    const int TARGET_HEIGHT = 720;
};