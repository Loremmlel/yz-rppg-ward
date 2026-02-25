#pragma once

#include <memory>

#include "../ui/MainWindow.h"
#include "../service/VitalService.h"
#include "../service/NetworkService.h"
#include "../service/VideoService.h"
#include "../service/WebSocketClient.h"

/**
 * @brief 应用程序顶层控制器（Mediator）
 *
 * 负责组装所有 Service 的生命周期，并完成它们之间及与 UI 层的信号槽连接。
 * 各 Service 彼此不直接依赖，Controller 是唯一知道全局拓扑的地方。
 *
 * 数据流：
 *  上行：Camera → VideoWidget → VideoService → NetworkService → WebSocketClient → Server
 *  下行：Server → WebSocketClient → VitalService → VitalsWidget
 */
class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void start() const;

    [[nodiscard]] VitalService    *getVitalService() const { return m_vitalService.get(); }
    [[nodiscard]] VideoService    *getVideoService() const { return m_videoService.get(); }
    [[nodiscard]] WebSocketClient *getWsClient()     const { return m_wsClient.get(); }

private:
    std::unique_ptr<WebSocketClient> m_wsClient;
    std::unique_ptr<VitalService>    m_vitalService;
    std::unique_ptr<VideoService>    m_videoService;
    std::unique_ptr<NetworkService>  m_networkService;
    std::unique_ptr<MainWindow>      m_mainWindow;

    std::unique_ptr<QThread> m_videoThread;
    std::unique_ptr<QThread> m_wsThread; ///< WebSocketClient 独占，确保 QWebSocket 的所有调用在同一线程
};
