#ifndef WARD_NETWORKSERVICE_H
#define WARD_NETWORKSERVICE_H

#include <QObject>
#include <QElapsedTimer>
#include "../model/AppConfig.h"

class WebSocketClient;

/**
 * @brief 视频帧网络发送服务
 *
 * 将 VideoService 裁剪好的人脸 ROI 编码为 JPEG，通过 WebSocketClient 推送给服务器。
 * 内置帧率节流（30 fps），防止发送队列在 VideoService 高频产出时堆积。
 */
class NetworkService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param wsClient 注入的 WebSocketClient，生命周期由 AppController 管理
     */
    explicit NetworkService(WebSocketClient *wsClient, QObject *parent = nullptr);

public slots:
    void sendFaceRoiStream(const QImage &faceRoi);

private:
    WebSocketClient *m_wsClient     {nullptr}; ///< 不持有所有权

    QElapsedTimer m_fpsTimer;
    static constexpr int TARGET_FPS        = 30;
    static constexpr int FRAME_INTERVAL_MS = 1000 / TARGET_FPS;

    int m_jpegQuality {75}; ///< 质量与带宽的经验折中值
};

#endif // WARD_NETWORKSERVICE_H
