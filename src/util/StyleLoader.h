#pragma once

#include <QString>

/**
 * @brief 样式加载工具类
 * 负责从 Qt 资源系统或本地路径读取 QSS 文件，并将其合并为统一的全局样式表。
 */
class StyleLoader {
public:
    /**
     * @brief 批量加载样式文件
     * @param styleFiles 样式文件的路径列表（支持 qrc 虚拟路径）
     * @return 合并后的完整样式表内容字符串
     */
    static QString loadStyleSheets(const QStringList &styleFiles);
};
