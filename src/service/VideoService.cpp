#include "VideoService.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include "../util/ImageHelper.h"

VideoService::VideoService(QObject *parent) : QObject(parent) {
    const auto path = loadModel("face_detection_yunet_2023mar.onnx");
    if (path.isEmpty()) {
        qWarning() << "人脸检测模型加载失败";
    }
    try {
        m_faceDetector = cv::FaceDetectorYN::create(
            path.toStdString(), "", cv::Size(320, 320),
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

    m_frameSkipCounter++;

    if (m_frameSkipCounter % 5 == 0 && !m_faceDetector.empty() && !m_isProcessing.load()) {
        m_isProcessing.store(true);

        auto mat = ImageHelper::QImage2CvMat(image);

        m_processingFuture = QtConcurrent::run([this, mat] {
            detectAndUpdateRect(mat);
        });
    }
}

void VideoService::detectAndUpdateRect(cv::Mat mat) {
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

    cv::Rect newRect(0, 0, 0, 0);
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

        newRect = cv::Rect(x, y, w, h);

        if (auto roiRect = newRect & cv::Rect(0, 0, mat.cols, mat.rows);
            roiRect.width > 0 && roiRect.height > 0) {
            auto faceRoi = mat(roiRect);
            cv::Mat resizedRoi;
            cv::resize(faceRoi, resizedRoi, cv::Size(256, 256));

            cv::cvtColor(resizedRoi, resizedRoi, cv::COLOR_BGR2RGB);
            QImage roiImage(resizedRoi.data, resizedRoi.cols, resizedRoi.rows, resizedRoi.step, QImage::Format_RGB888);

            emit faceRoiExtracted(roiImage.copy());
        }
    }

    QRect qRect(newRect.x, newRect.y, newRect.width, newRect.height);

    emit facePositionUpdated(qRect, hasFace);
    m_isProcessing.store(false);
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
