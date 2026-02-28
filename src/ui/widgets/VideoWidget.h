#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>

#include "../../service/VideoService.h"


/**
 * @brief 摄像头视频预览组件
 *
 * 负责摄像头初始化、帧采集与画面渲染。
 * 每帧通过 frameCaptured 向外发射原始图像，由 VideoService 做进一步处理。
 * 人脸检测结果由外部回调 updateFaceDetection 注入，组件不持有检测逻辑。
 */
class VideoWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget() override;

signals:
    /** 摄像头每帧原始图像，发往 VideoService 做人脸检测与 ROI 裁剪。 */
    void frameCaptured(const QImage &image);

public slots:
    void updateFaceDetection(const QRect &rect, bool hasFace);

    /** 床位绑定状态变化时调用，控制"未绑定床位"提示的显隐。 */
    void setBedBound(bool bound);

private slots:
    void processVideoFrame(const QVideoFrame &frame);

private:
    /**
     * @brief 从设备支持列表中匹配 1920×1080 @ 25-30fps 的格式
     *
     * 优先使用硬件原生格式，避免软件缩放带来的额外 CPU 开销。
     */
    void setupCameraFormat() const;

    QCamera              *m_camera          {nullptr};
    QMediaCaptureSession *m_captureSession  {nullptr};
    QVideoSink           *m_videoSink       {nullptr};
    QLabel               *m_displayLabel    {nullptr};
    QLabel               *m_warningLabel    {nullptr}; ///< 未检测到人脸时的覆盖提示
    QLabel               *m_noBedLabel      {nullptr}; ///< 未绑定床位时的覆盖提示

    QRect m_currentFaceRect;
    bool  m_hasFace       {false};

    const int TARGET_WIDTH  {1920};
    const int TARGET_HEIGHT {1080};
};
