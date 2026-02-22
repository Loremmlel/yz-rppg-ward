#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent){
    this->setObjectName("VideoWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 获取系统所有已链接的视频采集设备
    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        // 若无可用设备，则展示醒目的状态提示，并终止采集流程初始化
        auto* label = new QLabel(QStringLiteral("未检测到有效摄像头设备"), this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ff4d4f; font-size: 18px; font-weight: bold;");
        layout->addWidget(label);
        m_videoOutput = nullptr;
        m_captureSession = nullptr;
        m_camera = nullptr;
        return;
    }

    m_videoOutput = new QVideoWidget(this);
    m_captureSession = new QMediaCaptureSession(this);
    // 默认选用系统的首选/主摄像头设备
    m_camera = new QCamera(cameras.first(), this);

    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoOutput(m_videoOutput);

    layout->addWidget(m_videoOutput);
    
    m_camera->start();
}
