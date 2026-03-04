#include "VideoService.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QDataStream>
#include <QThreadPool>
#include <QtConcurrentRun>
#include <QVideoFrame>
#include <opencv2/imgcodecs.hpp>

#include "../util/ImageHelper.h"

VideoService::VideoService(QObject *parent) : QObject(parent) {
    const auto path = loadModel("face_detection_yunet_2023mar.onnx");
    if (path.isEmpty()) {
        qWarning() << "人脸检测模型加载失败";
    }
    try {
        m_faceDetector = cv::FaceDetectorYN::create(
            path.toStdString(), "", cv::Size(640, 480),
            0.6f, // 置信度阈值
            0.3f, // NMS 阈值
            5000 // 最大检测数量上限
        );
    } catch (const cv::Exception &e) {
        qWarning() << "人脸检测模型初始化失败：" << e.what();
    }

    m_statsTimer.setInterval(STATS_INTERVAL_MS);
    connect(&m_statsTimer, &QTimer::timeout, this, &VideoService::printStats);
    m_statsTimer.start();
}

VideoService::~VideoService() {
    m_statsTimer.stop();
    if (m_processingFuture.isRunning()) {
        m_processingFuture.waitForFinished();
    }
}

void VideoService::processFrame(const QVideoFrame &videoFrame) {
    if (!videoFrame.isValid()) return;

    // 摄像头采集时间戳：帧进入管线的第一时间记录
    const qint64 captureTimestampMs = QDateTime::currentMSecsSinceEpoch();

    // ── 统计：记录本帧到达 ────────────────────────────────────────────────
    m_statReceived++;
    if (m_statFirstTs == 0) m_statFirstTs = captureTimestampMs;
    m_statLastTs = captureTimestampMs;

    // QVideoFrame::toImage() 耗时（YUV→RGB 约 8-15ms）移到此处（工作线程），主线程零负担
    const QImage image = videoFrame.toImage();
    if (image.isNull()) return;

    // ── 高频管线：每帧都执行，用卡尔曼滤波平滑人脸位置 ──────────────────────
    if (m_hasFace && m_hasKalman) {
        // 跳帧时用上一次原始观测值做 update（hold + smooth）
        const QRect kfRect(
            static_cast<int>(std::round(m_kfX.update(m_rawFaceRect.x()))),
            static_cast<int>(std::round(m_kfY.update(m_rawFaceRect.y()))),
            std::max(1, static_cast<int>(std::round(m_kfW.update(m_rawFaceRect.width())))),
            std::max(1, static_cast<int>(std::round(m_kfH.update(m_rawFaceRect.height()))))
        );

        // 死区：仅当滤波框中心移动超过当前框尺寸的 2% 时才更新，抑制微抖
        const auto oldCenter = m_currentFaceRect.center();
        const auto newCenter = kfRect.center();
        const double threshold = 0.02 * std::max(m_currentFaceRect.width(),
                                                 m_currentFaceRect.height());
        if (!m_currentFaceRect.isValid() ||
            std::abs(newCenter.x() - oldCenter.x()) > threshold ||
            std::abs(newCenter.y() - oldCenter.y()) > threshold) {
            m_currentFaceRect = kfRect;
        }

        emit facePositionUpdated(m_currentFaceRect, true);

        const QRect clipped = m_currentFaceRect.intersected(image.rect());
        if (clipped.isValid() && !m_isEncoding.load()) {
            m_isEncoding.store(true);
            m_statEncoded++;
            const QImage roi = image.copy(clipped);
            QThreadPool::globalInstance()->start([this, roi, captureTimestampMs] {
                if (const auto frame = encodeRoi(roi, captureTimestampMs);
                    !frame.isEmpty()) {
                    emit faceRoiEncoded(frame);
                }
                m_isEncoding.store(false);
            });
        } else if (clipped.isValid()) {
            // ROI 有效，但编码线程正忙
            m_statDropEncBusy++;
        }
    } else {
        // 尚未检测到人脸，无法裁剪 ROI
        m_statDropNoFace++;
    }

    // ── 低频管线：每 5 帧触发一次异步检测，防止吃满 CPU ──────────────────────
    m_frameSkipCounter++;
    if (m_frameSkipCounter % 5 == 0 && !m_faceDetector.empty() && !m_isProcessing.load()) {
        m_isProcessing.store(true);
        const auto mat = ImageHelper::QImage2CvMat(image);
        m_processingFuture = QtConcurrent::run([this, mat] { detectWorker(mat); });
    }
}

void VideoService::detectWorker(const cv::Mat &mat) {
    double scale;
    cv::Mat detectionMat;

    // 缩放到 640px 宽度加速推理；检测结果坐标后续需等比放回原图尺度
    if (constexpr int targetDetWidth = 640; mat.cols > targetDetWidth) {
        scale = static_cast<double>(mat.cols) / targetDetWidth;
        cv::resize(mat, detectionMat,
                   cv::Size(targetDetWidth, static_cast<int>(mat.rows / scale)));
    } else {
        detectionMat = mat;
        scale = 1.0;
    }

    m_faceDetector->setInputSize(detectionMat.size());
    cv::Mat faces;
    m_faceDetector->detect(detectionMat, faces);

    QRect newRect;
    bool hasFace = false;
    if (faces.rows > 0) {
        // 多张人脸时取面积最大的，假设目标患者是画面主体
        hasFace = true;
        int maxArea = 0, maxIndex = 0;
        for (int i = 0; i < faces.rows; i++) {
            const int w = static_cast<int>(faces.at<float>(i, 2) * scale);
            const int h = static_cast<int>(faces.at<float>(i, 3) * scale);
            if (w * h > maxArea) {
                maxArea = w * h;
                maxIndex = i;
            }
        }

        const int x = static_cast<int>(faces.at<float>(maxIndex, 0) * scale);
        const int y = static_cast<int>(faces.at<float>(maxIndex, 1) * scale);
        const int w = static_cast<int>(faces.at<float>(maxIndex, 2) * scale);
        const int h = static_cast<int>(faces.at<float>(maxIndex, 3) * scale);
        newRect = QRect(x, y, w, h);
    }

    // 检测在工作线程完成，回到主线程写状态并发信号，避免竞态
    QMetaObject::invokeMethod(this, [this, newRect, hasFace] {
        if (hasFace) {
            if (!m_hasKalman) {
                // 首次检测到人脸：用原始观测值初始化滤波器
                m_kfX.reset(newRect.x());
                m_kfY.reset(newRect.y());
                m_kfW.reset(newRect.width());
                m_kfH.reset(newRect.height());
                m_currentFaceRect = newRect;
                m_hasKalman = true;
            } else {
                // 后续检测帧：用真实观测更新滤波器
                m_kfX.update(newRect.x());
                m_kfY.update(newRect.y());
                m_kfW.update(newRect.width());
                m_kfH.update(newRect.height());
            }
            m_rawFaceRect = newRect;
        } else {
            m_hasKalman = false;
        }

        m_hasFace = hasFace;

        if (!hasFace) {
            emit facePositionUpdated(newRect, false);
        }
        m_isProcessing.store(false);
    }, Qt::QueuedConnection);
}

QByteArray VideoService::encodeRoi(const QImage &roi, const qint64 captureTimestampMs) {
    const cv::Mat mat = ImageHelper::QImage2CvMat(roi);
    std::vector<uchar> imageBuf;

    if (const std::vector params = {cv::IMWRITE_WEBP_QUALITY, 90};
        !cv::imencode(".webp", mat, imageBuf, params)) {
        qWarning() << "[VideoService] 图像编码失败";
        return {};
    }

    QByteArray frame;
    frame.reserve(static_cast<qsizetype>(sizeof(qint64) + imageBuf.size()));
    QDataStream ds(&frame, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << captureTimestampMs;
    frame.append(reinterpret_cast<const char *>(imageBuf.data()),
                 static_cast<qsizetype>(imageBuf.size()));
    return frame;
}

QString VideoService::loadModel(const QString &modelName) {
    QString localPath = QDir::currentPath() + QDir::separator() + modelName;

    if (QFile::exists(localPath)) {
        return localPath;
    }

    // 首次运行时从资源包解压到本地，后续直接复用
    if (QFile(":/models/" + modelName).copy(localPath)) {
        return localPath;
    }

    return {};
}

void VideoService::printStats() {
    if (m_statReceived == 0) return; // 无帧到达，保持静默

    const double captureFps = (STATS_INTERVAL_MS > 0)
        ? m_statReceived * 1000.0 / STATS_INTERVAL_MS
        : 0.0;
    const double encodeFps = (STATS_INTERVAL_MS > 0)
        ? m_statEncoded * 1000.0 / STATS_INTERVAL_MS
        : 0.0;

    QString tsRange;
    if (m_statFirstTs > 0) {
        tsRange = QString("[%1 → %2, 跨度 %3ms]")
            .arg(QDateTime::fromMSecsSinceEpoch(m_statFirstTs).toString("hh:mm:ss.zzz"))
            .arg(QDateTime::fromMSecsSinceEpoch(m_statLastTs).toString("hh:mm:ss.zzz"))
            .arg(m_statLastTs - m_statFirstTs);
    }

    qDebug().noquote() << QString(
        "[VideoService] 近 %1s | 采集 %2 (%3fps) | 编码 %4 (%5fps) | "
        "无人脸丢弃 %6 | 编码忙丢弃 %7 | %8"
    ).arg(STATS_INTERVAL_MS / 1000)
     .arg(m_statReceived)
     .arg(QString::number(captureFps, 'f', 1))
     .arg(m_statEncoded)
     .arg(QString::number(encodeFps, 'f', 1))
     .arg(m_statDropNoFace)
     .arg(m_statDropEncBusy)
     .arg(tsRange);

    m_statReceived    = 0;
    m_statEncoded     = 0;
    m_statDropNoFace  = 0;
    m_statDropEncBusy = 0;
    m_statFirstTs     = 0;
    m_statLastTs      = 0;
}
