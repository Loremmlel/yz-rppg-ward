#include "ConfigService.h"
#include <QCoreApplication>
#include <QDir>

ConfigService::ConfigService(QObject *parent) : QObject(parent) {
    // 将 INI 文件存储在可执行文件同级目录
    const QString iniPath = QCoreApplication::applicationDirPath()
                            + QDir::separator()
                            + QStringLiteral("config.ini");
    m_settings = std::make_unique<QSettings>(iniPath, QSettings::IniFormat);
    load();
}

ConfigService *ConfigService::instance() {
    static ConfigService s_instance;
    return &s_instance;
}

void ConfigService::load() {
    m_settings->beginGroup(QStringLiteral("Network"));
    m_config.serverHost = m_settings->value(
        QStringLiteral("serverHost"),
        AppConfig::kDefaultHost).toString();
    m_config.serverPort = static_cast<quint16>(
        m_settings->value(
            QStringLiteral("serverPort"),
            AppConfig::kDefaultPort).toUInt());
    m_settings->endGroup();

    m_settings->beginGroup(QStringLiteral("Bed"));
    m_config.wardCode = m_settings->value(QStringLiteral("wardCode")).toString();
    m_config.roomNo   = m_settings->value(QStringLiteral("roomNo")).toString();
    m_config.bedId    = m_settings->value(QStringLiteral("bedId"), -1).toLongLong();
    m_settings->endGroup();
}

void ConfigService::saveConfig(const AppConfig &newConfig) {
    m_config = newConfig;

    m_settings->beginGroup(QStringLiteral("Network"));
    m_settings->setValue(QStringLiteral("serverHost"), m_config.serverHost);
    m_settings->setValue(QStringLiteral("serverPort"), m_config.serverPort);
    m_settings->endGroup();

    m_settings->beginGroup(QStringLiteral("Bed"));
    m_settings->setValue(QStringLiteral("wardCode"), m_config.wardCode);
    m_settings->setValue(QStringLiteral("roomNo"),   m_config.roomNo);
    m_settings->setValue(QStringLiteral("bedId"),     m_config.bedId);
    m_settings->endGroup();

    m_settings->sync();

    emit configChanged(m_config);
}

