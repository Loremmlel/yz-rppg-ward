#include "BedService.h"
#include "ApiClient.h"

#include <QJsonObject>

BedService::BedService(QObject *parent) : QObject(parent) {}

BedService *BedService::instance() {
    static BedService s_instance;
    return &s_instance;
}

void BedService::fetchWards() {
    ApiClient::instance()->getJson(
        QStringLiteral("/api/wards/list"),
        [this](const QJsonDocument &doc) {
            QStringList wardCodes;
            for (const auto &val : doc.array()) {
                wardCodes.append(val.toObject()
                    .value(QStringLiteral("wardCode")).toString());
            }
            emit wardsFetched(wardCodes);
        },
        [this](const QString &err) { emit errorOccurred(err); }
    );
}

void BedService::fetchRooms(const QString &wardCode) {
    ApiClient::instance()->getJson(
        QStringLiteral("/api/wards/%1/rooms").arg(wardCode),
        [this](const QJsonDocument &doc) { emit roomsFetched(doc.array()); },
        [this](const QString &err)       { emit errorOccurred(err); }
    );
}

void BedService::fetchBeds(const QString &wardCode, const QString &roomNo) {
    ApiClient::instance()->getJson(
        QStringLiteral("/api/wards/%1/rooms/%2/beds").arg(wardCode, roomNo),
        [this](const QJsonDocument &doc) { emit bedsFetched(doc.array()); },
        [this](const QString &err)       { emit errorOccurred(err); }
    );
}

