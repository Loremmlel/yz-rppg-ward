#include "VideoWidget.h"
#include <QMediaDevices>
#include <QCameraDevice>
#include <QVideoSink>
#include <QVideoFrame>
#include <QMediaCaptureSession>
#include <QOpenGLWidget>
#include <QPen>
#include <QResizeEvent>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("videoWidget");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        m_noDeviceLabel = new QLabel(QStringLiteral("未检测到有效摄像头设备"), this);
        m_noDeviceLabel->setObjectName("videoDisplayLabel");
        m_noDeviceLabel->setAlignment(Qt::AlignCenter);
        m_noDeviceLabel->setProperty("noCamera", true);
        layout->addWidget(m_noDeviceLabel);
        return;
    }

    // ── GPU 渲染管线初始化 ────────────────────────────────────────────────
    m_scene = new QGraphicsScene(this);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setAspectRatioMode(Qt::KeepAspectRatio);
    m_scene->addItem(m_videoItem);

    // 人脸框叠加层：z-order 高于视频项，颜色空间无关，纯几何绘制
    m_faceRect = new QGraphicsRectItem();
    QPen pen(Qt::green);
    pen.setWidth(3);
    m_faceRect->setPen(pen);
    m_faceRect->setBrush(Qt::NoBrush);
    m_faceRect->setVisible(false);
    m_faceRect->setZValue(1.0); // 确保在 videoItem (z=0) 之上
    m_scene->addItem(m_faceRect);

    m_graphicsView = new QGraphicsView(m_scene, this);
    m_graphicsView->setObjectName("videoDisplayLabel");
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setFrameShape(QFrame::NoFrame);
    m_graphicsView->setRenderHint(QPainter::Antialiasing, false);
    m_graphicsView->setBackgroundBrush(Qt::black);

    // 使用 QOpenGLWidget 作为 viewport，所有合成由 GPU 完成
    auto *glWidget = new QOpenGLWidget(m_graphicsView);
    m_graphicsView->setViewport(glWidget);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    layout->addWidget(m_graphicsView);

    // ── 摄像头与捕获会话 ─────────────────────────────────────────────────
    m_camera = new QCamera(cameras.first(), this);
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setCamera(m_camera);

    setupCameraFormat();

    // setVideoOutput 与 setVideoSink 互斥，后者覆盖前者。
    // 解决方案：只用 setVideoSink 接收帧，在回调中手动分发：
    //   ① 转发给 QGraphicsVideoItem 内部 sink → GPU 渲染
    //   ② 发射 frameCaptured 信号           → VideoService 工作线程
    m_videoSink = new QVideoSink(this);
    m_captureSession->setVideoSink(m_videoSink);

    QVideoSink *renderSink = m_videoItem->videoSink();
    connect(m_videoSink, &QVideoSink::videoFrameChanged,
            this, [this, renderSink](const QVideoFrame &frame) {
                renderSink->setVideoFrame(frame); // GPU 渲染
                emit frameCaptured(frame); // 工作线程
            });

    m_camera->start();
}

VideoWidget::~VideoWidget() {
    if (m_camera) {
        m_camera->stop();
    }
}

void VideoWidget::updateFaceDetection(const QRect &rect, const bool hasFace) const {
    if (!m_faceRect) return;

    if (!hasFace || rect.isEmpty()) {
        m_faceRect->setVisible(false);
        return;
    }

    // 视频项在 scene 中的实际显示区域（保持宽高比后的矩形）
    const QSizeF videoNativeSize = m_videoItem->nativeSize();
    if (videoNativeSize.isEmpty()) {
        m_faceRect->setVisible(false);
        return;
    }

    // 将检测结果坐标（原始帧坐标系）映射到 videoItem 本地坐标系
    const QRectF itemBounds = m_videoItem->boundingRect();
    const double scaleX = itemBounds.width() / videoNativeSize.width();
    const double scaleY = itemBounds.height() / videoNativeSize.height();

    const QRectF mappedRect(
        itemBounds.x() + rect.x() * scaleX,
        itemBounds.y() + rect.y() * scaleY,
        rect.width() * scaleX,
        rect.height() * scaleY
    );

    m_faceRect->setRect(mappedRect);
    m_faceRect->setVisible(true);
}

void VideoWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (m_videoItem && m_graphicsView) {
        // 让 videoItem 充满 view，scene 坐标系与 view 像素坐标一致
        const QSizeF viewSize = m_graphicsView->size();
        m_videoItem->setSize(viewSize);
        m_scene->setSceneRect(0, 0, viewSize.width(), viewSize.height());
    }
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
        const auto defaultFormat = m_camera->cameraDevice().videoFormats().first();
        qWarning() << "未找到目标格式，使用系统默认:" << defaultFormat.resolution()
                << defaultFormat.maxFrameRate() << "FPS";
    }
}