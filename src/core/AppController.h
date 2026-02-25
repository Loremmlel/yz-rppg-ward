#pragma once
#include <QObject>
#include <QThread>
#include <memory>

#include "../ui/MainWindow.h"
#include "../service/VitalService.h"
#include "../service/NetworkService.h"
#include "../service/VideoService.h"
#include "../service/WebSocketClient.h"


/**
 * @brief 应用程序核心控制器
 *
 * 职责：
 *  - 组装所有 Service 的生命周期
 *  - 完成各 Service 与 UI 之间的信号槽连接（Mediator 模式）
 *  - 管理后台线程
 *
 * 数据流：
 *  Camera → VideoWidget --frameCaptured--> VideoService --faceRoiExtracted-->
 *  NetworkService --sendBinaryMessage--> WebSocketClient --ws--> Server
 *
 *  Server --ws text--> WebSocketClient --textMessageReceived--> VitalService
 *  --dataUpdated--> VitalsWidget
 */
class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);

    ~AppController() override;

    /**
     * @brief 启动整个业务流程，包括后台服务与前台主视图
     */
    void start() const;

    /**
     * @brief 提供对 Vital 服务的访问句柄，以供其他模块进行必要的同步
     * @return 这里的 VitalService 实例处于活跃采集状态
     */
    VitalService *getVitalService() const { return m_vitalService.get(); }

    /**
     * @brief 提供对 Video 服务的访问句柄
     */
    VideoService *getVideoService() const { return m_videoService.get(); }

    /**
     * @brief 提供对 WebSocketClient 服务的访问句柄
     */
    WebSocketClient *getWsClient() const { return m_wsClient.get(); }

private:
    /**
     * @brief 生命周期受控的业务服务组件
     * 由 Controller 维护所有 Service 的生命周期，确保数据在视图层生命周期前初始化。
     */
    std::unique_ptr<WebSocketClient> m_wsClient;
    std::unique_ptr<VitalService> m_vitalService;
    std::unique_ptr<VideoService> m_videoService;
    std::unique_ptr<NetworkService> m_networkService;
    std::unique_ptr<MainWindow> m_mainWindow;

    std::unique_ptr<QThread> m_videoThread;
    std::unique_ptr<QThread> m_wsThread;   ///< WebSocketClient 专属线程
};
