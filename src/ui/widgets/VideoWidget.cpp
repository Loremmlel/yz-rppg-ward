#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent){
    this->setObjectName("VideoWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        auto* label = new QLabel(QStringLiteral("未检测到摄像头"), this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: white; font-size: 18px;");
        layout->addWidget(label);
        m_videoOutput = nullptr;
        m_captureSession = nullptr;
        m_camera = nullptr;
        return;
    }

    m_videoOutput = new QVideoWidget(this);
    m_captureSession = new QMediaCaptureSession(this);
    m_camera = new QCamera(cameras.first(), this);

    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoOutput(m_videoOutput);

    layout->addWidget(m_videoOutput);

    if (m_camera) {
        m_camera->start();
    }
}
