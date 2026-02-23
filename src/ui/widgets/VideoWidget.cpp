#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>
#include <QVideoSink>
#include <QDir>
#include <QtConcurrent/QtConcurrent>

#include "../../util/ImageHelper.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent){
    this->setObjectName("VideoWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto path = loadModel("face_detection_yunet_2023mar.onnx");
    if (path.isEmpty()) {
        qWarning() << "人脸检测模型加载失败";
    }
    try {
        m_faceDetector = cv::FaceDetectorYN::create(
            path.toStdString(), "", cv::Size(320, 320),
            0.6f,
            0.3f,
            5000
            );
    } catch (const cv::Exception& e) {
        qWarning() << "人脸检测模型初始化失败：" << e.what();
    }

    m_displayLabel = new QLabel(this);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setStyleSheet("background-color: black;");

    m_warningLabel = new QLabel("未检测到患者人脸", this);
    m_warningLabel->setAlignment(Qt::AlignCenter);
    m_warningLabel->setStyleSheet("background-color: rgba(255, 0, 0, 150); color: white; font-size: 16px; font-weight: bold; padding: 10px;");
    m_warningLabel->setVisible(false);
    m_warningLabel->setParent(m_displayLabel);

    layout->addWidget(m_displayLabel);

    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        m_displayLabel->setText("未检测到有效摄像头设备");
        m_displayLabel->setStyleSheet("color: #ff4d4f; font-size: 18px;");
        return;
    }

    m_camera = new QCamera(cameras.first(), this);
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setCamera(m_camera);

    setupCameraFormat();

    m_videoSink = new QVideoSink(this);
    m_captureSession->setVideoSink(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &VideoWidget::processVideoFrame);

    m_camera->start();
}

VideoWidget::~VideoWidget() {
    if (m_processingFuture.isRunning()) {
        m_processingFuture.waitForFinished();
    }
}

void VideoWidget::processVideoFrame(const QVideoFrame &frame) {
    // 帧丢弃
    if (m_isProcessing.load()) {
        return;
    }

    // 每两帧处理一次，降低 CPU 占用
    m_frameSkipCounter++;
    if (m_frameSkipCounter % 3 != 0) {
        return;
    }

    if (!frame.isValid() || m_faceDetector.empty()) {
        return;
    }

    // Qt视频帧转换为QImage
    auto image = frame.toImage();
    if (image.isNull()) {
        return;
    }

    m_isProcessing.store(true);

    m_processingFuture = QtConcurrent::run([this, image] {
          processFrameBackend(image);
    });
}

void VideoWidget::processFrameBackend(QImage image) {
    // 1. 【关键优化】高效的 QImage -> Mat 转换
    cv::Mat mat = ImageHelper::QImage2CvMat(image);

    // 2. 【关键优化】降低检测分辨率
    // 人脸检测不需要 720P 或 1080P，缩小到 640 宽度即可大幅降低 CPU 占用
    double scale = 1.0;
    cv::Mat detectionMat;
    int targetDetWidth = 640;

    if (mat.cols > targetDetWidth) {
        scale = static_cast<double>(mat.cols) / targetDetWidth;
        cv::resize(mat, detectionMat, cv::Size(targetDetWidth, static_cast<int>(mat.rows / scale)));
    } else {
        detectionMat = mat;
    }

    // 3. 人脸检测
    // 注意：这里传入的是缩小后的 Mat
    m_faceDetector->setInputSize(detectionMat.size());
    cv::Mat faces;
    m_faceDetector->detect(detectionMat, faces);

    // 4. 绘制
    // 如果为了性能，可以在小图上画，然后放大回大图；或者把坐标放大后画回原图。
    // 这里演示画回原图- 保持高清晰度显示

    bool hasFace = faces.rows > 0;

    // 必须锁一下模型或者确保模型是线程安全的，OpenCV DNN 推理通常不建议多线程并发调用同一个实例
    // 但由于加了 m_isProcessing 锁，这里同一时间只有一个线程在用 m_faceDetector，所以是安全的。

    if (hasFace) {
        for (int i = 0; i < faces.rows; i++) {
            // 检测结果的坐标是基于缩小后的图，需要映射回原图
            int x = static_cast<int>(faces.at<float>(i, 0) * scale);
            int y = static_cast<int>(faces.at<float>(i, 1) * scale);
            int w = static_cast<int>(faces.at<float>(i, 2) * scale);
            int h = static_cast<int>(faces.at<float>(i, 3) * scale);

            // 边界检查
            x = std::max(0, x);
            y = std::max(0, y);
            w = std::min(w, mat.cols - x);
            h = std::min(h, mat.rows - y);

            cv::rectangle(mat, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 0), 2);
        }
    }

    // 5. 转回 QImage 显示
    cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
    // 这里必须深拷贝，因为 bgrMat 是局部变量，函数结束就销毁了
    QImage finalImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(finalImage.copy());

    // 6. 更新 UI (必须切回主线程)
    QMetaObject::invokeMethod(this, [this, pixmap, hasFace]() {
        m_displayLabel->setPixmap(pixmap.scaled(m_displayLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation));
        m_warningLabel->setVisible(!hasFace);
        if (!hasFace) {
            m_warningLabel->resize(m_displayLabel->width(), 40);
            m_warningLabel->move(0, 0);
        }

        // 处理完成，解除锁定，允许接收新帧
        m_isProcessing.store(false);
    }, Qt::QueuedConnection);
}

void VideoWidget::setupCameraFormat() {
    QCameraFormat bestFormat;
    // 寻找最接近 720p 30fps 的硬件输出格式
    for (const auto& format : m_camera->cameraDevice().videoFormats()) {
        if (format.resolution() == QSize(TARGET_WIDTH, TARGET_HEIGHT) &&
            format.maxFrameRate() >= 25.0f && format.maxFrameRate() <= 30.0f) {
            bestFormat = format;
            break;
            }
    }

    if (!bestFormat.isNull()) {
        m_camera->setCameraFormat(bestFormat);
        qDebug() << "成功设置摄像头硬件格式为:" << bestFormat.resolution() << bestFormat.maxFrameRate() << "FPS";
    } else {
        qWarning() << "未能找到 720p@30FPS 硬件格式，将使用系统默认，并在后续进行软件降采样。";
    }
}

QString VideoWidget::loadModel(const QString &modelName) {
    const QString qrcPath = ":/models/" + modelName;
    QFile modelFile(qrcPath);
    QString localPath = QDir::currentPath() + QDir::separator() + modelName;

    if (QFile::exists(localPath)) {
        qDebug() << "人脸检测模型已存在：" << localPath;
    } else {
        if (modelFile.copy(localPath)) {
            qDebug() << "人脸检测模型复制到：" << localPath;
        } else {
            qDebug() << "人脸检测模型复制失败，从：" << qrcPath << "到" << localPath << "原因：" << modelFile.errorString();
            return "";
        }
    }
    return localPath;
}