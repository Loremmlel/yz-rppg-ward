#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_vitalService(std::make_unique<VitalService>(this)),
      m_mainWindow(std::make_unique<MainWindow>()) {

    // 建立数据总线：将 Service 产生的新数据通过信号槽机制同步给 UI 监控组件
    // 这里体现了 MVC 的核心连接，Controller 负责驱动 View 发生变化
    connect(m_vitalService.get(), &VitalService::dataUpdated,
            m_mainWindow->getVitalsWidget(), &VitalsWidget::updateData);
}

AppController::~AppController() = default;

void AppController::start() {
    // 启动体征采集流水线
    m_vitalService->startCollection();

    // 显示界面
    m_mainWindow->show();
}
