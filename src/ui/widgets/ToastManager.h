#pragma once

#include <QObject>
#include <QLabel>

class QWidget;

/**
 * @brief 轻量级 Toast 弹出通知管理器（单例）
 *
 * 在指定父控件的右上角区域弹出短暂提示，自动淡出消失。
 * 支持多条同时显示（纵向堆叠）。
 *
 * 用法：
 * @code
 *   ToastManager::instance()->setParentWidget(mainWindow);
 *   ToastManager::instance()->showToast("操作成功");
 *   ToastManager::instance()->showToast("连接失败", 5000);
 * @endcode
 */
class ToastManager : public QObject {
    Q_OBJECT

public:
    static ToastManager *instance();

    /**
     * @brief 设置 Toast 的父控件（通常是 MainWindow）
     *
     * 必须在显示 Toast 前调用。Toast 以该控件为坐标参考。
     */
    void setParentWidget(QWidget *parent);

    /**
     * @brief 显示一条 Toast
     * @param text       显示文本
     * @param durationMs 显示持续时间（毫秒），默认 3 秒
     */
    void showToast(const QString &text, int durationMs = 3000);

private:
    explicit ToastManager(QObject *parent = nullptr);

    void repositionToasts();

    QWidget *m_parentWidget{nullptr};
    QList<QLabel *> m_activeToasts;
};
