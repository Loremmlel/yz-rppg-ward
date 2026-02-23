#pragma once

#include <QObject>
#include <QTimer>
#include "../model/VitalData.h"

/**
 * @brief 健康数据服务中心
 * 核心逻辑层，负责数据的获取、处理与分发。
 * 未来将连接网络接口或传感器驱动，当前版本暂时模拟数据生成。
 */
class VitalService : public QObject {
    Q_OBJECT

public:
    explicit VitalService(QObject *parent = nullptr);

    ~VitalService() override;

    /**
     * @brief 获取当前最新的体征快照数据
     * @return 当前存储的体征实体模型
     */
    VitalData currentData() const { return m_lastData; }

    /**
     * @brief 开始数据抓取与更新
     */
    void startCollection();

    /**
     * @brief 暂停数据更新
     */
    void stopCollection();

signals:
    /**
     * @brief 信号：当有新的体征数据到达时发射
     * @param data 包含最新指标的结构体对象
     */
    void dataUpdated(const VitalData &data);

private slots:
    /**
     * @brief 私有槽函数，负责在模拟模式下周期性重构数据
     */
    void fetchLatestData();

private:
    VitalData m_lastData;
    QTimer *m_timer;
};
