#pragma once

#include <atomic>
#include <QRect>
#include <QFuture>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

#include "../util/KalmanFilter1D.h"

/**
 * @brief 视频帧处理服务
 *
 * 内部维护两条并行管线：
 *  - 高频管线（每帧）：基于卡尔曼滤波平滑/预测的人脸框裁剪 ROI，
 *    在工作线程内完成无损 WebP 编码后发射 faceRoiEncoded。
 *  - 低频管线（每 5 帧）：异步运行 YuNet 人脸检测，用检测结果更新卡尔曼滤波器。
 *
 * 两条管线解耦，保证 rPPG 数据流不受检测延迟影响。
 * 卡尔曼滤波消除帧间抖动，并在跳帧时提供位置预测。
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
    std::atomic<bool> m_isProcessing{false};
    std::atomic<bool> m_isEncoding{false};
    QFuture<void> m_processingFuture;

    QRect m_currentFaceRect;
    QRect m_rawFaceRect; ///< 最近一次检测器输出的原始框（未滤波）
    bool m_hasFace{false};
    bool m_hasKalman{false}; ///< 卡尔曼滤波器是否已初始化
    int m_frameSkipCounter{0};

    // 卡尔曼滤波器：分别跟踪人脸框的 x, y, w, h
    // 参数参考 open-rppg: processNoise=0.01, measureNoise=0.5, initialError=1
    KalmanFilter1D m_kfX{0.01, 0.5, 0.0, 1.0};
    KalmanFilter1D m_kfY{0.01, 0.5, 0.0, 1.0};
    KalmanFilter1D m_kfW{0.01, 0.5, 0.0, 1.0};
    KalmanFilter1D m_kfH{0.01, 0.5, 0.0, 1.0};
};
