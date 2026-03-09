#include "HealthReportService.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>

#include "ApiClient.h"

HealthReportService *HealthReportService::instance() {
    static HealthReportService s_instance;
    return &s_instance;
}

HealthReportService::HealthReportService(QObject *parent) : QObject(parent) {
}

void HealthReportService::generate(const qint64 bedId,
                                   const QDateTime &startTime,
                                   const QDateTime &endTime,
                                   const QString &interval) {
    emit loadingChanged(true);

    QJsonObject body;
    body.insert(QStringLiteral("bedId"), bedId);
    body.insert(QStringLiteral("startTime"), startTime.toUTC().toString(Qt::ISODate));
    body.insert(QStringLiteral("endTime"), endTime.toUTC().toString(Qt::ISODate));
    body.insert(QStringLiteral("interval"), interval);

    ApiClient::instance()->postJsonText(
        QStringLiteral("/api/report/generate"),
        body,
        [this, bedId, startTime, endTime, interval](const QString &html) {
            emit loadingChanged(false);

            const QString dirPath = reportDirPath();
            const QString filePath = QDir(dirPath).filePath(buildFileName(bedId, startTime, endTime, interval));

            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                emit errorOccurred(QStringLiteral("⚠ 报告生成成功，但保存本地历史失败"));
                emit reportReady(html, QString());
                return;
            }

            file.write(html.toUtf8());
            file.close();
            emit reportReady(html, filePath);
        },
        [this](const QString &err) {
            emit loadingChanged(false);
            emit errorOccurred(QStringLiteral("⚠ 生成报告失败：") + err);
        }
    );
}

QList<HealthReportService::HistoryItem> HealthReportService::listHistory() {
    QList<HistoryItem> items;

    const QDir dir(reportDirPath());
    const QFileInfoList files = dir.entryInfoList({QStringLiteral("*.html")}, QDir::Files, QDir::Time);
    items.reserve(files.size());

    for (const QFileInfo &info: files) {
        items.append({info.fileName(), info.absoluteFilePath(), info.lastModified()});
    }

    return items;
}

QString HealthReportService::readHistoryHtml(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString HealthReportService::reportDirPath() {
    const QString dirPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("report"));
    QDir().mkpath(dirPath);
    return dirPath;
}

QString HealthReportService::buildFileName(const qint64 bedId,
                                           const QDateTime &startTime,
                                           const QDateTime &endTime,
                                           const QString &interval) {
    const QString nowTag = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"));
    const QString startTag = startTime.toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString endTag = endTime.toString(QStringLiteral("yyyyMMdd_HHmmss"));

    return QStringLiteral("bed-%1_start-%2_end-%3_interval-%4_ts-%5.html")
            .arg(QString::number(bedId), startTag, endTag, sanitize(interval), nowTag);
}

QString HealthReportService::sanitize(const QString &raw) {
    QString out;
    out.reserve(raw.size());
    for (const QChar ch: raw) {
        if (ch.isLetterOrNumber() || ch == QChar('-') || ch == QChar('_')) out.append(ch);
        else out.append(QChar('-'));
    }
    return out;
}
