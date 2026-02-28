#include "VideoService.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QDataStream>
#include <QtConcurrentRun>
#include <opencv2/imgcodecs.hpp>

#include "../util/ImageHelper.h"

VideoService::VideoService(QObject *parent) : QObject(parent) {
    const auto path = loadModel("face_detection_yunet_2023mar.onnx");
    if (path.isEmpty()) {
        qWarning() << "人脸检测模型加载失败";
    }
    try {
        m_faceDetector = cv::FaceDetectorYN::create(
            path.toStdString(), "", cv::Size(640, 480),
            0.6f,  // 置信度阈值
            0.3f,  // NMS 阈值
            5000   // 最大检测数量上限
        );
    } catch (const cv::Exception &e) {
        qWarning() << "人脸检测模型初始化失败：" << e.what();
    }
}

VideoService::~VideoService() {
    if (m_processingFuture.isRunning()) {
        m_processingFuture.waitForFinished();
    }
}

void VideoService::processFrame(const QImage &image) {
    if (image.isNull()) return;

    // ── 高频管线：每帧都执行，依赖上一次检测缓存的矩形 ──────────────────────
    if (m_hasFace && m_currentFaceRect.isValid()) {
        const QRect clipped = m_currentFaceRect.intersected(image.rect());
        if (clipped.isValid()) {
            const QByteArray frame = encodeRoi(image.copy(clipped));
            if (!frame.isEmpty()) {
                emit faceRoiEncoded(frame);
            }
        }
    }

    // ── 低频管线：每 5 帧触发一次异步检测，防止吃满 CPU ──────────────────────
    m_frameSkipCounter++;
    if (m_frameSkipCounter % 5 == 0 && !m_faceDetector.empty() && !m_isProcessing.load()) {
        m_isProcessing.store(true);
        const auto mat = ImageHelper::QImage2CvMat(image);
        m_processingFuture = QtConcurrent::run([this, mat] { detectWorker(mat); });
    }
}

void VideoService::detectWorker(const cv::Mat &mat) {
    double scale;
    cv::Mat detectionMat;

    // 缩放到 640px 宽度加速推理；检测结果坐标后续需等比放回原图尺度
    if (constexpr int targetDetWidth = 640; mat.cols > targetDetWidth) {
        scale = static_cast<double>(mat.cols) / targetDetWidth;
        cv::resize(mat, detectionMat,
                   cv::Size(targetDetWidth, static_cast<int>(mat.rows / scale)));
    } else {
        detectionMat = mat;
        scale = 1.0;
    }

    m_faceDetector->setInputSize(detectionMat.size());
    cv::Mat faces;
    m_faceDetector->detect(detectionMat, faces);

    QRect newRect;
    bool hasFace = false;
    if (faces.rows > 0) {
        // 多张人脸时取面积最大的，假设目标患者是画面主体
        hasFace = true;
        int maxArea = 0, maxIndex = 0;
        for (int i = 0; i < faces.rows; i++) {
            const int w = static_cast<int>(faces.at<float>(i, 2) * scale);
            const int h = static_cast<int>(faces.at<float>(i, 3) * scale);
            if (w * h > maxArea) { maxArea = w * h; maxIndex = i; }
        }

        const int x = static_cast<int>(faces.at<float>(maxIndex, 0) * scale);
        const int y = static_cast<int>(faces.at<float>(maxIndex, 1) * scale);
        const int w = static_cast<int>(faces.at<float>(maxIndex, 2) * scale);
        const int h = static_cast<int>(faces.at<float>(maxIndex, 3) * scale);
        newRect = QRect(x, y, w, h);
    }

    // 检测在工作线程完成，回到主线程写状态并发信号，避免竞态
    QMetaObject::invokeMethod(this, [this, newRect, hasFace] {
        m_currentFaceRect = newRect;
        m_hasFace = hasFace;
        emit facePositionUpdated(newRect, hasFace);
        m_isProcessing.store(false);
    }, Qt::QueuedConnection);
}

QByteArray VideoService::encodeRoi(const QImage &roi) {
    const cv::Mat mat = ImageHelper::QImage2CvMat(roi);
    std::vector<uchar> webpBuf;
    const std::vector<int> params = {cv::IMWRITE_WEBP_QUALITY, 101}; // 101 = 无损
    if (!cv::imencode(".webp", mat, webpBuf, params)) {
        qWarning() << "[VideoService] 图像 WebP 编码失败";
        return {};
    }

    const qint64 timestampMs = QDateTime::currentMSecsSinceEpoch();
    QByteArray frame;
    frame.reserve(static_cast<qsizetype>(sizeof(qint64) + webpBuf.size()));
    QDataStream ds(&frame, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << timestampMs;
    frame.append(reinterpret_cast<const char *>(webpBuf.data()),
                 static_cast<qsizetype>(webpBuf.size()));
    return frame;
}

QString VideoService::loadModel(const QString &modelName) {
    QString localPath = QDir::currentPath() + QDir::separator() + modelName;

    if (QFile::exists(localPath)) {
        return localPath;
    }

    // 首次运行时从资源包解压到本地，后续直接复用
    if (QFile(":/models/" + modelName).copy(localPath)) {
        return localPath;
    }

    return {};
}
