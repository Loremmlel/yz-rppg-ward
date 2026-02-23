#include "StyleLoader.h"
#include <QFile>
#include <QTextStream>

QString StyleLoader::loadStyleSheets(const QStringList &styleFiles) {
    QString combinedStyle;
    for (const QString &fileName: styleFiles) {
        QFile file(fileName);
        // 以只读文本模式读取每个 QSS 段落，并追加到最终样式表中
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            combinedStyle += stream.readAll();
            // 插入换行符避免不同文件结尾与开头规则冲突
            combinedStyle += "\n";
        }
    }
    return combinedStyle;
}
