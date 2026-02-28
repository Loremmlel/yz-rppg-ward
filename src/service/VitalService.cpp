#include "VitalService.h"

#include <QJsonObject>
#include <QRandomGenerator>
#include <QDebug>

#include "../model/WsProtocol.h"

VitalService::VitalService(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &VitalService::fetchLatestData);
}

VitalService::~VitalService() = default;

void VitalService::startCollection() const {
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void VitalService::stopCollection() const {
    m_timer->stop();
}

void VitalService::onWsConnected() {
    m_onlineMode = true;
    m_timer->stop();
}

void VitalService::onWsDisconnected() {
    m_onlineMode = false;
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void VitalService::onServerMessage(const QString &jsonText) {
    if (!m_onlineMode) return;

    const QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "[VitalService] 无效的 JSON 消息:" << jsonText;
        return;
    }

    const QJsonObject obj = doc.object();

    // 字段缺失时 toInt 回退到当前值，避免单字段异常导致整条数据丢失
    const int hr = obj.value(WsProtocol::KEY_HEART_RATE).toInt(m_lastData.heartRate);
    const int sp = obj.value(WsProtocol::KEY_SPO2).toInt(m_lastData.SpO2);
    const int rr = obj.value(WsProtocol::KEY_RESPIRATION_RATE).toInt(m_lastData.respirationRate);

    m_lastData = VitalData(hr, sp, rr);
    emit dataUpdated(m_lastData);
}

void VitalService::fetchLatestData() {
    const int hr = QRandomGenerator::global()->bounded(60, 100);
    const int sp = QRandomGenerator::global()->bounded(95, 100);
    const int rr = QRandomGenerator::global()->bounded(12, 22);

    m_lastData = VitalData(hr, sp, rr);
    emit dataUpdated(m_lastData);
}
