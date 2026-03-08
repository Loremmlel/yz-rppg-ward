#pragma once

#include <QWebSocket>
#include <QTimer>
#include <QUrl>
#include "../model/AppConfig.h"

/**
 * @brief 底层 WebSocket 客户端
 *
 * 封装连接生命周期管理与断线自动重连，对上层屏蔽网络细节。
 * FrameUploadService 通过它推送图像帧，MetricsService 通过它接收指标 JSON。
 *
 * 线程模型：应 moveToThread 到专属线程。外部通过 QueuedConnection
 * 调用 sendBinaryMessage/sendTextMessage，无需额外加锁。
 */
class WebSocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebSocketClient(QObject *parent = nullptr);

    ~WebSocketClient() override;

    [[nodiscard]] bool isConnected() const;

public
    slots:
    /** 建立连接；若已连接则先断开再重连（幂等）。 */
    

    void connectToServer();

    /** 主动断开，不触发重连。 */
    void disconnectFromServer();

    void sendBinaryMessage(const QByteArray &data) const;

    void sendTextMessage(const QString &message) const;

    /** 配置变更时更新目标地址并重连。 */
    void onConfigChanged(const AppConfig &config);

    signals:
    

    void connected();

    void disconnected();

    /** 服务器推送的文本帧，通常是 JSON。 */
    void textMessageReceived(const QString &message);

    void binaryMessageReceived(const QByteArray &data);

    void errorOccurred(const QString &errorString);

private
    slots:
    

    void onConnected();

    void onDisconnected();

    void onTextMessageReceived(const QString &message);

    void onBinaryMessageReceived(const QByteArray &data);

    void onError(QAbstractSocket::SocketError error);

    void attemptReconnect();

private:
    [[nodiscard]] QUrl buildUrl() const;

    void logDropped(const QString &what) const; ///< 限速日志：未连接时丢弃消息

    QWebSocket *m_socket{nullptr};
    QTimer *m_reconnectTimer{nullptr};

    QString m_host;
    quint16 m_port{0};
    qint64 m_bedId{-1}; ///< 绑定的床位 ID，附加到 WS URL query
    mutable int m_dropLogSuppressed{0}; ///< 被抑制的丢弃日志计数
    mutable qint64 m_lastDropLogMs{0}; ///< 上次打印丢弃日志的时间戳（ms）
    static constexpr int k_dropLogIntervalMs{5000}; ///< 丢弃日志最小打印间隔

    bool m_userDisconnected{false}; ///< 区分主动断开与意外掉线，避免主动断开后触发重连
    bool m_connectingInProgress{false}; ///< 防止连接尝试期间重复 open()
    int m_reconnectAttempts{0}; ///< 连续重连次数，用于指数退避
    static constexpr int k_reconnectBaseMs{2000}; ///< 基础重连间隔（毫秒）
    static constexpr int k_reconnectMaxMs{60000}; ///< 最大重连间隔（毫秒）
};