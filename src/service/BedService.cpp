#include "BedService.h"
#include "ConfigService.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDebug>

BedService::BedService(QObject *parent)
    : QObject(parent),
      m_nam(new QNetworkAccessManager(this))
{
}

BedService *BedService::instance() {
    static BedService s_instance;
    return &s_instance;
}

QUrl BedService::baseUrl() const {
    const auto cfg = ConfigService::instance()->config();
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(cfg.serverHost);
    url.setPort(cfg.serverPort);
    return url;
}

void BedService::fetchWards() {
    auto url = baseUrl();
    url.setPath(QStringLiteral("/api/wards/list"));

    auto *reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[BedService] fetchWards 失败:" << reply->errorString();
            emit errorOccurred(reply->errorString());
            return;
        }
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        QStringList wardCodes;
        for (const auto &val : doc.array()) {
            const auto obj = val.toObject();
            wardCodes.append(obj.value(QStringLiteral("wardCode")).toString());
        }
        emit wardsFetched(wardCodes);
    });
}

void BedService::fetchRooms(const QString &wardCode) {
    QUrl url = baseUrl();
    url.setPath(QStringLiteral("/api/wards/%1/rooms").arg(wardCode));

    auto *reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[BedService] fetchRooms 失败:" << reply->errorString();
            emit errorOccurred(reply->errorString());
            return;
        }
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        emit roomsFetched(doc.array());
    });
}

void BedService::fetchBeds(const QString &wardCode, const QString &roomNo) {
    QUrl url = baseUrl();
    url.setPath(QStringLiteral("/api/wards/%1/rooms/%2/beds").arg(wardCode, roomNo));

    auto *reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[BedService] fetchBeds 失败:" << reply->errorString();
            emit errorOccurred(reply->errorString());
            return;
        }
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        emit bedsFetched(doc.array());
    });
}

