#include "VitalsTrendService.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <algorithm>
#include <numeric>

#include "ApiClient.h"
#include "ConfigService.h"

// ── 单例 ─────────────────────────────────────────────────────────────────────
VitalsTrendService *VitalsTrendService::instance() {
    static VitalsTrendService s_instance;
    return &s_instance;
}

VitalsTrendService::VitalsTrendService(QObject *parent) : QObject(parent) {
    qRegisterMetaType<TrendResult>();
}

// ── 查询入口 ─────────────────────────────────────────────────────────────────
void VitalsTrendService::query(const qint64 bedId,
                               const QDateTime &startTime,
                               const QDateTime &endTime,
                               const QString &interval) {
    emit loadingChanged(true);

    QUrlQuery q;
    q.addQueryItem(QStringLiteral("bedId"), QString::number(bedId));
    q.addQueryItem(QStringLiteral("startTime"), startTime.toUTC().toString(Qt::ISODate));
    q.addQueryItem(QStringLiteral("endTime"), endTime.toUTC().toString(Qt::ISODate));
    q.addQueryItem(QStringLiteral("interval"), interval);

    // 捕获本地时间和粒度秒数供 buildResult 使用
    const QDateTime localStart = startTime.toLocalTime();
    const QDateTime localEnd = endTime.toLocalTime();
    const qint64 intervalSecs = parseIntervalSecs(interval);

    ApiClient::instance()->getJson(
        QStringLiteral("/api/vitals/trend"),
        q,
        [this, localStart, localEnd, intervalSecs](const QJsonDocument &doc) {
            emit loadingChanged(false);

            if (!doc.isArray()) {
                emit errorOccurred(QStringLiteral("⚠ 服务端返回数据格式异常"));
                return;
            }

            QList<VitalsTrendData> records;
            for (const auto &val: doc.array()) {
                if (!val.isObject()) continue;
                const QJsonObject obj = val.toObject();

                VitalsTrendData rec;
                rec.bucketTime = QDateTime::fromString(
                    obj.value(QStringLiteral("bucketTime")).toString(), Qt::ISODate);

                auto parseOpt = [&](const QJsonObject &o, const QString &key) -> std::optional<double> {
                    if (o.contains(key) && !o.value(key).isNull())
                        return o.value(key).toDouble();
                    return std::nullopt;
                };

                if (const auto bv = obj.value(QStringLiteral("basicVitals"));
                    bv.isObject()) {
                    const auto o = bv.toObject();
                    rec.basicVitals.hrAvg = parseOpt(o, QStringLiteral("hrAvg"));
                    rec.basicVitals.brAvg = parseOpt(o, QStringLiteral("brAvg"));
                    rec.basicVitals.sqiAvg = parseOpt(o, QStringLiteral("sqiAvg"));
                }
                if (const auto td = obj.value(QStringLiteral("hrvTimeDomain"));
                    td.isObject()) {
                    const auto o = td.toObject();
                    rec.hrvTimeDomain.sdnnMedian = parseOpt(o, QStringLiteral("sdnnMedian"));
                    rec.hrvTimeDomain.rmssdMedian = parseOpt(o, QStringLiteral("rmssdMedian"));
                    rec.hrvTimeDomain.sdsdMedian = parseOpt(o, QStringLiteral("sdsdMedian"));
                    rec.hrvTimeDomain.pnn50Median = parseOpt(o, QStringLiteral("pnn50Median"));
                    rec.hrvTimeDomain.pnn20Median = parseOpt(o, QStringLiteral("pnn20Median"));
                }
                if (const auto fd = obj.value(QStringLiteral("hrvFreqDomain"));
                    fd.isObject()) {
                    const auto o = fd.toObject();
                    rec.hrvFreqDomain.lfHfRatio = parseOpt(o, QStringLiteral("lfHfRatio"));
                    rec.hrvFreqDomain.hfAvg = parseOpt(o, QStringLiteral("hfAvg"));
                    rec.hrvFreqDomain.lfAvg = parseOpt(o, QStringLiteral("lfAvg"));
                    rec.hrvFreqDomain.vlfAvg = parseOpt(o, QStringLiteral("vlfAvg"));
                    rec.hrvFreqDomain.tpAvg = parseOpt(o, QStringLiteral("tpAvg"));
                }
                records.append(rec);
            }

            if (records.isEmpty()) {
                emit errorOccurred(QStringLiteral("📭 该时间段内暂无数据"));
                return;
            }
            emit resultReady(buildResult(records, localStart, localEnd, intervalSecs));
        },
        [this](const QString &err) {
            emit loadingChanged(false);
            emit errorOccurred(QStringLiteral("⚠ 请求失败：") + err);
        }
    );
}

// ── 结果构建 ─────────────────────────────────────────────────────────────────
VitalsTrendService::TrendResult
VitalsTrendService::buildResult(const QList<VitalsTrendData> &records,
                                const QDateTime &queryStart,
                                const QDateTime &queryEnd,
                                const qint64 intervalSecs) {
    // 提取时间戳（本地时间）
    QList<QDateTime> ts;
    ts.reserve(records.size());
    for (const auto &r: records)
        ts.append(r.bucketTime.toLocalTime());

    // 提取单字段点列表的辅助 lambda
    auto extract = [&](auto field) {
        QList<std::optional<double> > pts;
        pts.reserve(records.size());
        for (const auto &r: records) pts.append(field(r));
        return pts;
    };

    auto make = [&](auto field, bool useMedian) -> MetricSeries {
        auto pts = extract(field);
        return {ts, pts, useMedian ? calcMedian(pts) : calcMean(pts)};
    };

    TrendResult res;
    res.queryStart = queryStart;
    res.queryEnd = queryEnd;
    res.intervalSecs = intervalSecs;
    // 基础生命体征 → 均值参考线
    res.hrAvg = make([](const VitalsTrendData &r) { return r.basicVitals.hrAvg; }, false);
    res.brAvg = make([](const VitalsTrendData &r) { return r.basicVitals.brAvg; }, false);
    res.sqiAvg = make([](const VitalsTrendData &r) { return r.basicVitals.sqiAvg; }, false);
    // HRV 时域 → 中位数参考线（后端已是各 bucket 中位数，再取中位数）
    res.sdnnMedian = make([](const VitalsTrendData &r) { return r.hrvTimeDomain.sdnnMedian; }, true);
    res.rmssdMedian = make([](const VitalsTrendData &r) { return r.hrvTimeDomain.rmssdMedian; }, true);
    res.sdsdMedian = make([](const VitalsTrendData &r) { return r.hrvTimeDomain.sdsdMedian; }, true);
    res.pnn50Median = make([](const VitalsTrendData &r) { return r.hrvTimeDomain.pnn50Median; }, true);
    res.pnn20Median = make([](const VitalsTrendData &r) { return r.hrvTimeDomain.pnn20Median; }, true);
    // HRV 频域 → 均值参考线
    res.lfHfRatio = make([](const VitalsTrendData &r) { return r.hrvFreqDomain.lfHfRatio; }, false);
    res.hfAvg = make([](const VitalsTrendData &r) { return r.hrvFreqDomain.hfAvg; }, false);
    res.lfAvg = make([](const VitalsTrendData &r) { return r.hrvFreqDomain.lfAvg; }, false);
    res.vlfAvg = make([](const VitalsTrendData &r) { return r.hrvFreqDomain.vlfAvg; }, false);
    res.tpAvg = make([](const VitalsTrendData &r) { return r.hrvFreqDomain.tpAvg; }, false);
    return res;
}

// ── 统计 ─────────────────────────────────────────────────────────────────────
std::optional<double>
VitalsTrendService::calcMean(const QList<std::optional<double> > &pts) {
    double sum = 0.0;
    int cnt = 0;
    for (const auto &p: pts)
        if (p.has_value()) {
            sum += *p;
            ++cnt;
        }
    return cnt > 0 ? std::optional(sum / cnt) : std::nullopt;
}

std::optional<double>
VitalsTrendService::calcMedian(const QList<std::optional<double> > &pts) {
    QList<double> vals;
    for (const auto &p: pts) if (p.has_value()) vals.append(*p);
    if (vals.isEmpty()) return std::nullopt;
    const int n = vals.size();
    const auto mid = vals.begin() + n / 2;
    std::ranges::nth_element(vals, mid);
    if (n % 2 == 1) return *mid;

    return std::midpoint(*std::ranges::max_element(vals.begin(), mid), *mid);
}

// ── 工具：间隔字符串 → 秒数 ──────────────────────────────────────────────────
qint64 VitalsTrendService::parseIntervalSecs(const QString &interval) {
    if (interval.isEmpty()) return 0;

    // 格式：<数字><单位>，单位 s/m/h/d
    bool ok = false;
    const QChar unit = interval.back().toLower();
    const qint64 num = interval.chopped(1).toLongLong(&ok);
    if (!ok || num <= 0) return 0;

    switch (unit.unicode()) {
        case 's': return num;
        case 'm': return num * 60;
        case 'h': return num * 3600;
        case 'd': return num * 86400;
        default: return 0;
    }
}