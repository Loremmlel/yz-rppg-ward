#pragma once

#include <QTimer>
#include "../model/VitalData.h"

/**
 * @brief 健康数据服务中心
 *
 * 数据来源策略（双模式）：
 *  - **在线模式**：监听 WebSocketClient::textMessageReceived 信号，
 *    解析服务器推送的 JSON 并发射 dataUpdated。
 *  - **降级模式**：WebSocket 断开时自动切换为 QTimer 驱动的随机模拟数据，
 *    方便离线调试，不影响 UI 层逻辑。
 *
 * UI 层只需关注 dataUpdated 信号，无需感知当前处于哪种模式。
 */
class VitalService : public QObject {
    Q_OBJECT

public:
    explicit VitalService(QObject *parent = nullptr);
    ~VitalService() override;

    /** 获取当前最新的体征快照数据 */
    [[nodiscard]] VitalData currentData() const { return m_lastData; }

    /** 开始数据抓取与更新（启动降级模拟定时器） */
    void startCollection() const;

    /** 暂停数据更新（停止降级模拟定时器） */
    void stopCollection() const;

signals:
    /** 新的体征数据到达（在线数据或模拟数据均通过此信号发出） */
    void dataUpdated(const VitalData &data);

public slots:
    /**
     * @brief 接收 WebSocket 推送的体征 JSON，解析后发射 dataUpdated
     *        连接到 WebSocketClient::textMessageReceived
     *
     * 期望的 JSON 格式：
     * @code
     * { "heart_rate": 72, "spo2": 98, "respiration_rate": 16 }
     * @endcode
     */
    void onServerMessage(const QString &jsonText);

    /** WebSocket 已连接：暂停模拟，切换为在线模式 */
    void onWsConnected();

    /** WebSocket 已断开：恢复模拟，切换为降级模式 */
    void onWsDisconnected();

private slots:
    /** 降级模式：周期性生成随机体征数据 */
    void fetchLatestData();

private:
    VitalData m_lastData;
    QTimer   *m_timer;

    bool m_onlineMode {false}; ///< true = 使用 WebSocket 数据；false = 使用模拟数据
};
