#include "NetworkService.h"
#include "WebSocketClient.h"
#include <QBuffer>
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
    if (m_fpsTimer.elapsed() < FRAME_INTERVAL_MS) {
        return;
    }
    m_fpsTimer.restart();

    // 调试预览：首次调用时创建独立窗口，后续帧复用同一实例
#ifdef QT_DEBUG
    static QLabel *previewLabel = [] {
        auto *label = new QLabel();
        label->setWindowTitle("DEBUG: Face ROI Stream");
        label->setFixedSize(256, 256);
        label->setAlignment(Qt::AlignCenter);
        label->show();
        return label;
    }();
    previewLabel->setPixmap(QPixmap::fromImage(faceRoi));
#endif

    QByteArray jpegData;
    QBuffer    buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);
    if (!faceRoi.save(&buffer, "JPEG", m_jpegQuality)) {
        qWarning() << "[NetworkService] 图像 JPEG 编码失败";
        return;
    }

    // WebSocketClient 在独立线程中运行，必须通过 QueuedConnection 跨线程传递
    QMetaObject::invokeMethod(m_wsClient, "sendBinaryMessage",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, jpegData));

    qDebug() << "[NetworkService] 已发送帧，大小:" << jpegData.size() << "字节";
}
