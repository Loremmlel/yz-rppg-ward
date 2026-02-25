#pragma once
#include <QString>

/**
 * @brief 应用程序配置数据模型
 * 存储所有持久化配置项的值对象（Value Object），纯数据，无行为。
 */
struct AppConfig {
    QString serverHost; ///< 后端服务器地址（IP 或域名）
    quint16 serverPort; ///< 后端服务器端口

    static constexpr quint16 kDefaultPort = 8080;
    static const inline auto kDefaultHost = QStringLiteral("127.0.0.1");

    AppConfig()
        : serverHost(kDefaultHost), serverPort(kDefaultPort) {}

    AppConfig(QString host, quint16 port)
        : serverHost(std::move(host)), serverPort(port) {}
};

