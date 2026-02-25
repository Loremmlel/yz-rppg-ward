#include "StyleLoader.h"
#include <QFile>
#include <QTextStream>

QString StyleLoader::loadStyleSheets(const QStringList &styleFiles) {
    QString combinedStyle;
    for (const QString &fileName : styleFiles) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            combinedStyle += stream.readAll();
            combinedStyle += "\n"; // 防止相邻文件的末尾规则与开头规则合并
        }
    }
    return combinedStyle;
}
