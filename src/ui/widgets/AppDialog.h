#pragma once

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <functional>

/**
 * @brief 全局应用对话框管理器（单例）
 *
 * 在主窗口上方以全屏遮罩方式显示对话框，不破坏窗口层级，不触发系统模态。
 * 支持：
 *  - 纯信息展示（showInfo）：标题 + 富文本内容 + "知道了"按钮
 *  - 确认对话框（showConfirm）：标题 + 内容 + 确认/取消按钮 + 回调
 *
 * 用法：
 * @code
 *   AppDialog::instance()->setParentWidget(mainWindow);
 *   AppDialog::instance()->showInfo("保存成功", "配置已保存并立即生效。");
 *   AppDialog::instance()->showConfirm("确认退出", "确定要退出吗？",
 *       []{ qApp->quit(); });
 * @endcode
 */
class AppDialog : public QObject {
    Q_OBJECT

public:
    static AppDialog *instance();

    /** 必须在使用前调用，绑定到 MainWindow */
    void setParentWidget(QWidget *parent);

    /**
     * @brief 显示信息对话框（仅"知道了"按钮）
     * @param title   对话框标题
     * @param content 正文（支持简单 HTML 富文本）
     */
    void showInfo(const QString &title, const QString &content);

    /**
     * @brief 显示确认对话框（确认 + 取消按钮）
     * @param title      对话框标题
     * @param content    正文
     * @param onConfirm  点击"确认"时的回调
     * @param confirmText 确认按钮文字，默认"确认"
     * @param cancelText  取消按钮文字，默认"取消"
     */
    void showConfirm(const QString &title,
                     const QString &content,
                     std::function<void()> onConfirm,
                     const QString &confirmText = QStringLiteral("确认"),
                     const QString &cancelText  = QStringLiteral("取消"));

    /** 关闭当前对话框 */
    void close();

private:
    explicit AppDialog(QObject *parent = nullptr);

    void show(const QString &title,
              const QString &content,
              const QList<QPair<QString, std::function<void()>>> &buttons);

    void updateGeometry() const;

    QWidget    *m_parentWidget{nullptr};

    // ── 覆盖层（半透明遮罩） ──
    QWidget    *m_overlay{nullptr};

    // ── 对话框容器 ──
    QFrame     *m_dialog{nullptr};
    QLabel     *m_titleLabel{nullptr};
    QLabel     *m_contentLabel{nullptr};
    QWidget    *m_buttonRow{nullptr};
};

