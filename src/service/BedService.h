#pragma once

#include <QObject>
#include <QJsonArray>

/**
 * @brief 床位数据查询服务（单例）
 *
 * 通过 REST API 获取病区 → 病房 → 床位的层级数据，
 * 供设置页面的级联下拉框使用。底层 HTTP 调用委托给 ApiClient。
 */
class BedService : public QObject {
    Q_OBJECT

public:
    static BedService *instance();

    /** 获取所有病区代码列表 */
    void fetchWards();

    /** 获取指定病区下的所有病房（含床位） */
    void fetchRooms(const QString &wardCode);

    /** 获取指定病区指定病房下的所有床位 */
    void fetchBeds(const QString &wardCode, const QString &roomNo);

signals:
    /** 病区列表已获取 */
    void wardsFetched(const QStringList &wardCodes);

    /** 病房列表已获取（JSON 数组，每个元素含 roomNo 和 beds） */
    void roomsFetched(const QJsonArray &rooms);

    /** 床位列表已获取（JSON 数组，每个元素含 id, bedNo, status 等） */
    void bedsFetched(const QJsonArray &beds);

    /** 网络请求出错 */
    void errorOccurred(const QString &error);

private:
    explicit BedService(QObject *parent = nullptr);
};
