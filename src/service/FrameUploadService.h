#pragma once
#include <QObject>
#include <QElapsedTimer>

#include "../model/AppConfig.h"

class WebSocketClient;

/**
 * @brief 视频帧上传服务
 *
 * 接收 VideoService 已编码的帧数据，通过 WebSocketClient 推送给服务器。
 * 内置帧率节流（30 fps），防止发送队列在 VideoService 高频产出时堆积。
 * 未绑定床位时拒绝上传，避免无效数据发送。
 */
class FrameUploadService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param wsClient 注入的 WebSocketClient，生命周期由 AppController 管理
     */
    explicit FrameUploadService(WebSocketClient *wsClient, QObject *parent = nullptr);

public slots:
    /** 接收已编码的帧（8B 时间戳 + WebP），节流后转发至 WebSocket。 */
    void sendEncodedFrame(const QByteArray &frame);

    /** 配置变更时更新床位绑定状态。 */
    void onConfigChanged(const AppConfig &config);

private:
    WebSocketClient *m_wsClient     {nullptr}; ///< 不持有所有权
    bool m_bedBound                 {false};   ///< 是否已绑定有效床位

    QElapsedTimer m_fpsTimer;
    static constexpr int TARGET_FPS        = 30;
    static constexpr int FRAME_INTERVAL_MS = 1000 / TARGET_FPS;
};

