#pragma once

#include <QObject>
#include <QDateTime>
#include <QList>
#include <optional>

#include "../model/VitalsTrendData.h"

/**
 * @brief 生命体征历史趋势服务
 *
 * 职责：
 *  - 接收查询参数，向 REST API 发起请求
 *  - 解析响应 JSON → QList<VitalsTrendData>
 *  - 计算每个指标序列的参考值（均值或中位数）
 *  - 通过信号将结果分发给 UI 层
 *
 * UI 层只需连接信号，不关心网络细节。
 */
class VitalsTrendService : public QObject {
    Q_OBJECT

public:
    /** 单条指标序列的完整数据包，供 TrendCard 直接消费 */
    struct MetricSeries {
        QList<QDateTime>             timestamps; ///< 各 bucket 时刻（本地时间）
        QList<std::optional<double>> points;     ///< 各 bucket 数值，nullopt 表示缺失
        std::optional<double>        refValue;   ///< 参考线值（均值 or 中位数）
    };

    /** 一次查询结果，包含全部 13 个指标序列 */
    struct TrendResult {
        QDateTime queryStart; ///< 用户选择的查询起始时刻（本地时间）
        QDateTime queryEnd;   ///< 用户选择的查询结束时刻（本地时间）
        // 基础生命体征
        MetricSeries hrAvg;
        MetricSeries brAvg;
        MetricSeries sqiAvg;
        // HRV 时域
        MetricSeries sdnnMedian;
        MetricSeries rmssdMedian;
        MetricSeries sdsdMedian;
        MetricSeries pnn50Median;
        MetricSeries pnn20Median;
        // HRV 频域
        MetricSeries lfHfRatio;
        MetricSeries hfAvg;
        MetricSeries lfAvg;
        MetricSeries vlfAvg;
        MetricSeries tpAvg;
    };

    static VitalsTrendService *instance();

    /**
     * @brief 发起历史趋势查询
     *
     * 查询完成后发出 resultReady 或 errorOccurred 信号。
     *
     * @param bedId     床位 ID
     * @param startTime 查询起始时刻（本地时间，内部自动转 UTC）
     * @param endTime   查询结束时刻（本地时间，内部自动转 UTC）
     * @param interval  时间粒度，如 "5m"、"1h"
     */
    void query(qint64 bedId,
               const QDateTime &startTime,
               const QDateTime &endTime,
               const QString   &interval);

signals:
    /** 查询成功，携带完整结果 */
    void resultReady(const TrendResult &result);

    /** 查询失败或数据为空 */
    void errorOccurred(const QString &message);

    /** 正在请求中（可用于 UI 加载状态） */
    void loadingChanged(bool loading);

private:
    explicit VitalsTrendService(QObject *parent = nullptr);

    /** 将解析完的 records 转换为 TrendResult */
    static TrendResult buildResult(const QList<VitalsTrendData> &records,
                                   const QDateTime &queryStart,
                                   const QDateTime &queryEnd);

    /** 计算均值 */
    static std::optional<double> calcMean(const QList<std::optional<double>> &pts);

    /** 计算中位数 */
    static std::optional<double> calcMedian(const QList<std::optional<double>> &pts);
};

Q_DECLARE_METATYPE(VitalsTrendService::TrendResult)

