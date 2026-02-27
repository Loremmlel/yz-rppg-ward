#include <QApplication>
#include "core/AppController.h"
#include "util/StyleLoader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 全局样式在应用级别加载（QToolTip / QMessageBox 等顶层弹窗不是 MainWindow 的子控件）
    app.setStyleSheet(StyleLoader::load(QStringLiteral(":/styles/global.qss")));

    AppController controller;
    controller.start();

    return QApplication::exec();
}
