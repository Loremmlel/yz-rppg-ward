#include "WebSocketClient.h"
#include "ConfigService.h"
#include <QDebug>
#include <QUrlQuery>

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

void WebSocketClient::sendBinaryMessage(const QByteArray &data) const {
    if (!isConnected()) {
        qWarning() << "[WebSocketClient] 未连接，丢弃二进制帧，大小：" << data.size() << "字节";
        return;
    }
    m_socket->sendBinaryMessage(data);
}

void WebSocketClient::sendTextMessage(const QString &message) const {
    if (!isConnected()) {
        qWarning() << "[WebSocketClient] 未连接，丢弃文本消息";
        return;
    }
    m_socket->sendTextMessage(message);
}

void WebSocketClient::onConfigChanged(const AppConfig &config) {
    const bool changed = (m_host != config.serverHost
                          || m_port != config.serverPort
                          || m_bedId != config.bedId);
    m_host  = config.serverHost;
    m_port  = config.serverPort;
    m_bedId = config.bedId;

    if (changed) {
        connectToServer();
    }
}

void WebSocketClient::onConnected() {
    qDebug() << "[WebSocketClient] 已连接到" << m_socket->requestUrl().toString();
    m_reconnectTimer->stop();
    emit connected();
}

void WebSocketClient::onDisconnected() {
    qDebug() << "[WebSocketClient] 连接已断开";
    emit disconnected();

    if (!m_userDisconnected) {
        // 意外掉线，启动重连计时
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
        connectToServer();
    }
}

QUrl WebSocketClient::buildUrl() const {
    QUrl url;
    url.setScheme(QStringLiteral("ws"));
    url.setHost(m_host);
    url.setPort(m_port);
    url.setPath(QStringLiteral("/ws"));

    if (m_bedId > 0) {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("bedId"), QString::number(m_bedId));
        url.setQuery(query);
    }
    return url;
}

