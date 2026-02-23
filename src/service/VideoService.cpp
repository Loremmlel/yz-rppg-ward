#include "VideoService.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

VideoService::VideoService(QObject *parent) : QObject(parent), m_currentFaceRect(0, 0, 0, 0) {
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
    } catch (const cv::Exception& e) {
        qWarning() << "人脸检测模型初始化失败：" << e.what();
    }
}

VideoService::~VideoService() {
    if (m_processingFuture.isRunning()) {
        m_processingFuture.waitForFinished();
    }
}

void VideoService::processFrame(const cv::Mat& mat) {
    m_frameSkipCounter++;

    // 控制检测间隔，且确保当前没有正在处理的后台任务
    if (const auto shouldDetect = m_frameSkipCounter % 5 == 0;
        shouldDetect && !m_faceDetector.empty() && !m_isProcessing.load()) {

        m_isProcessing.store(true);
        m_processingFuture = QtConcurrent::run([this, matClone = mat.clone()] {
            detectAndUpdateRect(matClone);
        });
    }

    if (isFaceLocked()) {
        // TODO: 裁剪256*256并通过网络服务传输给后端 (即 userRequest 提到的传输部分)
    }
}

cv::Rect VideoService::currentFaceRect() const {
    std::lock_guard lock(m_faceRectMutex);
    return m_currentFaceRect;
}

bool VideoService::isFaceLocked() const {
    std::lock_guard lock(m_faceRectMutex);
    return m_currentFaceRect.width > 0 && m_currentFaceRect.height > 0;
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
    if (faces.rows > 0) {
        // 策略：取最大的人脸
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

        // 映射回���图坐标
        const int x = static_cast<int>(faces.at<float>(maxIndex, 0) * scale);
        const int y = static_cast<int>(faces.at<float>(maxIndex, 1) * scale);
        const int w = static_cast<int>(faces.at<float>(maxIndex, 2) * scale);
        const int h = static_cast<int>(faces.at<float>(maxIndex, 3) * scale);

        newRect = cv::Rect(x, y, w, h);
    }

    // 更新缓存坐标
    {
        std::lock_guard lock(m_faceRectMutex);
        m_currentFaceRect = newRect;
    }

    emit facePositionUpdated(newRect);
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

