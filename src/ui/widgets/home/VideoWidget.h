#pragma once
#include <QWidget>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoFrame>
#include <QVideoSink>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsRectItem>
#include <QMediaCaptureSession>
#include <QLabel>

/**
 * @brief 摄像头视频预览组件
 *
 * 渲染管线：QGraphicsVideoItem（GPU YUV→RGB，无 CPU 颜色空间转换）
 *   + QOpenGLWidget viewport（GPU 合成）
 *   + QGraphicsRectItem 人脸框覆盖层（同一 scene，z-order 可控）。
 *
 * 主线程完全不调用 toImage()；QVideoSink 仍挂在 captureSession，
 * 仅用于向 VideoService 工作线程发射 QVideoFrame，供 OpenCV 处理。
 */
class VideoWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);

    ~VideoWidget() override;

    signals:
    /** 摄像头原始帧（QVideoFrame 引用计数浅拷贝），发往 VideoService 在工作线程处理。 */
    

    void frameCaptured(const QVideoFrame &frame);

public
    slots:
    /** 由 VideoService::facePositionUpdated 信号驱动，更新人脸框覆盖层。 */
    

    void updateFaceDetection(const QRect &rect, bool hasFace) const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief 从设备支持列表中匹配 TARGET_WIDTH×TARGET_HEIGHT @ 25-30fps 的格式
     */
    void setupCameraFormat() const;

    QCamera *m_camera{nullptr};
    QMediaCaptureSession *m_captureSession{nullptr};

    // ── GPU 渲染管线 ──────────────────────────────────────────────────────
    QGraphicsView *m_graphicsView{nullptr};
    QGraphicsScene *m_scene{nullptr};
    QGraphicsVideoItem *m_videoItem{nullptr}; ///< 视频层（GPU YUV→RGB shader）
    QGraphicsRectItem *m_faceRect{nullptr}; ///< 人脸框叠加层（高于视频层）

    // ── 工作线程管线（保留供 VideoService 使用，主线程不消费帧数据）────────
    QVideoSink *m_videoSink{nullptr};

    // ── 无摄像头提示标签 ──────────────────────────────────────────────────
    QLabel *m_noDeviceLabel{nullptr};

    const int TARGET_WIDTH{1920};
    const int TARGET_HEIGHT{1080};
};