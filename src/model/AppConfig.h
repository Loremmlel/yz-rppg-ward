#pragma once
#include <QString>

/**
 * @brief 应用程序配置（值对象）
 *
 * 纯数据载体，无行为。由 ConfigService 负责读写，通过 configChanged 信号传播。
 */
struct AppConfig {
    QString serverHost; ///< IP 或域名
    quint16 serverPort;

    static constexpr quint16 kDefaultPort = 8080;
    static const inline auto kDefaultHost = QStringLiteral("127.0.0.1");

    AppConfig()
        : serverHost(kDefaultHost), serverPort(kDefaultPort) {}

    AppConfig(QString host, quint16 port)
        : serverHost(std::move(host)), serverPort(port) {}
};
