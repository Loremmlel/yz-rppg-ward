#include "NetworkService.h"
#include "WebSocketClient.h"
#include <QBuffer>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QPixmap>

NetworkService::NetworkService(WebSocketClient *wsClient, QObject *parent)
    : QObject(parent), m_wsClient(wsClient) {
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
    QBuffer buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);
    if (!faceRoi.save(&buffer, "JPEG", m_jpegQuality)) {
        qWarning() << "[NetworkService] 图像 JPEG 编码失败";
        return;
    }

    // 帧格式：[8 字节大端 int64 毫秒时间戳] + [JPEG 数据]
    // 服务端读取前 8 字节即可还原采集时刻，用于 rPPG 时序对齐
    const qint64 timestampMs = QDateTime::currentMSecsSinceEpoch();
    QByteArray frame;
    frame.reserve(static_cast<qsizetype>(sizeof(qint64)) + jpegData.size());
    QDataStream ds(&frame, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << timestampMs;
    frame.append(jpegData);

    // WebSocketClient 在独立线程中运行，必须通过 QueuedConnection 跨线程传递
    QMetaObject::invokeMethod(m_wsClient, "sendBinaryMessage",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, frame));

    qDebug() << "[NetworkService] 已发送帧，时间戳:" << timestampMs
            << "ms，帧大小:" << frame.size() << "字节";
}
