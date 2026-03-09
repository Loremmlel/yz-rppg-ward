#include <QApplication>
#include "core/AppController.h"
#include "util/StyleLoader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 所有样式统一在 QApplication 级别一次性加载，避免控件级 setStyleSheet 截断继承链
    app.setStyleSheet(StyleLoader::loadMultiple({
        QStringLiteral(":/styles/global.qss"),
        QStringLiteral(":/styles/main_window.qss"),
        QStringLiteral(":/styles/status_bar.qss"),
        QStringLiteral(":/styles/home.qss"),
        QStringLiteral(":/styles/video_widget.qss"),
        QStringLiteral(":/styles/metrics_panel.qss"),
        QStringLiteral(":/styles/metric_card.qss"),
        QStringLiteral(":/styles/settings.qss"),
        QStringLiteral(":/styles/vitals_trend_page.qss"),
        QStringLiteral(":/styles/health_report_page.qss"),
        QStringLiteral(":/styles/app_dialog.qss"),
    }));

    const AppController controller;
    controller.start();

    return QApplication::exec();
}