#include "StyleLoader.h"

#include <QFile>
#include <QWidget>
#include <QDebug>

QString StyleLoader::load(const QString &qrcPath) {
    QFile file(qrcPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "StyleLoader: failed to open" << qrcPath;
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString StyleLoader::loadMultiple(const QStringList &qrcPaths) {
    QString combined;
    for (const auto &path : qrcPaths) {
        combined += load(path);
        combined += QChar('\n');
    }
    return combined;
}

void StyleLoader::apply(QWidget *widget, const QString &qrcPath) {
    widget->setStyleSheet(load(qrcPath));
}

void StyleLoader::applyMultiple(QWidget *widget, const QStringList &qrcPaths) {
    widget->setStyleSheet(loadMultiple(qrcPaths));
}
