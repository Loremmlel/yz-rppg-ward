#pragma once

#include <QString>

/**
 * @brief QSS 样式表加载工具
 *
 * 支持 qrc 虚拟路径与本地路径，将多个样式文件合并为一个字符串，
 * 便于统一通过 QApplication::setStyleSheet 应用。
 */
class StyleLoader {
public:
    /**
     * @brief 顺序加载并合并样式文件
     * @param styleFiles qrc 或本地路径列表，后加载的规则优先级更高
     */
    static QString loadStyleSheets(const QStringList &styleFiles);
};
