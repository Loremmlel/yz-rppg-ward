#include <QApplication>
#include <QFile>
#include <QTextStream>

#include "core/AppController.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 加载并合并样式表
    QString styleSheet;
    const QStringList styleFiles = {
        ":/styles/MainWindow.qss",
        ":/styles/VideoWidget.qss",
        ":/styles/MetricsWidget.qss",
        ":/styles/MetricCard.qss"
    };

    for (const QString &fileName : styleFiles) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            styleSheet += stream.readAll();
            styleSheet += "\n"; // 确保样式表之间有分隔
        }
    }
    app.setStyleSheet(styleSheet);

    AppController controller;
    controller.start();

    return QApplication::exec();
}
