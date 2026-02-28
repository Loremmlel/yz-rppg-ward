#pragma once
#include <QFile>
#include <QString>

class QWidget;

/**
 * @brief 样式表加载工具类
 *
 * 从 Qt 资源系统 (.qrc) 中读取 QSS 文件，并应用到指定 Widget。
 */
class StyleLoader {
public:
    StyleLoader() = delete;

    /**
     * @brief 读取资源路径中的 QSS 文件内容
     * @param qrcPath 资源路径，如 ":/styles/global.qss"
     * @return QSS 字符串；若读取失败则返回空字符串
     */
    static QString load(const QString &qrcPath) {
        QFile file(qrcPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "StyleLoader: failed to open" << qrcPath;
            return {};
        }
        return QString::fromUtf8(file.readAll());
    };

    /**
     * @brief 读取并拼接多个 QSS 文件
     * @param qrcPaths 资源路径列表
     * @return 合并后的 QSS 字符串
     */
    static QString loadMultiple(const QStringList &qrcPaths) {
        QString combined;
        for (const auto &path: qrcPaths) {
            combined += load(path);
            combined += QChar('\n');
        }
        return combined;
    };

    /**
     * @brief 将指定 QSS 文件应用到 Widget
     * @param widget 目标控件
     * @param qrcPath 资源路径
     */
    static void apply(QWidget *widget, const QString &qrcPath) {
        widget->setStyleSheet(load(qrcPath));
    };

    /**
     * @brief 将多个 QSS 文件合并后应用到 Widget
     * @param widget 目标控件
     * @param qrcPaths 资源路径列表
     */
    static void applyMultiple(QWidget *widget, const QStringList &qrcPaths) {
        widget->setStyleSheet(loadMultiple(qrcPaths));
    };
};
