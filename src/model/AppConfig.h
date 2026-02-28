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

    QString wardCode; ///< 选定的病区代码
    QString roomNo; ///< 选定的房间号
    qint64 bedId{-1}; ///< 选定的床位 ID（-1 表示未选择）

    static constexpr quint16 kDefaultPort = 8080;
    static const inline auto kDefaultHost = QStringLiteral("127.0.0.1");

    /** @brief 是否已绑定有效床位 */
    [[nodiscard]] bool hasBed() const { return bedId > 0; }

    AppConfig()
        : serverHost(kDefaultHost), serverPort(kDefaultPort) {
    }

    AppConfig(QString host, quint16 port,
              QString ward = {}, QString room = {}, qint64 bed = -1)
        : serverHost(std::move(host)), serverPort(port),
          wardCode(std::move(ward)), roomNo(std::move(room)), bedId(bed) {
    }
};
