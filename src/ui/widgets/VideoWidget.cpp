#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>
#include <QVideoSink>
#include <QDir>
#include <QtConcurrent/QtConcurrent>

#include "../../util/ImageHelper.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent), m_currentFaceRect(0, 0, 0, 0){
    this->setObjectName("VideoWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto path = loadModel("face_detection_yunet_2023mar.onnx");
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

    m_displayLabel = new QLabel(this);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setStyleSheet("background-color: black;");

    m_warningLabel = new QLabel("未检测到患者人脸", this);
    m_warningLabel->setAlignment(Qt::AlignCenter);
    m_warningLabel->setStyleSheet("background-color: rgba(255, 0, 0, 150); color: white; font-size: 16px; font-weight: bold; padding: 10px;");
    m_warningLabel->setVisible(false);
    m_warningLabel->setParent(m_displayLabel);

    layout->addWidget(m_displayLabel);

    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        m_displayLabel->setText("未检测到有效摄像头设备");
        m_displayLabel->setStyleSheet("color: #ff4d4f; font-size: 18px;");
        return;
    }

    m_camera = new QCamera(cameras.first(), this);
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setCamera(m_camera);

    setupCameraFormat();

    m_videoSink = new QVideoSink(this);
    m_captureSession->setVideoSink(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &VideoWidget::processVideoFrame);

    m_camera->start();
}

VideoWidget::~VideoWidget() = default;

void VideoWidget::processVideoFrame(const QVideoFrame &frame) {
    // Qt视频帧转换为QImage
    auto image = frame.toImage();
    if (image.isNull()) {
        return;
    }

    // 转换为 Mat (为了方便裁剪和显示)
    // 注意：这里为了性能，如果仅显示可以用 QVideoFrame 直接转 QPixmap，
    // 但为了后续裁剪传输，我们先转 Mat。
    auto mat = ImageHelper::QImage2CvMat(image);

    // 人脸检测，低频执行
    m_frameSkipCounter++;
    bool shouldDetect = (m_frameSkipCounter % 5 == 0); // 每5帧检测一次

    if (shouldDetect && !m_faceDetector.empty() && !m_isProcessing.load()) {
        m_isProcessing.store(true);
        QtConcurrent::run([this, mat] {
            detectAndUpdateRect(mat.clone());
        });
    }

    auto displayMat = mat.clone();

    cv::Rect faceToDraw;
    {
        std::lock_guard lock(m_faceRectMutex);
        faceToDraw = m_currentFaceRect;
    }
    const bool hasFace = (faceToDraw.width > 0 && faceToDraw.height > 0);
    if (hasFace) {
        cv::rectangle(displayMat, faceToDraw, cv::Scalar(0, 255, 0), 2);
    }

    // 转回QImage显示
    cv::cvtColor(displayMat, displayMat, cv::COLOR_BGR2RGB);
    QImage displayImage(displayMat.data, displayMat.cols, displayMat.rows, displayMat.step, QImage::Format_RGB888);

    // 更新 UI (直接更新，无需 invokeMethod，因为 processVideoFrame 通常就在主线程
    m_displayLabel->setPixmap(QPixmap::fromImage(displayImage)
        .scaled(m_displayLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation));
    m_warningLabel->setVisible(!hasFace);

    if (hasFace) {
        // TODO: 裁剪256*256并传输给后端
    }
}

void VideoWidget::detectAndUpdateRect(cv::Mat mat) {
    double scale = 1.0;

    cv::Mat detectionMat;
    int targetDetWidth = 640;

    if (mat.cols > targetDetWidth) {
        scale = static_cast<double>(mat.cols) / targetDetWidth;
        cv::resize(mat, detectionMat, cv::Size(targetDetWidth, static_cast<int>(mat.rows / scale)));
    } else {
        detectionMat = mat;
        scale = 1.0;
    }

    m_faceDetector->setInputSize(detectionMat.size());
    cv::Mat faces;
    m_faceDetector->detect(detectionMat, faces);

    cv::Rect newRect(0,0,0,0);
    if (faces.rows > 0) {
        // 策略：取最大的人脸
        int maxArea = 0;
        int maxIndex = 0;
        for (int i = 0; i < faces.rows; i++) {
            int w = static_cast<int>(faces.at<float>(i, 2) * scale);
            int h = static_cast<int>(faces.at<float>(i, 3) * scale);
            if (w * h > maxArea) {
                maxArea = w * h;
                maxIndex = i;
            }
        }

        // 映射回原图坐标
        int x = static_cast<int>(faces.at<float>(maxIndex, 0) * scale);
        int y = static_cast<int>(faces.at<float>(maxIndex, 1) * scale);
        int w = static_cast<int>(faces.at<float>(maxIndex, 2) * scale);
        int h = static_cast<int>(faces.at<float>(maxIndex, 3) * scale);

        newRect = cv::Rect(x, y, w, h);
    }

    // 更新缓存坐标
    {
        std::lock_guard lock(m_faceRectMutex);
        m_currentFaceRect = newRect;
    }
    m_isProcessing.store(false);
}

void VideoWidget::setupCameraFormat() {
    QCameraFormat bestFormat;
    // 寻找最接近 720p 30fps 的硬件输出格式
    for (const auto& format : m_camera->cameraDevice().videoFormats()) {
        if (format.resolution() == QSize(TARGET_WIDTH, TARGET_HEIGHT) &&
            format.maxFrameRate() >= 25.0f && format.maxFrameRate() <= 30.0f) {
            bestFormat = format;
            break;
            }
    }

    if (!bestFormat.isNull()) {
        m_camera->setCameraFormat(bestFormat);
        qDebug() << "成功设置摄像头硬件格式为:" << bestFormat.resolution() << bestFormat.maxFrameRate() << "FPS";
    } else {
        qWarning() << "未能找到 720p@30FPS 硬件格式，将使用系统默认，并在后续进行软件降采样。";
    }
}

QString VideoWidget::loadModel(const QString &modelName) {
    const QString qrcPath = ":/models/" + modelName;
    QFile modelFile(qrcPath);
    QString localPath = QDir::currentPath() + QDir::separator() + modelName;

    if (QFile::exists(localPath)) {
        qDebug() << "人脸检测模型已存在：" << localPath;
    } else {
        if (modelFile.copy(localPath)) {
            qDebug() << "人脸检测模型复制到：" << localPath;
        } else {
            qDebug() << "人脸检测模型复制失败，从：" << qrcPath << "到" << localPath << "原因：" << modelFile.errorString();
            return "";
        }
    }
    return localPath;
}