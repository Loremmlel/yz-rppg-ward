#include "ApiClient.h"
#include "ConfigService.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDebug>
#include <QNetworkRequest>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent),
      m_nam(new QNetworkAccessManager(this)) {
}

ApiClient *ApiClient::instance() {
    static ApiClient s_instance;
    return &s_instance;
}

QUrl ApiClient::buildUrl(const QString &path) {
    const auto cfg = ConfigService::instance()->config();
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(cfg.serverHost);
    url.setPort(cfg.serverPort);
    url.setPath(path);
    return url;
}

void ApiClient::getJson(const QString &path,
                        const SuccessCallback &onSuccess,
                        const ErrorCallback &onError) {
    const QUrl url = buildUrl(path);
    auto *reply = m_nam->get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [reply, onSuccess, onError] {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const QString err = reply->errorString();
            qWarning() << "[ApiClient] GET 失败:" << reply->url().toString() << err;
            if (onError) onError(err);
            return;
        }

        const auto doc = QJsonDocument::fromJson(reply->readAll());
        if (onSuccess) onSuccess(doc);
    });
}

void ApiClient::getJson(const QString &path,
                        const QUrlQuery &query,
                        const SuccessCallback &onSuccess,
                        const ErrorCallback &onError) {
    QUrl url = buildUrl(path);
    url.setQuery(query); // QUrlQuery 内部已正确处理每个参数值的百分号编码
    auto *reply = m_nam->get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [reply, onSuccess, onError] {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const QString err = reply->errorString();
            qWarning() << "[ApiClient] GET 失败:" << reply->url().toString() << err;
            if (onError) onError(err);
            return;
        }

        const auto doc = QJsonDocument::fromJson(reply->readAll());
        if (onSuccess) onSuccess(doc);
    });
}

void ApiClient::postJsonText(const QString &path,
                             const QJsonObject &body,
                             const TextSuccessCallback &onSuccess,
                             const ErrorCallback &onError) {
    const QUrl url = buildUrl(path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    auto *reply = m_nam->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [reply, onSuccess, onError] {
        reply->deleteLater();

        const QString responseText = QString::fromUtf8(reply->readAll());
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (reply->error() != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300) {
            const QString err = statusCode > 0
                                    ? QStringLiteral("HTTP %1: %2").arg(statusCode).arg(responseText)
                                    : reply->errorString();
            qWarning() << "[ApiClient] POST 失败:" << reply->url().toString() << err;
            if (onError) onError(err);
            return;
        }

        if (onSuccess) onSuccess(responseText);
    });
}
