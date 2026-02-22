#pragma once

#include <QStringList>
#include <QString>

/**
 * @brief 样式加载工具类
 * 负责从资源文件中加载、合并并应用 QSS 样式。
 */
class StyleLoader {
public:
    /**
     * @brief 加载指定列表的样式文件并返回合并后的字符串
     * @param styleFiles 资源路径列表
     * @return 合并后的样式表字符串
     */
    static QString loadStyleSheets(const QStringList& styleFiles);
};

