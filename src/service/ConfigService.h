#pragma once
#include <QSettings>
#include <memory>

#include "../model/AppConfig.h"

/**
 * @brief 配置管理服务（单例）
 *
 * 职责：
 *  - 从 INI 文件加载配置、持久化保存配置
 *  - 当配置发生变更时，通过 configChanged 信号通知所有订阅方
 *
 * 用法（Observer 模式）：
 * @code
 *   connect(ConfigService::instance(), &ConfigService::configChanged,
 *           this, &MyClass::onConfigChanged);
 * @endcode
 */
class ConfigService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取全局单例
     */
    static ConfigService *instance();

    /**
     * @brief 返回当前配置的快照
     */
    [[nodiscard]] AppConfig config() const { return m_config; }

    /**
     * @brief 将新配置持久化到 INI 文件，并通知所有订阅者
     * @param newConfig 新的配置值对象
     */
    void saveConfig(const AppConfig &newConfig);

signals:
    /**
     * @brief 配置已保存并生效时发出
     * @param config 最新的配置快照
     */
    void configChanged(const AppConfig &config);

private:
    explicit ConfigService(QObject *parent = nullptr);

    void load();

    AppConfig m_config;
    std::unique_ptr<QSettings> m_settings;
};

