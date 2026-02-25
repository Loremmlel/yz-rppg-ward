#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QUrl>
#include "../model/AppConfig.h"

/**
 * @brief 底层 WebSocket 客户端封装
 *
 * 职责：
 *  - 管理 QWebSocket 生命周期（连接、断开、重连）
 *  - 提供线程安全的二进制/文本发送接口（通过信号槽 QueuedConnection）
 *  - 将服务器推送的文本消息向上广播
 *  - 监听 ConfigService::configChanged，动态切换服务器地址
 *
 * 使用方：
 *  - NetworkService：调用 sendBinaryMessage() 推送图像帧
 *  - VitalService：监听 textMessageReceived() 接收体征 JSON
 *
 * 线程模型：
 *  本对象应 moveToThread 到专属 QThread，所有槽函数在该线程执行。
 *  外部调用 sendBinaryMessage/sendTextMessage 通过 QueuedConnection 排队，无需加锁。
 */
class WebSocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebSocketClient(QObject *parent = nullptr);
    ~WebSocketClient() override;

    /** 当前是否已成功连接到服务器 */
    bool isConnected() const;

public slots:
    /** 根据当前配置建立 WebSocket 连接（幂等：已连接则先断开再重连） */
    void connectToServer();

    /** 主动断开连接，不触发重连 */
    void disconnectFromServer();

    /** 发送二进制帧（线程安全，可从任意线程通过 QueuedConnection 调用） */
    void sendBinaryMessage(const QByteArray &data);

    /** 发送文本帧（线程安全，可从任意线程通过 QueuedConnection 调用） */
    void sendTextMessage(const QString &message);

    /** 响应配置变更：更新服务器地址并重连 */
    void onConfigChanged(const AppConfig &config);

signals:
    /** WebSocket 已成功连接 */
    void connected();

    /** WebSocket 连接已断开 */
    void disconnected();

    /** 收到服务器推送的文本消息（通常是 JSON） */
    void textMessageReceived(const QString &message);

    /** 收到服务器推送的二进制消息 */
    void binaryMessageReceived(const QByteArray &data);

    /** 连接发生错误 */
    void errorOccurred(const QString &errorString);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &data);
    void onError(QAbstractSocket::SocketError error);
    void attemptReconnect();

private:
    QUrl buildUrl() const;

    QWebSocket  *m_socket   {nullptr};
    QTimer      *m_reconnectTimer {nullptr};

    QString  m_host;
    quint16  m_port         {0};

    bool     m_userDisconnected {false}; ///< 用于区分主动断开与意外断开
    int      m_reconnectInterval {5000}; ///< 重连间隔（毫秒）
};

