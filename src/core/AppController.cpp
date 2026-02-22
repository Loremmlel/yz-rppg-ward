#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_mainWindow(std::make_unique<MainWindow>()) {
}

AppController::~AppController() = default;

void AppController::start() const {
    m_mainWindow->show();
}
