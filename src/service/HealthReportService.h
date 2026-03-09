#pragma once

#include <QObject>
#include <QDateTime>

class HealthReportService : public QObject {
    Q_OBJECT

public:
    struct HistoryItem {
        QString fileName;
        QString filePath;
        QDateTime generatedAt;
    };

    static HealthReportService *instance();

    void generate(qint64 bedId,
                  const QDateTime &startTime,
                  const QDateTime &endTime,
                  const QString &interval);

    [[nodiscard]] static QList<HistoryItem> listHistory();
    [[nodiscard]] static QString readHistoryHtml(const QString &filePath);

signals:
    void reportReady(const QString &html, const QString &savedPath);
    void errorOccurred(const QString &message);
    void loadingChanged(bool loading);

private:
    explicit HealthReportService(QObject *parent = nullptr);

    [[nodiscard]] static QString reportDirPath();
    [[nodiscard]] static QString buildFileName(qint64 bedId,
                                               const QDateTime &startTime,
                                               const QDateTime &endTime,
                                               const QString &interval);
    [[nodiscard]] static QString sanitize(const QString &raw);
};

