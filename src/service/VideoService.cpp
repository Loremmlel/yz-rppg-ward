#include "VideoService.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QtConcurrent/QtConcurrent>

#include "../util/ImageHelper.h"

VideoService::VideoService(QObject *parent) : QObject(parent) {
    const auto path = loadModel("face_detection_yunet_2023mar.onnx");
    if (path.isEmpty()) {
        qWarning() << "人脸检测模型加载失败";
    }
    try {
        m_faceDetector = cv::FaceDetectorYN::create(
            path.toStdString(), "", cv::Size(640, 480),
            0.6f,
            0.3f,
            5000
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

    // ========================================================
    // 流管线 A：rPPG 高频传输流 (严格按摄像头帧率运行)
    // ========================================================
    if (m_hasFace && m_currentFaceRect.isValid()) {
        if (m_currentFaceRect.intersected(image.rect()).width() > 0) {
            constexpr int OUTPUT_SIZE = 256;
            // 以人脸矩形中心为基准，裁剪固定 256x256 区域
            const QPoint center = m_currentFaceRect.center();
            constexpr int half = OUTPUT_SIZE / 2;
            const QRect cropRect(center.x() - half, center.y() - half, OUTPUT_SIZE, OUTPUT_SIZE);

            QImage roiImage(OUTPUT_SIZE, OUTPUT_SIZE, QImage::Format_RGB888);
            roiImage.fill(Qt::black);

            // 计算源矩形（原图内的有效部分）与目标矩形（roiImage 内的对应位置）
            const QRect srcRect = cropRect.intersected(image.rect());
            const QRect dstRect = srcRect.translated(-cropRect.topLeft());

            if (srcRect.isValid()) {
                QPainter painter(&roiImage);
                painter.drawImage(dstRect, image, srcRect);
            }

            emit faceRoiExtracted(roiImage);
        }
    }

    // ========================================================
    // 流管线 B：人脸检测低频更新流 (降频执行，防止吃满 CPU)
    // ========================================================
    m_frameSkipCounter++;
    if (m_frameSkipCounter % 5 == 0 && !m_faceDetector.empty() && !m_isProcessing.load()) {
        m_isProcessing.store(true);

        auto mat = ImageHelper::QImage2CvMat(image);

        m_processingFuture = QtConcurrent::run([this, mat] {
            detectWorker(mat);
        });
    }
}

void VideoService::detectWorker(const cv::Mat &mat) {
    double scale;
    cv::Mat detectionMat;

    // 为了加速检测，内部进行必要的缩放
    if (constexpr int targetDetWidth = 640; mat.cols > targetDetWidth) {
        scale = static_cast<double>(mat.cols) / targetDetWidth;
        cv::resize(mat, detectionMat, cv::Size(targetDetWidth, static_cast<int>(mat.rows / scale)));
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
        // 策略：取最大的人脸
        hasFace = true;
        int maxArea = 0;
        int maxIndex = 0;
        for (int i = 0; i < faces.rows; i++) {
            const int w = static_cast<int>(faces.at<float>(i, 2) * scale);
            const int h = static_cast<int>(faces.at<float>(i, 3) * scale);
            if (w * h > maxArea) {
                maxArea = w * h;
                maxIndex = i;
            }
        }

        // 坐标转换回原图尺度
        const int x = static_cast<int>(faces.at<float>(maxIndex, 0) * scale);
        const int y = static_cast<int>(faces.at<float>(maxIndex, 1) * scale);
        const int w = static_cast<int>(faces.at<float>(maxIndex, 2) * scale);
        const int h = static_cast<int>(faces.at<float>(maxIndex, 3) * scale);

        newRect = QRect(x, y, w, h);
    }
    QMetaObject::invokeMethod(this, [this, newRect, hasFace] {
        this->m_currentFaceRect = newRect;
        this->m_hasFace = hasFace;

        emit facePositionUpdated(newRect, hasFace);
        this->m_isProcessing.store(false);
    }, Qt::QueuedConnection);
}

QString VideoService::loadModel(const QString &modelName) {
    const QString qrcPath = ":/models/" + modelName;
    QFile modelFile(qrcPath);
    QString localPath = QDir::currentPath() + QDir::separator() + modelName;

    if (QFile::exists(localPath)) {
        return localPath;
    }

    if (modelFile.copy(localPath)) {
        return localPath;
    }

    return "";
}
