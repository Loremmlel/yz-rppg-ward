#include "VideoService.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QtConcurrentRun>
#include <algorithm>

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
        if (m_currentFaceRect.intersected(image.rect()).width() > 0) {
            constexpr int OUTPUT_SIZE = 256;

            // 长边扩展 1.2 倍后取正方形，确保人脸在 rPPG 算法输入时不被裁切
            const int faceW = m_currentFaceRect.width();
            const int faceH = m_currentFaceRect.height();
            const int squareSide = static_cast<int>(std::max(faceW, faceH) * 1.2);

            const QPoint center = m_currentFaceRect.center();
            const int half = squareSide / 2;
            const QRect cropRect(center.x() - half, center.y() - half, squareSide, squareSide);

            // 先在黑底画布上绘制，处理超出原图边界的情况
            QImage roiImage(squareSide, squareSide, QImage::Format_RGB888);
            roiImage.fill(Qt::black);

            const QRect srcRect = cropRect.intersected(image.rect());
            const QRect dstRect = srcRect.translated(-cropRect.topLeft());

            if (srcRect.isValid()) {
                QPainter painter(&roiImage);
                painter.drawImage(dstRect, image, srcRect);
            }

            const QImage scaledRoi = roiImage.scaled(OUTPUT_SIZE, OUTPUT_SIZE,
                                                      Qt::IgnoreAspectRatio,
                                                      Qt::SmoothTransformation);
            emit faceRoiExtracted(scaledRoi);
        }
    }

    // ── 低频管线：每 10 帧触发一次异步检测，防止吃满 CPU ──────────────────────
    m_frameSkipCounter++;
    if (m_frameSkipCounter % 10 == 0 && !m_faceDetector.empty() && !m_isProcessing.load()) {
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
