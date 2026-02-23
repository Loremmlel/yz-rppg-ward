#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>
#include <QPainter>
#include <QVideoSink>
#include <QMediaCaptureSession>

#include "../../util/ImageHelper.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("VideoWidget");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_displayLabel = new QLabel(this);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setStyleSheet("background-color: black;");

    m_warningLabel = new QLabel("未检测到患者人脸", this);
    m_warningLabel->setAlignment(Qt::AlignCenter);
    m_warningLabel->setStyleSheet(
        "background-color: rgba(255, 0, 0, 150); color: white; font-size: 16px; font-weight: bold; padding: 10px;");
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

void VideoWidget::updateFaceDetection(const QRect &rect, bool hasFace) {
    m_currentFaceRect = rect;
    m_hasFace = hasFace;
    m_warningLabel->setVisible(!hasFace);
}

void VideoWidget::processVideoFrame(const QVideoFrame &frame) {
    auto image = frame.toImage();
    if (image.isNull()) return;

    emit frameCaptured(image);

    image = image.convertToFormat(QImage::Format_RGB888);

    if (m_hasFace) {
        QPainter painter(&image);
        QPen pen(Qt::green);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRect(m_currentFaceRect);
    }

    m_displayLabel->setPixmap(QPixmap::fromImage(image)
        .scaled(m_displayLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation));
}

void VideoWidget::setupCameraFormat() const {
    QCameraFormat bestFormat;
    for (const auto &format: m_camera->cameraDevice().videoFormats()) {
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
        auto defaultFormat = m_camera->cameraDevice().videoFormats().first();
        qWarning() << "未能找到合适的硬件格式，将使用系统默认。格式为：" << defaultFormat.resolution() << defaultFormat.maxFrameRate() << "FPS";
    }
}
