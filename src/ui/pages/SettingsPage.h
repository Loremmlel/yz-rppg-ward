#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief 设置页面视图
 * 目前作为占位符，未来可在此扩展系统配置功能。
 */
class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

private:
    void initUI();
};

