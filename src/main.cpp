#include <QApplication>
#include "core/AppController.h"
#include "util/StyleLoader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 从资源加载样式并应用
    const QStringList styleFiles = {
            ":/styles/MainWindow.qss",
            ":/styles/VideoWidget.qss",
            ":/styles/MetricsWidget.qss",
            ":/styles/MetricCard.qss"
    };

    app.setStyleSheet(StyleLoader::loadStyleSheets(styleFiles));

    AppController controller;
    controller.start();

    return QApplication::exec();
}
