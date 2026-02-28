#pragma once
#include <QPushButton>

class NetworkSettingsGroup;
class BedSettingsGroup;

/**
 * @brief 设置页面
 *
 * 组合 NetworkSettingsGroup 和 BedSettingsGroup 两个分组控件，
 * 仅持有全局保存按钮和保存逻辑。
 */
class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

private slots:
    void onSaveClicked();

private:
    void initUI();
    void loadCurrentConfig();

    NetworkSettingsGroup *m_networkGroup{};
    BedSettingsGroup     *m_bedGroup{};
    QPushButton          *m_saveBtn{};
};
