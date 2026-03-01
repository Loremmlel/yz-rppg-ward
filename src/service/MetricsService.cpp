#include "MetricsService.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QDebug>

#include "../model/WsProtocol.h"

MetricsService::MetricsService(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &MetricsService::fetchLatestData);
}

MetricsService::~MetricsService() = default;

void MetricsService::startCollection() const {
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void MetricsService::stopCollection() const {
    m_timer->stop();
}

void MetricsService::onWsConnected() {
    m_onlineMode = true;
    m_timer->stop();
}

void MetricsService::onWsDisconnected() {
    m_onlineMode = false;
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void MetricsService::onServerMessage(const QString &jsonText) {
    if (!m_onlineMode) return;

    const QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "[MetricsService] 无效的 JSON 消息:" << jsonText;
        return;
    }

    const QJsonObject obj = doc.object();

    std::optional<double> hr;
    if (obj.contains(WsProtocol::KEY_HEART_RATE) && !obj.value(WsProtocol::KEY_HEART_RATE).isNull()) {
        hr = obj.value(WsProtocol::KEY_HEART_RATE).toDouble();
    }

    std::optional<double> sq;
    if (obj.contains(WsProtocol::KEY_SQI) && !obj.value(WsProtocol::KEY_SQI).isNull()) {
        sq = obj.value(WsProtocol::KEY_SQI).toDouble();
    }

    m_lastData = MetricsData(hr, sq);
    emit dataUpdated(m_lastData);
}

void MetricsService::fetchLatestData() {
    const double hrVal = 60.0 + QRandomGenerator::global()->generateDouble() * 40.0;
    const double sqVal = 50.0 + QRandomGenerator::global()->generateDouble() * 50.0;

    m_lastData = MetricsData(hrVal, sqVal);
    emit dataUpdated(m_lastData);
}
