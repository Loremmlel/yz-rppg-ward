#include "AppController.h"
#include "../service/ConfigService.h"

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_wsClient(std::make_unique<WebSocketClient>()),
      m_metricsService(std::make_unique<MetricsService>(this)),
      m_videoService(std::make_unique<VideoService>()),
      m_frameUploadService(std::make_unique<FrameUploadService>(m_wsClient.get(), this)),
      m_mainWindow(std::make_unique<MainWindow>()),
      m_videoThread(std::make_unique<QThread>(this)),
      m_wsThread(std::make_unique<QThread>(this)) {
    m_videoService->moveToThread(m_videoThread.get());
    // WebSocketClient 必须与其 QWebSocket 成员在同一线程，单独给它一个线程
    m_wsClient->moveToThread(m_wsThread.get());
    m_videoThread->start();
    m_wsThread->start();

    // ── 上行：视频帧 → 人脸检测 → 编码（工作线程） → WebSocket ──────────
    // QVideoFrame 是引用计数类型，QueuedConnection 跨线程传递无深拷贝（原 QImage 需拷贝 ~8MB）
    // toImage()/YUV→RGB 转换在 VideoService 工作线程完成，主线程不参与任何图像处理
    connect(m_mainWindow->getVideoWidget(), &VideoWidget::frameCaptured,
            m_videoService.get(), &VideoService::processFrame, Qt::QueuedConnection);

    connect(m_videoService.get(), &VideoService::facePositionUpdated,
            m_mainWindow->getVideoWidget(), &VideoWidget::updateFaceDetection);

    connect(m_videoService.get(), &VideoService::faceRoiEncoded,
            m_frameUploadService.get(), &FrameUploadService::sendEncodedFrame);

    // ── 下行：WebSocket → MetricsService → UI ───────────────────────────────────
    connect(m_wsClient.get(), &WebSocketClient::connected,
            m_metricsService.get(), &MetricsService::onWsConnected);
    connect(m_wsClient.get(), &WebSocketClient::disconnected,
            m_metricsService.get(), &MetricsService::onWsDisconnected);
    connect(m_wsClient.get(), &WebSocketClient::textMessageReceived,
            m_metricsService.get(), &MetricsService::onServerMessage);

    connect(m_metricsService.get(), &MetricsService::dataUpdated,
            m_mainWindow->getMetricsPanel(), &MetricsPanel::updateData);

    // ── 状态栏通知 ──────────────────────────────────────────────────────────
    auto *statusBar = m_mainWindow->notificationBar();

    // 床位绑定状态
    auto updateBedBanner = [statusBar](const AppConfig &cfg) {
        if (cfg.hasBed()) {
            statusBar->hideBanner(QStringLiteral("no-bed"));
        } else {
            statusBar->showBanner(QStringLiteral("no-bed"),
                                  QStringLiteral("未绑定床位，图像流已暂停上传。请在设置页面选择床位"),
                                  StatusBar::Warning);
        }
    };
    updateBedBanner(ConfigService::instance()->config());
    connect(ConfigService::instance(), &ConfigService::configChanged,
            this, updateBedBanner);

    // 人脸检测状态
    connect(m_videoService.get(), &VideoService::facePositionUpdated,
            this, [statusBar](const QRect &, bool hasFace) {
                if (hasFace) {
                    statusBar->hideBanner(QStringLiteral("no-face"));
                } else {
                    statusBar->showBanner(QStringLiteral("no-face"),
                                          QStringLiteral("未检测到患者人脸"),
                                          StatusBar::Error);
                }
            });

    // wsThread 启动后才调用 open()，避免在线程就绪前触发网络操作
    QMetaObject::invokeMethod(m_wsClient.get(), "connectToServer", Qt::QueuedConnection);
}

AppController::~AppController() {
    // 工作线程对象的销毁必须发生在其所属线程，否则 QTimer::stop() 等操作会跨线程触发警告。
    // 正确方案：
    //   1. 通过 QThread::finished + Qt::DirectConnection 注册一个 lambda，
    //      在工作线程的 exec() 返回后（仍处于工作线程上下文）直接 delete 对象。
    //   2. 调用 quit() 令工作线程退出事件循环，触发 finished 信号。
    //   3. wait() 阻塞主线程直到工作线程完全退出（delete 已执行完毕）。
    // 注意：不能用 deleteLater——finished 触发时事件循环已退出，延迟删除永远不会被处理。
    // 注意：不能在主线程 moveToThread——只有对象当前所在线程才能调用 moveToThread。
    auto safeStopThread = [](QObject *obj, const std::unique_ptr<QThread> &thread) {
        if (!obj || !thread) return;
        connect(thread.get(), &QThread::finished,
                         thread.get(), [obj]() { delete obj; },
                         Qt::DirectConnection);
        thread->quit();
        thread->wait();
    };

    safeStopThread(m_videoService.release(), m_videoThread);
    safeStopThread(m_wsClient.release(),     m_wsThread);
}

void AppController::start() const {
    // 以降级模式启动；WebSocket 连接成功后 MetricsService 会自动切换到在线数据
    m_metricsService->startCollection();
    m_mainWindow->show();
}
