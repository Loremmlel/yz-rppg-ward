#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMap>

/**
 * @brief 常驻状态横幅栏
 *
 * 在页面顶部显示零到多条常驻通知（按 ID 管理），
 * 支持 Info / Warning / Error 三种严重级别，各自对应不同配色。
 *
 * 用法：
 * @code
 *   statusBar->showBanner("no-bed", "未绑定床位", StatusBar::Warning);
 *   statusBar->hideBanner("no-bed");
 * @endcode
 */
class StatusBar : public QWidget {
    Q_OBJECT

public:
    enum Severity { Info, Warning, Error };
    Q_ENUM(Severity)

    explicit StatusBar(QWidget *parent = nullptr);

    /**
     * @brief 显示（或更新）一条常驻横幅
     * @param id       唯一标识，用于后续隐藏或更新
     * @param text     显示文本
     * @param severity 严重级别
     */
    void showBanner(const QString &id, const QString &text, Severity severity = Info);

    /** 隐藏指定 ID 的横幅，若不存在则无操作 */
    void hideBanner(const QString &id);

    /** 当前是否有可见横幅 */
    [[nodiscard]] bool hasBanners() const;

private:
    QLabel *getOrCreateLabel(const QString &id);
    static QString severityClass(Severity severity);

    QVBoxLayout *m_layout{};
    QMap<QString, QLabel *> m_banners; ///< id → label
};

