#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoFrame>
#include <QVideoWidget>

#include "../../../service/VideoService.h"


/**
 * @brief 摄像头视频预览组件
 *
 * 负责摄像头初始化、帧采集与画面渲染。
 * 每帧通过 frameCaptured 发射 QVideoFrame（引用计数，无深拷贝），
 * 由 VideoService 在工作线程内完成 YUV→RGB 转换与后续处理。
 * 主线程仅负责预览渲染，不参与任何图像处理。
 * 人脸检测结果由外部回调 updateFaceDetection 注入，组件不持有检测逻辑。
 * 状态提示（人脸未检测到、床位未绑定等）由外部 StatusBar 管理。
 */
class VideoWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);

    ~VideoWidget() override;

signals:
    /** 摄像头原始帧（QVideoFrame 引用计数浅拷贝），发往 VideoService 在工作线程处理。 */
    void frameCaptured(const QVideoFrame &frame);

public slots:
    void updateFaceDetection(const QRect &rect, bool hasFace);

private slots:
    void processVideoFrame(const QVideoFrame &frame);

private:
    /**
     * @brief 从设备支持列表中匹配 1920×1080 @ 25-30fps 的格式
     *
     * 优先使用硬件原生格式，避免软件缩放带来的额外 CPU 开销。
     */
    void setupCameraFormat() const;

    QCamera *m_camera{nullptr};
    QMediaCaptureSession *m_captureSession{nullptr};
    QVideoSink *m_videoSink{nullptr};
    QLabel *m_displayLabel{nullptr};

    QRect m_currentFaceRect;
    bool m_hasFace{false};

    const int TARGET_WIDTH{1920};
    const int TARGET_HEIGHT{1080};
};
