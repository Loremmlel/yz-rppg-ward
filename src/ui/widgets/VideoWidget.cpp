#include "VideoWidget.h"
#include <QMediaDevices>
#include <QLabel>
#include <QCameraDevice>
#include <QVideoSink>
#include <QDir>

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

void VideoWidget::processVideoFrame(const QVideoFrame &frame) {
    if (!frame.isValid() || m_faceDetector.empty()) {
        return;
    }

    // Qt视频帧转换为QImage
    auto image = frame.toImage();
    if (image.isNull()) {
        return;
    }

    // 转换为RGB888
    image = image.convertToFormat(QImage::Format_RGB888);

    // 转换为OpenCV Mat
    cv::Mat mat(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());
    cv::Mat bgrMat;
    cv::cvtColor(mat, bgrMat, cv::COLOR_RGB2BGR);

    if (bgrMat.cols > TARGET_WIDTH) {
        cv::resize(bgrMat, bgrMat, cv::Size(TARGET_WIDTH, bgrMat.rows * TARGET_WIDTH / bgrMat.cols));
    }

    // 人脸检测
    m_faceDetector->setInputSize(cv::Size(bgrMat.cols, bgrMat.rows));
    cv::Mat faces;
    m_faceDetector->detect(bgrMat, faces);

    bool hasFace = faces.rows > 0;
    // 在图像上画框
    if (hasFace) {
        for (int i = 0; i < faces.rows; i++) {
            int x = static_cast<int>(faces.at<float>(i, 0));
            int y = static_cast<int>(faces.at<float>(i, 1));
            int w = static_cast<int>(faces.at<float>(i, 2));
            int h = static_cast<int>(faces.at<float>(i, 3));

            // 确保框在图像范围内
            x = std::max(0, x);
            y = std::max(0, y);
            w = std::min(w, bgrMat.cols - x);
            h = std::min(h, bgrMat.rows - y);

            // 画绿色矩形框
            cv::rectangle(bgrMat, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 0), 2);
        }
    }

    // 转换回RGB
    cv::cvtColor(bgrMat, bgrMat, cv::COLOR_BGR2RGB);
    QImage finalImage(bgrMat.data, bgrMat.cols, bgrMat.rows, bgrMat.step, QImage::Format_RGB888);
    // 深拷贝一份确保内存安全
    auto pixmap = QPixmap::fromImage(finalImage.copy());

    // 更新UI
    m_displayLabel->setPixmap(pixmap.scaled(m_displayLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    // 显示警告
    m_warningLabel->setVisible(!hasFace);
    if (!hasFace) {
        m_warningLabel->resize(m_displayLabel->width(), 40);
        m_warningLabel->move(0, 0);
    }

    // TODO: 网络传输
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