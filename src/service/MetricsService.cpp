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

    // 字段缺失时回退到当前值，避免单字段异常导致整条数据丢失
    const int hr = obj.value(WsProtocol::KEY_HEART_RATE).toInt(m_lastData.heartRate);
    const int sq = obj.value(WsProtocol::KEY_SQI).toInt(m_lastData.sqi);

    m_lastData = MetricsData(hr, sq);
    emit dataUpdated(m_lastData);
}

void MetricsService::fetchLatestData() {
    const int hr = QRandomGenerator::global()->bounded(60, 100);
    const int sq = QRandomGenerator::global()->bounded(50, 100);

    m_lastData = MetricsData(hr, sq);
    emit dataUpdated(m_lastData);
}

