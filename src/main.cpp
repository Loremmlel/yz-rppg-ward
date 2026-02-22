#include <QApplication>
#include <QFile>
#include <QTextStream>

#include "core/AppController.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Load stylesheet from resources
    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        app.setStyleSheet(stream.readAll());
    }

    AppController controller;
    controller.start();

    return QApplication::exec();
}
