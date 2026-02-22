#pragma once
#include <qobject.h>

#include "../ui/MainWindow.h"


/**
 * @brief 应用程序核心控制器
 * 负责管理主窗口生命周期、业务流转及资源调度。
 */
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void start() const;
private:
    std::unique_ptr<MainWindow> m_mainWindow;
};