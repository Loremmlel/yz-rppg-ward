#include <QApplication>
#include "core/AppController.h"
#include "util/StyleLoader.h"

/**
 * @brief 应用程序入口点
 * 初始化应用程序环境，加载全局样式，并启动主视图控制器。
 * @param argc 命令行参数计数
 * @param argv 命令行参数数组
 * @return 应用程序退出代码
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    const QStringList styleFiles = {
        ":/styles/MainWindow.qss",
        ":/styles/VideoWidget.qss",
        ":/styles/VitalsWidget.qss",
        ":/styles/VitalCard.qss"
    };

    app.setStyleSheet(StyleLoader::loadStyleSheets(styleFiles));

    AppController controller;
    controller.start();

    return QApplication::exec();
}
