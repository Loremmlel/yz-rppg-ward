#include "VitalService.h"
#include <QRandomGenerator>

VitalService::VitalService(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    // 初始化更新间隔，后期可根据不同业务策略或网络报文频率动态调整
    connect(m_timer, &QTimer::timeout, this, &VitalService::fetchLatestData);
}

VitalService::~VitalService() = default;

void VitalService::startCollection() {
    if (!m_timer->isActive()) {
        m_timer->start(1000); // 周期 1s
    }
}

void VitalService::stopCollection() {
    m_timer->stop();
}

void VitalService::fetchLatestData() {
    // 模拟数据生成逻辑
    // 未来此处应接入网络 Socket 监听、WebSocket 或串口驱动。
    int hr = QRandomGenerator::global()->bounded(60, 100);
    int sp = QRandomGenerator::global()->bounded(95, 100);
    int rr = QRandomGenerator::global()->bounded(12, 22);

    m_lastData = VitalData(hr, sp, rr);

    // 发射更新通知
    emit dataUpdated(m_lastData);
}
