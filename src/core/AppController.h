#pragma once
#include <qobject.h>

#include "../ui/MainWindow.h"


class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void start() const;
private:
    std::unique_ptr<MainWindow> m_mainWindow;
};