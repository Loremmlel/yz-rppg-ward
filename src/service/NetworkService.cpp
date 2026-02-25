#include "NetworkService.h"
#include "WebSocketClient.h"
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QPixmap>

NetworkService::NetworkService(WebSocketClient *wsClient, QObject *parent)
    : QObject(parent), m_wsClient(wsClient)
{
    m_fpsTimer.start();
}

void NetworkService::sendFaceRoiStream(const QImage &faceRoi) {
    // ---- 帧率节流（30 fps）----
    if (m_fpsTimer.elapsed() < FRAME_INTERVAL_MS) {
        return;
    }
    m_fpsTimer.restart();

    // ---- 调试预览窗口（静态，首次调用时创建，后续复用）----
    static QLabel *previewLabel = [] {
        auto *label = new QLabel();
        label->setWindowTitle("DEBUG: Face ROI Stream");
        label->setFixedSize(256, 256);
        label->setAlignment(Qt::AlignCenter);
        label->show();
        return label;
    }();
    previewLabel->setPixmap(QPixmap::fromImage(faceRoi));
    // -------------------------------------------------------

    // ---- 编码为 JPEG 并通过 WebSocket 发送 ----
    QByteArray jpegData;
    QBuffer    buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);
    if (!faceRoi.save(&buffer, "JPEG", m_jpegQuality)) {
        qWarning() << "[NetworkService] 图像 JPEG 编码失败";
        return;
    }

    // sendBinaryMessage 通过 QueuedConnection 安全跨线程调用
    QMetaObject::invokeMethod(m_wsClient, "sendBinaryMessage",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, jpegData));

    qDebug() << "[NetworkService] 已发送帧，大小:" << jpegData.size() << "字节";
}
