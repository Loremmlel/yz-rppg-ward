#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>
#include <QVideoSink>

#include "../../util/ImageHelper.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("VideoWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

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

VideoWidget::~VideoWidget() {
    if (m_camera) {
        m_camera->stop();
    }
}

void VideoWidget::setVideoService(VideoService *service) {
    m_videoService = service;
}

void VideoWidget::processVideoFrame(const QVideoFrame &frame) {
    // Qt视频帧转换为QImage
    const auto image = frame.toImage();
    if (image.isNull()) {
        return;
    }

    const auto mat = ImageHelper::QImage2CvMat(image);

    // 将图像交给后台 Service 进行处理（检测、传输等）
    if (m_videoService) {
        m_videoService->processFrame(mat);
    }

    auto displayMat = mat.clone();

    // 从 Service 获取最新的人脸检测结果用于 UI 渲染
    cv::Rect faceToDraw;
    bool hasFace = false;
    if (m_videoService) {
        faceToDraw = m_videoService->currentFaceRect();
        hasFace = m_videoService->isFaceLocked();
    }

    if (hasFace) {
        cv::rectangle(displayMat, faceToDraw, cv::Scalar(0, 255, 0), 2);
    }

    // 转回QImage显示
    cv::cvtColor(displayMat, displayMat, cv::COLOR_BGR2RGB);
    QImage displayImage(displayMat.data, displayMat.cols, displayMat.rows, displayMat.step, QImage::Format_RGB888);

    // 更新 UI (直接更新，无需 invokeMethod，因为 processVideoFrame 通常就在主线程)
    m_displayLabel->setPixmap(QPixmap::fromImage(displayImage)
        .scaled(m_displayLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation));
    m_warningLabel->setVisible(!hasFace);
}

void VideoWidget::setupCameraFormat() const {
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

