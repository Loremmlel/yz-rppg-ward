#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_wsClient(std::make_unique<WebSocketClient>()),
      m_vitalService(std::make_unique<VitalService>(this)),
      m_videoService(std::make_unique<VideoService>()),
      m_networkService(std::make_unique<NetworkService>(m_wsClient.get(), this)),
      m_mainWindow(std::make_unique<MainWindow>()),
      m_videoThread(std::make_unique<QThread>(this)),
      m_wsThread(std::make_unique<QThread>(this)) {
    m_videoService->moveToThread(m_videoThread.get());
    // WebSocketClient 必须与其 QWebSocket 成员在同一线程，单独给它一个线程
    m_wsClient->moveToThread(m_wsThread.get());
    m_videoThread->start();
    m_wsThread->start();

    // ── 上行：视频帧 → 人脸检测 → JPEG 编码 → WebSocket ───────────────────────
    connect(m_mainWindow->getVideoWidget(), &VideoWidget::frameCaptured,
            m_videoService.get(), &VideoService::processFrame, Qt::QueuedConnection);

    connect(m_videoService.get(), &VideoService::facePositionUpdated,
            m_mainWindow->getVideoWidget(), &VideoWidget::updateFaceDetection);

    connect(m_videoService.get(), &VideoService::faceRoiExtracted,
            m_networkService.get(), &NetworkService::sendFaceRoiStream);

    // ── 下行：WebSocket → VitalService → UI ───────────────────────────────────
    connect(m_wsClient.get(), &WebSocketClient::connected,
            m_vitalService.get(), &VitalService::onWsConnected);
    connect(m_wsClient.get(), &WebSocketClient::disconnected,
            m_vitalService.get(), &VitalService::onWsDisconnected);
    connect(m_wsClient.get(), &WebSocketClient::textMessageReceived,
            m_vitalService.get(), &VitalService::onServerMessage);

    connect(m_vitalService.get(), &VitalService::dataUpdated,
            m_mainWindow->getVitalsWidget(), &VitalsWidget::updateData);

    // wsThread 启动后才调用 open()，避免在线程就绪前触发网络操作
    QMetaObject::invokeMethod(m_wsClient.get(), "connectToServer", Qt::QueuedConnection);
}

AppController::~AppController() {
    auto stopThread = [](QThread *t) {
        if (t->isRunning()) { t->quit(); t->wait(); }
    };
    stopThread(m_videoThread.get());
    stopThread(m_wsThread.get());
}

void AppController::start() const {
    // 以降级模式启动；WebSocket 连接成功后 VitalService 会自动切换到在线数据
    m_vitalService->startCollection();
    m_mainWindow->show();
}
