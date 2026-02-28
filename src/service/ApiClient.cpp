#include "ApiClient.h"
#include "ConfigService.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QDebug>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent),
      m_nam(new QNetworkAccessManager(this))
{
}

ApiClient *ApiClient::instance() {
    static ApiClient s_instance;
    return &s_instance;
}

QUrl ApiClient::buildUrl(const QString &path) const {
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

