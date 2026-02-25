#include <QApplication>
#include "core/AppController.h"
#include "util/StyleLoader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 多文件 QSS 合并后统一设置，后加载的规则优先级更高
    const QStringList styleFiles = {
        ":/styles/MainWindow.qss",
        ":/styles/VideoWidget.qss",
        ":/styles/VitalsWidget.qss",
        ":/styles/VitalCard.qss",
        ":/styles/SettingsPage.qss"
    };
    app.setStyleSheet(StyleLoader::loadStyleSheets(styleFiles));

    AppController controller;
    controller.start();

    return QApplication::exec();
}
