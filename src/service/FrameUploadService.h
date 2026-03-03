#pragma once
#include <QObject>
#include <QTimer>

#include "../model/AppConfig.h"

class WebSocketClient;

/**
 * @brief 图像帧上传服务
 *
 * 接收编码后的图像帧数据，通过 WebSocketClient 推送给服务器。
 * 未绑定床位时拒绝上传，避免无效数据发送。
 * 每 5 秒打印一次统计摘要（上传帧数、因无床位丢弃帧数）。
 */
class FrameUploadService : public QObject {
    Q_OBJECT

public:
    explicit FrameUploadService(WebSocketClient *wsClient, QObject *parent = nullptr);

public slots:
    /** 接收已编码的图像帧（8B 时间戳 + imageData），转发至 WebSocket。 */
    void sendEncodedFrame(const QByteArray &frame);

    /** 配置变更时更新床位绑定状态。 */
    void onConfigChanged(const AppConfig &config);

private slots:
    void printStats();

private:
    WebSocketClient *m_wsClient{nullptr}; ///< 不持有所有权
    bool m_bedBound{false};               ///< 是否已绑定有效床位

    // ── 统计计数器（每个统计周期重置）──────────────────────────────────────
    QTimer m_statsTimer;
    static constexpr int STATS_INTERVAL_MS = 5000;

    int m_statUploaded{0};   ///< 已上传帧数
    int m_statDropNoBed{0};  ///< 因未绑定床位丢弃帧数
};
