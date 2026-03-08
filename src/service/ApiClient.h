#pragma once

#include <QObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QUrlQuery>

#include <functional>

/**
 * @brief HTTP REST 客户端（单例）
 *
 * 封装 QNetworkAccessManager，提供面向 JSON 的 GET 便捷方法。
 * 基础 URL 自动从 ConfigService 的 serverHost / serverPort 推导。
 *
 * 所有回调均在主线程事件循环中执行，可安全操作 UI。
 */
class ApiClient : public QObject {
    Q_OBJECT

public:
    static ApiClient *instance();

    using SuccessCallback = std::function<void(const QJsonDocument &)>;
    using ErrorCallback = std::function<void(const QString &)>;

    /**
     * @brief 发起 GET 请求并以 JSON 形式回调
     * @param path    URL 路径，如 "/api/wards/list"
     * @param onSuccess 成功回调，参数为解析后的 QJsonDocument
     * @param onError   失败回调，参数为错误描述字符串；可省略
     */
    void getJson(const QString &path,
                 const SuccessCallback &onSuccess,
                 const ErrorCallback &onError = nullptr);

    /**
     * @brief 发起 GET 请求（带查询参数），以 JSON 形式回调
     * @param path    URL 路径，如 "/api/vitals/trend"（不含 '?'）
     * @param query   查询参数，由调用方用 QUrlQuery 构建，避免手动拼接导致二次编码
     * @param onSuccess 成功回调
     * @param onError   失败回调；可省略
     */
    void getJson(const QString &path,
                 const QUrlQuery &query,
                 const SuccessCallback &onSuccess,
                 const ErrorCallback &onError = nullptr);

private:
    explicit ApiClient(QObject *parent = nullptr);

    [[nodiscard]] static QUrl buildUrl(const QString &path);

    QNetworkAccessManager *m_nam{nullptr};
};