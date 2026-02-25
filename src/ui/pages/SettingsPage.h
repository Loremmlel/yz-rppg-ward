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
 * @brief 设置页面视图
 * 提供服务器地址与端口的配置 UI。
 * 用户点击「保存」后，将新配置委托给 ConfigService 持久化，
 * 并通过 configChanged 信号广播给所有订阅方。
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

    QLineEdit  *m_hostEdit{};
    QSpinBox   *m_portSpin{};
    QPushButton *m_saveBtn{};
};
