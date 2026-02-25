#include "WebSocketClient.h"
#include "ConfigService.h"
#include <QDebug>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent),
      m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this)),
      m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->setInterval(m_reconnectInterval);

    connect(m_socket, &QWebSocket::connected,    this, &WebSocketClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived,
            this, &WebSocketClient::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived,
            this, &WebSocketClient::onBinaryMessageReceived);
    connect(m_socket, &QWebSocket::errorOccurred,
            this, &WebSocketClient::onError);
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &WebSocketClient::attemptReconnect);

    // 读取初始配置
    const AppConfig cfg = ConfigService::instance()->config();
    onConfigChanged(cfg);

    connect(ConfigService::instance(), &ConfigService::configChanged,
            this, &WebSocketClient::onConfigChanged);
}

WebSocketClient::~WebSocketClient() {
    m_userDisconnected = true;
    m_reconnectTimer->stop();
    m_socket->close();
}

// ─── 公有接口 ────────────────────────────────────────────────────────────────

bool WebSocketClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void WebSocketClient::connectToServer() {
    if (m_host.isEmpty() || m_port == 0) {
        qWarning() << "[WebSocketClient] 服务器地址未配置，跳过连接";
        return;
    }
    m_userDisconnected = false;
    m_reconnectTimer->stop();

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->close();
    }
    qDebug() << "[WebSocketClient] 正在连接" << buildUrl().toString();
    m_socket->open(buildUrl());
}

void WebSocketClient::disconnectFromServer() {
    m_userDisconnected = true;
    m_reconnectTimer->stop();
    m_socket->close();
}

void WebSocketClient::sendBinaryMessage(const QByteArray &data) {
    if (!isConnected()) {
        qWarning() << "[WebSocketClient] 未连接，丢弃二进制帧（大小：" << data.size() << "字节）";
        return;
    }
    m_socket->sendBinaryMessage(data);
}

void WebSocketClient::sendTextMessage(const QString &message) {
    if (!isConnected()) {
        qWarning() << "[WebSocketClient] 未连接，丢弃文本消息";
        return;
    }
    m_socket->sendTextMessage(message);
}

void WebSocketClient::onConfigChanged(const AppConfig &config) {
    const bool changed = (m_host != config.serverHost || m_port != config.serverPort);
    m_host = config.serverHost;
    m_port = config.serverPort;
    qDebug() << "[WebSocketClient] 配置已更新:" << m_host << ":" << m_port;

    if (changed) {
        // 配置变更，重新连接
        connectToServer();
    }
}

// ─── 私有槽 ──────────────────────────────────────────────────────────────────

void WebSocketClient::onConnected() {
    qDebug() << "[WebSocketClient] 已连接到" << m_socket->requestUrl().toString();
    m_reconnectTimer->stop();
    emit connected();
}

void WebSocketClient::onDisconnected() {
    qDebug() << "[WebSocketClient] 连接已断开";
    emit disconnected();

    if (!m_userDisconnected) {
        qDebug() << "[WebSocketClient] 将在" << m_reconnectInterval / 1000
                 << "秒后尝试重连...";
        m_reconnectTimer->start();
    }
}

void WebSocketClient::onTextMessageReceived(const QString &message) {
    emit textMessageReceived(message);
}

void WebSocketClient::onBinaryMessageReceived(const QByteArray &data) {
    emit binaryMessageReceived(data);
}

void WebSocketClient::onError(QAbstractSocket::SocketError /*error*/) {
    const QString errStr = m_socket->errorString();
    qWarning() << "[WebSocketClient] 连接错误:" << errStr;
    emit errorOccurred(errStr);
}

void WebSocketClient::attemptReconnect() {
    if (!m_userDisconnected) {
        qDebug() << "[WebSocketClient] 正在重连...";
        connectToServer();
    }
}

// ─── 私有辅助 ─────────────────────────────────────────────────────────────────

QUrl WebSocketClient::buildUrl() const {
    QUrl url;
    url.setScheme(QStringLiteral("ws"));
    url.setHost(m_host);
    url.setPort(m_port);
    url.setPath(QStringLiteral("/"));
    return url;
}

