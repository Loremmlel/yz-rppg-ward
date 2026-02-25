#include "NetworkService.h"
#include "ConfigService.h"
#include <QDebug>

NetworkService::NetworkService(QObject *parent) : QObject(parent) {
    // 启动时读取当前配置
    const AppConfig cfg = ConfigService::instance()->config();
    onConfigChanged(cfg);

    // 订阅后续配置变更（Observer 模式）
    connect(ConfigService::instance(), &ConfigService::configChanged,
            this, &NetworkService::onConfigChanged);
}

void NetworkService::sendFaceRoiStream(const QImage &faceRoi) {
    // TODO: 使用 m_serverHost / m_serverPort 建立连接并发送数据
}

void NetworkService::onConfigChanged(const AppConfig &config) {
    m_serverHost = config.serverHost;
    m_serverPort = config.serverPort;
    qDebug() << "[NetworkService] 服务器配置已更新:"
             << m_serverHost << ":" << m_serverPort;
}
