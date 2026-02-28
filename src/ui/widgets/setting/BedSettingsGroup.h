#pragma once
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QJsonArray>

#include "../../../model/AppConfig.h"

/**
 * @brief 床位配置分组控件
 *
 * 三级级联下拉（病区 → 病房 → 床位），通过 BedService REST API 填充数据。
 * 支持从配置恢复已保存选项。
 */
class BedSettingsGroup : public QGroupBox {
    Q_OBJECT

public:
    explicit BedSettingsGroup(QWidget *parent = nullptr);

    /** 从配置快照恢复 UI 状态并发起级联加载 */
    void loadConfig(const AppConfig &cfg);

    [[nodiscard]] QString wardCode() const;
    [[nodiscard]] QString roomNo()   const;
    [[nodiscard]] qint64  bedId()    const;

private slots:
    void onWardChanged(const QString &wardCode);
    void onRoomChanged(const QString &roomNo);

    void onWardsFetched(const QStringList &wardCodes);
    void onRoomsFetched(const QJsonArray &rooms);
    void onBedsFetched(const QJsonArray &beds);
    void onBedServiceError(const QString &error);

    void onRefreshClicked();

private:
    void initUI();
    void startLoading();

    QComboBox   *m_wardCombo{};
    QComboBox   *m_roomCombo{};
    QComboBox   *m_bedCombo{};
    QPushButton *m_refreshBtn{};
    QLabel      *m_statusLabel{};

    // 暂存已保存选择，用于级联加载完成后恢复选中项
    QString m_savedWardCode;
    QString m_savedRoomNo;
    qint64  m_savedBedId{-1};
    bool    m_isRestoringSelection{false};
};

