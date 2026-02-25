#include "NetworkService.h"
#include "ConfigService.h"
#include <QDebug>
#include <QLabel>
#include <QPixmap>

NetworkService::NetworkService(QObject *parent) : QObject(parent) {
    // 启动时读取当前配置
    const AppConfig cfg = ConfigService::instance()->config();
    onConfigChanged(cfg);

    // 订阅后续配置变更（Observer 模式）
    connect(ConfigService::instance(), &ConfigService::configChanged,
            this, &NetworkService::onConfigChanged);
}

void NetworkService::sendFaceRoiStream(const QImage &faceRoi) {
    // ---- 调试预览窗口（静态，首次调用时创建，后续复用）----
    static QLabel *previewLabel = [] {
        auto *label = new QLabel();
        label->setWindowTitle("DEBUG: Face ROI Stream");
        label->setFixedSize(256, 256);
        label->setAlignment(Qt::AlignCenter);
        label->show();
        return label;
    }();
    previewLabel->setPixmap(QPixmap::fromImage(faceRoi));
    // -------------------------------------------------------

    // TODO: 使用 m_serverHost / m_serverPort 建立连接并发送数据
}

void NetworkService::onConfigChanged(const AppConfig &config) {
    m_serverHost = config.serverHost;
    m_serverPort = config.serverPort;
    qDebug() << "[NetworkService] 服务器配置已更新:"
             << m_serverHost << ":" << m_serverPort;
}
