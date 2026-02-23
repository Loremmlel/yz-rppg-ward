#pragma once

#include <QObject>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <atomic>
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

public slots:
    void processFrame(const QImage &image);

signals:
    /**
     * @brief 当检测结果发生显著更新时发射
     */
    void facePositionUpdated(const QRect &rect, bool hasFace);

    /**
     * 提取出人脸特征图，发送给NetworkService
     * @param roiImage
     */
    void faceRoiExtracted(const QImage &roiImage);

private:
    /**
     * @brief 内部执行人脸检测核心逻辑，由单独的工作线程调用
     */
    void detectAndUpdateRect(cv::Mat mat);

    /**
     * @brief 模型资源加载辅助
     */
    static QString loadModel(const QString &modelName);

    cv::Ptr<cv::FaceDetectorYN> m_faceDetector;
    std::atomic<bool> m_isProcessing{false};
    QFuture<void> m_processingFuture;

    int m_frameSkipCounter = 0;
};
