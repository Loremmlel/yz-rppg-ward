#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>
#include <QPainter>
#include <QVideoSink>
#include <QVideoFrame>
#include <QMediaCaptureSession>

#include "../../../util/ImageHelper.h"
#include "../../../util/StyleLoader.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("videoWidget");
    StyleLoader::apply(this, QStringLiteral(":/styles/video_widget.qss"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_displayLabel = new QLabel(this);
    m_displayLabel->setObjectName("videoDisplayLabel");
    m_displayLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_displayLabel);

    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        m_displayLabel->setText("未检测到有效摄像头设备");
        m_displayLabel->setProperty("noCamera", true);
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
}

void VideoWidget::processVideoFrame(const QVideoFrame &frame) {
    if (!frame.isValid()) return;

    // 发射 QVideoFrame（引用计数浅拷贝，无全帧像素拷贝），VideoService 在工作线程内做 toImage()
    emit frameCaptured(frame);

    // ── 主线程预览：独立转换一次用于显示，不影响 VideoService 管线 ──────────
    // 预览降分辨率以节省主线程时间；FastTransformation 避免软件双线性插值开销
    const QImage image = frame.toImage();
    if (image.isNull()) return;

    QImage displayImage = image;
    if (m_hasFace) {
        QPainter painter(&displayImage);
        QPen pen(Qt::green);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRect(m_currentFaceRect);
    }

    m_displayLabel->setPixmap(QPixmap::fromImage(displayImage)
        .scaled(m_displayLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
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
        qDebug() << "摄像头格式:" << bestFormat.resolution() << bestFormat.maxFrameRate() << "FPS";
    } else {
        auto defaultFormat = m_camera->cameraDevice().videoFormats().first();
        qWarning() << "未找到目标格式，使用系统默认:" << defaultFormat.resolution()
                << defaultFormat.maxFrameRate() << "FPS";
    }
}
