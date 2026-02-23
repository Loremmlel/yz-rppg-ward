#pragma once
#include <qobject.h>
#include <memory>

#include "../ui/MainWindow.h"
#include "../service/VitalService.h"
#include "../service/NetworkService.h"
#include "../service/VideoService.h"


/**
 * @brief 应用程序核心控制器
 * 负责管理主窗口生命周期、业务流转及资源调度。
 */
class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);

    ~AppController() override;

    /**
     * @brief 启动整个业务流程，包括后台服务与前台主视图
     */
    void start();

    /**
     * @brief 提供对 Vital 服务的访问句柄，以供其他模块进行必要的同步
     * @return 这里的 VitalService 实例处于活跃采集状态
     */
    VitalService *getVitalService() const { return m_vitalService.get(); }

    /**
     * @brief 提供对 Video 服务的访问句柄
     */
    VideoService *getVideoService() const { return m_videoService.get(); }

private:
    /**
     * @brief 生命周期受控的业务服务组件
     * 由 Controller 维护所有 Service 的生命周期，确保数据在视图层生命周期前初始化。
     */
    std::unique_ptr<VitalService> m_vitalService;
    std::unique_ptr<VideoService> m_videoService;
    std::unique_ptr<NetworkService> m_networkService;
    std::unique_ptr<MainWindow> m_mainWindow;
};
