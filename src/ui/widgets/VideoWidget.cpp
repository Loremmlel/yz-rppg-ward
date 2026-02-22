#include "VideoWidget.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent){
    this->setObjectName("VideoWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_videoOutput = new QVideoWidget(this);
    m_captureSession = new QMediaCaptureSession(this);
    m_camera = new QCamera(this);

    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoOutput(m_videoOutput);

    layout->addWidget(m_videoOutput);

    if (m_camera) {
        m_camera->start();
    }
}
