#pragma once

#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QWidget>

#include "../../service/HealthReportService.h"
#include "../widgets/common/TimeRangeControlBar.h"

class HealthReportPage : public QWidget {
    Q_OBJECT

public:
    explicit HealthReportPage(QWidget *parent = nullptr);

private slots:
    void onQueryRequested(const QDateTime &start,
                          const QDateTime &end,
                          const QString &interval) const;
    void onHistorySelected(int row);
    void refreshHistory();

private:
    void initUI();
    void setStatus(const QString &text, bool isError) const;
    void showHtml(const QString &html) const;

    TimeRangeControlBar *m_controlBar{nullptr};
    QListWidget *m_historyList{nullptr};
    QLabel *m_htmlLabel{nullptr};
    QLabel *m_statusLabel{nullptr};

    QList<HealthReportService::HistoryItem> m_historyItems;
};

