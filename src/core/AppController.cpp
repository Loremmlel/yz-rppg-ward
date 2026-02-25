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
    // ── 线程安排 ──────────────────────────────────────────────────────────────
    m_videoService->moveToThread(m_videoThread.get());
    m_wsClient->moveToThread(m_wsThread.get());
    m_videoThread->start();
    m_wsThread->start();

    // ── 上行：视频帧 → 人脸检测 → 编码 → WebSocket 发送 ─────────────────────
    connect(m_mainWindow->getVideoWidget(), &VideoWidget::frameCaptured,
            m_videoService.get(), &VideoService::processFrame, Qt::QueuedConnection);

    connect(m_videoService.get(), &VideoService::facePositionUpdated,
            m_mainWindow->getVideoWidget(), &VideoWidget::updateFaceDetection);

    connect(m_videoService.get(), &VideoService::faceRoiExtracted,
            m_networkService.get(), &NetworkService::sendFaceRoiStream);

    // ── 下行：WebSocket → VitalService → UI ──────────────────────────────────
    // WebSocket 连接状态通知 VitalService，控制在线/降级模式切换
    connect(m_wsClient.get(), &WebSocketClient::connected,
            m_vitalService.get(), &VitalService::onWsConnected);
    connect(m_wsClient.get(), &WebSocketClient::disconnected,
            m_vitalService.get(), &VitalService::onWsDisconnected);

    // 服务器推送的 JSON 体征数据
    connect(m_wsClient.get(), &WebSocketClient::textMessageReceived,
            m_vitalService.get(), &VitalService::onServerMessage);

    // 体征数据流向 UI（在线数据或降级模拟均通过此信号）
    connect(m_vitalService.get(), &VitalService::dataUpdated,
            m_mainWindow->getVitalsWidget(), &VitalsWidget::updateData);

    // ── WebSocket 启动（在 wsThread 事件循环就绪后执行）────────────────────
    QMetaObject::invokeMethod(m_wsClient.get(), "connectToServer",
                              Qt::QueuedConnection);
}

AppController::~AppController() {
    auto stopThread = [](QThread *t) {
        if (t->isRunning()) {
            t->quit();
            t->wait();
        }
    };
    stopThread(m_videoThread.get());
    stopThread(m_wsThread.get());
}

void AppController::start() const {
    // 启动降级模拟（WebSocket 连接成功后会自动切换为在线模式）
    m_vitalService->startCollection();
    m_mainWindow->show();
}
