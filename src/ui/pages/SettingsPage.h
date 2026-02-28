#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QJsonArray>

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

    // ── 床位级联选择 ──
    void onWardChanged(const QString &wardCode);
    void onRoomChanged(const QString &roomNo);
    void onBedChanged(int index);

    void onWardsFetched(const QStringList &wardCodes);
    void onRoomsFetched(const QJsonArray &rooms);
    void onBedsFetched(const QJsonArray &beds);
    void onBedServiceError(const QString &error);

    void onRefreshClicked();

private:
    void initUI();
    void loadCurrentConfig();
    void loadBedSelection();

    QLineEdit   *m_hostEdit  {};
    QSpinBox    *m_portSpin  {};

    QComboBox   *m_wardCombo {};
    QComboBox   *m_roomCombo {};
    QComboBox   *m_bedCombo  {};
    QPushButton *m_refreshBtn {};
    QLabel      *m_bedStatusLabel {};

    QPushButton *m_saveBtn   {};

    // 暂存已保存的床位选择，用于级联加载完成后恢复选中项
    QString m_savedWardCode;
    QString m_savedRoomNo;
    qint64  m_savedBedId{-1};
    bool    m_isRestoringSelection{false}; ///< 正在恢复已保存选择，抑制级联清空
};
