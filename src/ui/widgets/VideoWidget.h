#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaCaptureSession>


class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

private:
    QVideoWidget* m_videoOutput;
    QCamera* m_camera;
    QMediaCaptureSession* m_captureSession;
};