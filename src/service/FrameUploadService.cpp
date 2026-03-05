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
    : QObject(parent), m_wsClient(wsClient), m_statsTimer(new QTimer(this)),
      m_bedBound(ConfigService::instance()->config().hasBed()) {
    connect(ConfigService::instance(), &ConfigService::configChanged,
            this, &FrameUploadService::onConfigChanged);

    m_statsTimer->setInterval(STATS_INTERVAL_MS);
    connect(m_statsTimer, &QTimer::timeout, this, &FrameUploadService::printStats);
    m_statsTimer->start();
}

FrameUploadService::~FrameUploadService() {
    m_statsTimer->stop();
}

void FrameUploadService::onConfigChanged(const AppConfig &config) {
    m_bedBound = config.hasBed();
}

void FrameUploadService::sendEncodedFrame(const QByteArray &frame) {
    if (!m_bedBound) {
        m_statDropNoBed++;
        return;
    }

    // 调试预览：将已编码的图像数据解码后显示在独立窗口
#ifdef QT_DEBUG
    static QLabel *previewLabel = [] {
        auto *label = new QLabel();
        label->setWindowTitle("DEBUG: Face ROI Stream");
        label->setAlignment(Qt::AlignCenter);
        label->show();
        return label;
    }();
    // 跳过前 8 字节时间戳，剩余为 imageData
    const QByteArray imageData = frame.mid(static_cast<qsizetype>(sizeof(qint64)));
    const std::vector<uchar> buf(imageData.cbegin(), imageData.cend());
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
    m_statUploaded++;
}

void FrameUploadService::printStats() {
    const int arrived = m_statUploaded + m_statDropNoBed;
    if (arrived == 0) return;

    const double uploadFps = m_statUploaded * 1000.0 / STATS_INTERVAL_MS;

    qDebug().noquote() << QString(
        "[FrameUpload] 近 %1s | 到达 %2 | 上传 %3 (%4fps) | 无床位丢弃 %5"
    ).arg(STATS_INTERVAL_MS / 1000)
     .arg(arrived)
     .arg(m_statUploaded)
     .arg(QString::number(uploadFps, 'f', 1))
     .arg(m_statDropNoBed);

    m_statUploaded  = 0;
    m_statDropNoBed = 0;
}
