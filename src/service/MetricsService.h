#pragma once

#include <QTimer>
#include "../model/MetricsData.h"

/**
 * @brief 监测指标数据服务
 *
 * 采用在线/降级双模式：
 *  - WebSocket 连接期间，解析服务器推送的 JSON 并分发数据。
 *  - WebSocket 断开时，自动切换为 QTimer 驱动的随机模拟，保证 UI 持续有数据可显示。
 *
 * UI 层只关注 dataUpdated 信号，无需感知当前处于哪种模式。
 */
class MetricsService : public QObject {
    Q_OBJECT

public:
    explicit MetricsService(QObject *parent = nullptr);

    ~MetricsService() override;

    [[nodiscard]] MetricsData currentData() const { return m_lastData; }

    /** 启动降级模拟定时器（WebSocket 上线后会自动停止）。 */
    void startCollection() const;

    void stopCollection() const;

signals:
    void dataUpdated(const MetricsData &data);

public slots:
    /**
     * @brief 解析服务器推送的指标 JSON，连接到 WebSocketClient::textMessageReceived。
     *
     * 期望格式：{ "heart_rate": 72, "sqi": 85 }
     * 字段缺失时保留上一次的值。
     */
    void onServerMessage(const QString &jsonText);

    /** WebSocket 已连接：停止模拟，切换为在线数据。 */
    void onWsConnected();

    /** WebSocket 已断开：恢复模拟，防止 UI 数据冻结。 */
    void onWsDisconnected();

private slots:
    void fetchLatestData();

private:
    MetricsData m_lastData;
    QTimer *m_timer;
    bool m_onlineMode{false};
};

