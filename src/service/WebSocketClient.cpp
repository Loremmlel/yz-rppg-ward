#include "WebSocketClient.h"
#include "ConfigService.h"
#include <QDebug>
#include <QUrlQuery>
#include <QDateTime>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent),
      m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this)),
      m_reconnectTimer(new QTimer(this)) {
    m_reconnectTimer->setSingleShot(true);

    connect(m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
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

    // 若已处于正在连接或已连接状态，则不重复发起 open()，避免内存泄漏
    const auto state = m_socket->state();
    if (state == QAbstractSocket::ConnectingState
        || state == QAbstractSocket::ConnectedState) {
        return;
    }

    if (state != QAbstractSocket::UnconnectedState) {
        m_socket->abort(); // 用 abort() 立即清理，不等待 close 握手
    }

    m_connectingInProgress = true;
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
        logDropped(QStringLiteral("二进制帧（%1 字节）").arg(data.size()));
        return;
    }
    m_socket->sendBinaryMessage(data);
}

void WebSocketClient::sendTextMessage(const QString &message) const {
    if (!isConnected()) {
        Q_UNUSED(message)
        logDropped(QStringLiteral("文本消息"));
        return;
    }
    m_socket->sendTextMessage(message);
}

void WebSocketClient::onConfigChanged(const AppConfig &config) {
    const bool changed = (m_host != config.serverHost
                          || m_port != config.serverPort
                          || m_bedId != config.bedId);
    m_host = config.serverHost;
    m_port = config.serverPort;
    m_bedId = config.bedId;

    if (changed) {
        connectToServer();
    }
}

void WebSocketClient::onConnected() {
    qDebug() << "[WebSocketClient] 已连接到" << m_socket->requestUrl().toString();
    m_connectingInProgress = false;
    m_reconnectAttempts = 0;
    m_reconnectTimer->stop();
    emit connected();
}

void WebSocketClient::onDisconnected() {
    m_connectingInProgress = false;
    qDebug() << "[WebSocketClient] 连接已断开";
    emit disconnected();

    if (!m_userDisconnected) {
        // 指数退避：2s, 4s, 8s, ... 最大 60s
        const int delayMs = qMin(k_reconnectBaseMs << m_reconnectAttempts, k_reconnectMaxMs);
        ++m_reconnectAttempts;
        qDebug() << "[WebSocketClient] 将在" << delayMs << "ms 后尝试重连（第"
                << m_reconnectAttempts << "次）";
        m_reconnectTimer->start(delayMs);
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
    m_connectingInProgress = false;
    // 不在此处启动重连，onDisconnected 会在 errorOccurred 后紧接着触发，
    // 由 onDisconnected 负责退避重连，避免双重触发。
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

void WebSocketClient::logDropped(const QString &what) const {
    ++m_dropLogSuppressed;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastDropLogMs >= k_dropLogIntervalMs) {
        if (m_dropLogSuppressed > 1) {
            qWarning() << "[WebSocketClient] 未连接，已丢弃" << m_dropLogSuppressed
                    << "条消息（最新：" << what << "）";
        } else {
            qWarning() << "[WebSocketClient] 未连接，丢弃" << what;
        }
        m_dropLogSuppressed = 0;
        m_lastDropLogMs = now;
    }
}