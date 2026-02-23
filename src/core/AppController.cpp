#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_vitalService(std::make_unique<VitalService>(this)),
      m_videoService(std::make_unique<VideoService>(this)),
      m_networkService(std::make_unique<NetworkService>(this)),
      m_mainWindow(std::make_unique<MainWindow>()) {
    // 体征数据流向UI
    connect(m_vitalService.get(), &VitalService::dataUpdated,
            m_mainWindow->getVitalsWidget(), &VitalsWidget::updateData);

    // 视频帧从UI流向VideoService
    connect(m_mainWindow->getVideoWidget(), &VideoWidget::frameCaptured,
            m_videoService.get(), &VideoService::processFrame, Qt::QueuedConnection);

    // 人脸检测结果从VideoService流回UI
    connect(m_videoService.get(), &VideoService::facePositionUpdated,
            m_mainWindow->getVideoWidget(), &VideoWidget::updateFaceDetection);

    // 裁剪出的人脸特征流向NetworkService
    connect(m_videoService.get(), &VideoService::faceRoiExtracted,
            m_networkService.get(), &NetworkService::sendFaceRoiStream);
}

AppController::~AppController() = default;

void AppController::start() {
    // 启动体征采集流水线
    m_vitalService->startCollection();

    // 显示界面
    m_mainWindow->show();
}
