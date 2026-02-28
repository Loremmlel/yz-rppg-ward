#include "NetworkService.h"
#include "WebSocketClient.h"
#include <QDebug>

NetworkService::NetworkService(WebSocketClient *wsClient, QObject *parent)
    : QObject(parent), m_wsClient(wsClient) {
    m_fpsTimer.start();
}

void NetworkService::sendEncodedFrame(const QByteArray &frame) {
    if (m_fpsTimer.elapsed() < FRAME_INTERVAL_MS) {
        return;
    }
    m_fpsTimer.restart();

    // WebSocketClient 在独立线程中运行，必须通过 QueuedConnection 跨线程传递
    QMetaObject::invokeMethod(m_wsClient, "sendBinaryMessage",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, frame));

    qDebug() << "[NetworkService] 已发送帧，帧大小:" << frame.size() << "字节";
}
