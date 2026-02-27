#include <QApplication>
#include "core/AppController.h"
#include "util/StyleLoader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 多文件 QSS 合并后统一设置；global 最先加载，组件样式后加载可覆盖
    const QStringList styleFiles = {
        ":/styles/global.qss",
        ":/styles/main_window.qss",
        ":/styles/home_page.qss",
        ":/styles/video_widget.qss",
        ":/styles/vitals_widget.qss",
        ":/styles/vital_card.qss",
        ":/styles/settings_page.qss"
    };
    app.setStyleSheet(StyleLoader::loadStyleSheets(styleFiles));

    AppController controller;
    controller.start();

    return QApplication::exec();
}
