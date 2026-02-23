#pragma once

#include <QObject>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <atomic>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

/**
 * @brief 视频数据处理与传输服务
 * 负责视频流中的人脸检测算法执行，以及处理后的核心数据(如人脸裁剪图)的外部传输。
 */
class VideoService : public QObject {
    Q_OBJECT
public:
    explicit VideoService(QObject *parent = nullptr);
    ~VideoService() override;

    /**
     * @brief 尝试对传入的视频帧进行特征提取（如人脸检测）
     * 这是一个非阻塞调用，内部维护频率控制与异步线程
     * @param mat 输入的原始图像矩阵
     */
    void processFrame(const cv::Mat& mat);

    /**
     * @brief 返回当前处理链路中缓存的最新人脸位置矩形
     */
    cv::Rect currentFaceRect() const;

    /**
     * @brief 判断当前是否处于有效的人脸锁定状态
     */
    bool isFaceLocked() const;

signals:
    /**
     * @brief 当检测结果发生显著更新时发射
     */
    void facePositionUpdated(const cv::Rect& rect);

private:
    /**
     * @brief 内部执行人脸检测核心逻辑，由单独的工作线程调用
     */
    void detectAndUpdateRect(cv::Mat mat);

    /**
     * @brief 模型资源加载辅助
     */
    static QString loadModel(const QString& modelName);

    cv::Ptr<cv::FaceDetectorYN> m_faceDetector;
    std::atomic<bool> m_isProcessing{false};
    QFuture<void> m_processingFuture;

    mutable std::mutex m_faceRectMutex;
    cv::Rect m_currentFaceRect;

    int m_frameSkipCounter = 0;
};

