#include "VitalService.h"
#include "../model/WsProtocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QDebug>

VitalService::VitalService(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this))
{
    m_timer->setInterval(1000); // 降级模式：1s 周期
    connect(m_timer, &QTimer::timeout, this, &VitalService::fetchLatestData);
}

VitalService::~VitalService() = default;

// ─── 集合控制 ────────────────────────────────────────────────────────────────

void VitalService::startCollection() const {
    // 启动降级模拟（WebSocket 连接后会自动暂停）
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void VitalService::stopCollection() const {
    m_timer->stop();
}

// ─── WebSocket 事件槽 ─────────────────────────────────────────────────────────

void VitalService::onWsConnected() {
    qDebug() << "[VitalService] WebSocket 已连接，切换到在线数据模式";
    m_onlineMode = true;
    m_timer->stop(); // 停止降级模拟
}

void VitalService::onWsDisconnected() {
    qDebug() << "[VitalService] WebSocket 已断开，降级为模拟数据模式";
    m_onlineMode = false;
    // 恢复模拟（仅在集合已启动时）
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void VitalService::onServerMessage(const QString &jsonText) {
    if (!m_onlineMode) {
        return; // 理论上不应到达，但防御性检查
    }

    const QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "[VitalService] 无效的 JSON 消消息:" << jsonText;
        return;
    }

    const QJsonObject obj = doc.object();

    // 协议字段名从 WsProtocol 读取，避免硬编码散落各处
    const int hr = obj.value(WsProtocol::KEY_HEART_RATE).toInt(m_lastData.heartRate);
    const int sp = obj.value(WsProtocol::KEY_SPO2).toInt(m_lastData.SpO2);
    const int rr = obj.value(WsProtocol::KEY_RESPIRATION_RATE).toInt(m_lastData.respirationRate);

    m_lastData = VitalData(hr, sp, rr);
    emit dataUpdated(m_lastData);

    qDebug() << "[VitalService] 在线数据 - HR:" << hr
             << "SpO2:" << sp << "RR:" << rr;
}

// ─── 降级模拟 ─────────────────────────────────────────────────────────────────

void VitalService::fetchLatestData() {
    // 模拟随机数据（WebSocket 离线时的降级策略）
    const int hr = QRandomGenerator::global()->bounded(60, 100);
    const int sp = QRandomGenerator::global()->bounded(95, 100);
    const int rr = QRandomGenerator::global()->bounded(12, 22);

    m_lastData = VitalData(hr, sp, rr);
    emit dataUpdated(m_lastData);
}
