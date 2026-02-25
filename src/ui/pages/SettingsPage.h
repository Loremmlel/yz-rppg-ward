#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

#include "../../service/ConfigService.h"

/**
 * @brief 服务器连接配置页面
 *
 * 用户提交后委托给 ConfigService::saveConfig，
 * ConfigService 会发出 configChanged 信号，所有订阅方（WebSocketClient 等）自动响应，
 * 无需页面本身知道有哪些下游。
 */
class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

private slots:
    void onSaveClicked();

private:
    void initUI();
    void loadCurrentConfig() const;

    QLineEdit   *m_hostEdit  {};
    QSpinBox    *m_portSpin  {};
    QPushButton *m_saveBtn   {};
};
