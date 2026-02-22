#include "StyleLoader.h"
#include <QFile>
#include <QTextStream>

QString StyleLoader::loadStyleSheets(const QStringList& styleFiles) {
    QString combinedStyle;
    for (const QString &fileName : styleFiles) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            combinedStyle += stream.readAll();
            combinedStyle += "\n"; // 确保不同文件的样式规则之间有分隔
        }
    }
    return combinedStyle;
}

