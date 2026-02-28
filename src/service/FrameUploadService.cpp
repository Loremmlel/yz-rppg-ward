#include "FrameUploadService.h"
#include "WebSocketClient.h"
#include "ConfigService.h"
#include <QDebug>

#ifdef QT_DEBUG
#include <QLabel>
#include <QPixmap>
#include <opencv2/imgcodecs.hpp>
#endif

FrameUploadService::FrameUploadService(WebSocketClient *wsClient, QObject *parent)
    : QObject(parent), m_wsClient(wsClient),
      m_bedBound(ConfigService::instance()->config().hasBed()) {
    m_fpsTimer.start();

    connect(ConfigService::instance(), &ConfigService::configChanged,
            this, &FrameUploadService::onConfigChanged);
}

void FrameUploadService::onConfigChanged(const AppConfig &config) {
    m_bedBound = config.hasBed();
}

void FrameUploadService::sendEncodedFrame(const QByteArray &frame) {
    if (!m_bedBound) {
        return; // 未绑定床位，丢弃帧
    }

    if (m_fpsTimer.elapsed() < FRAME_INTERVAL_MS) {
        return;
    }
    m_fpsTimer.restart();

    // 调试预览：将已编码的 WebP 解码后显示在独立窗口
#ifdef QT_DEBUG
    static QLabel *previewLabel = [] {
        auto *label = new QLabel();
        label->setWindowTitle("DEBUG: Face ROI Stream");
        label->setAlignment(Qt::AlignCenter);
        label->show();
        return label;
    }();
    // 跳过前 8 字节时间戳，剩余为 WebP 数据
    const QByteArray webpData = frame.mid(static_cast<qsizetype>(sizeof(qint64)));
    const std::vector<uchar> buf(webpData.cbegin(), webpData.cend());
    if (const auto decoded = cv::imdecode(buf, cv::IMREAD_COLOR);
        !decoded.empty()) {
        const QImage img(decoded.data, decoded.cols, decoded.rows, decoded.step, QImage::Format_BGR888);
        previewLabel->setFixedSize(img.size());
        previewLabel->setPixmap(QPixmap::fromImage(img));
    }
#endif

    // WebSocketClient 在独立线程中运行，必须通过 QueuedConnection 跨线程传递
    QMetaObject::invokeMethod(m_wsClient, "sendBinaryMessage",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, frame));

    qDebug() << "[FrameUploadService] 已发送帧，帧大小:" << frame.size() << "字节";
}
