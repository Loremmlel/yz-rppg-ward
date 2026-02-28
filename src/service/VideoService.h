#pragma once

#include <atomic>
#include <QRect>
#include <QFuture>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

/**
 * @brief 视频帧处理服务
 *
 * 内部维护两条并行管线：
 *  - 高频管线（每帧）：基于上一次检测结果直接裁剪人脸检测框区域，
 *    在工作线程内完成无损 WebP 编码后发射 faceRoiEncoded。
 *  - 低频管线（每 5 帧）：异步运行 YuNet 人脸检测，更新矩形框位置。
 *
 * 两条管线解耦，保证 rPPG 数据流不受检测延迟影响。
 */
class VideoService : public QObject {
    Q_OBJECT

public:
    explicit VideoService(QObject *parent = nullptr);
    ~VideoService() override;

public slots:
    void processFrame(const QImage &image);

signals:
    void facePositionUpdated(const QRect &rect, bool hasFace);

    /** 已编码的无损 WebP 帧（含 8 字节时间戳头），可直接通过 WebSocket 发送。 */
    void faceRoiEncoded(const QByteArray &frame);

private:
    void detectWorker(const cv::Mat &mat);

    /** 将人脸 ROI 编码为 [8B 时间戳 + 无损 WebP]，返回空则编码失败。 */
    static QByteArray encodeRoi(const QImage &roi);

    /**
     * @brief 将模型从 Qt 资源包解压到可执行文件同级目录
     *
     * OpenCV 的 FaceDetectorYN::create 只接受本地文件路径，无法直接读取 qrc 虚拟路径。
     */
    static QString loadModel(const QString &modelName);

    cv::Ptr<cv::FaceDetectorYN> m_faceDetector;
    std::atomic<bool>           m_isProcessing{false};
    QFuture<void>               m_processingFuture;

    QRect m_currentFaceRect;
    bool  m_hasFace           {false};
    int   m_frameSkipCounter  {0};
};
