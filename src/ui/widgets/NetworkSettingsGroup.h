#pragma once
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>

#include "../../model/AppConfig.h"

/**
 * @brief 网络配置分组控件
 *
 * 包含服务器地址和端口输入框，供 SettingsPage 组合使用。
 */
class NetworkSettingsGroup : public QGroupBox {
    Q_OBJECT

public:
    explicit NetworkSettingsGroup(QWidget *parent = nullptr);

    /** 从配置快照恢复 UI 状态 */
    void loadConfig(const AppConfig &cfg);

    [[nodiscard]] QString host() const;
    [[nodiscard]] quint16 port() const;

private:
    void initUI();

    QLineEdit *m_hostEdit{};
    QSpinBox  *m_portSpin{};
};

