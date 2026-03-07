#include "AppDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QResizeEvent>

// ── 单例 ─────────────────────────────────────────────────────────────────────
AppDialog *AppDialog::instance() {
    static AppDialog s_instance;
    return &s_instance;
}

AppDialog::AppDialog(QObject *parent) : QObject(parent) {}

void AppDialog::setParentWidget(QWidget *parent) {
    m_parentWidget = parent;
}

// ── 公开接口 ─────────────────────────────────────────────────────────────────
void AppDialog::showInfo(const QString &title, const QString &content) {
    show(title, content, {
        { QStringLiteral("知道了"), [this]{ close(); } }
    });
}

void AppDialog::showConfirm(const QString &title,
                             const QString &content,
                             std::function<void()> onConfirm,
                             const QString &confirmText,
                             const QString &cancelText)
{
    show(title, content, {
        { cancelText,  [this]{ close(); } },
        { confirmText, [this, onConfirm = std::move(onConfirm)]{ close(); onConfirm(); } }
    });
}

// ── 核心显示逻辑 ─────────────────────────────────────────────────────────────
void AppDialog::show(const QString &title,
                     const QString &content,
                     const QList<QPair<QString, std::function<void()>>> &buttons)
{
    if (!m_parentWidget) return;

    // 关闭已有对话框
    close();

    // ── 遮罩层 ──
    m_overlay = new QWidget(m_parentWidget);
    m_overlay->setObjectName("dialogOverlay");
    m_overlay->setStyleSheet(
        "QWidget#dialogOverlay { background-color: rgba(0, 0, 0, 120); }");
    m_overlay->setGeometry(m_parentWidget->rect());
    m_overlay->show();
    m_overlay->raise();

    // ── 对话框框架 ──
    m_dialog = new QFrame(m_parentWidget);
    m_dialog->setObjectName("AppDialog");
    m_dialog->setStyleSheet(
        "QFrame#AppDialog {"
        "  background-color: #FFFFFF;"
        "  border-radius: 12px;"
        "  border: 1px solid #DEE2E6;"
        "}"
    );
    m_dialog->setFixedWidth(420);

    auto *dialogLayout = new QVBoxLayout(m_dialog);
    dialogLayout->setContentsMargins(28, 24, 28, 20);
    dialogLayout->setSpacing(0);

    // ── 标题 ──
    m_titleLabel = new QLabel(title, m_dialog);
    m_titleLabel->setObjectName("dialogTitle");
    m_titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #212529; margin-bottom: 12px;");
    m_titleLabel->setWordWrap(true);
    dialogLayout->addWidget(m_titleLabel);

    // ── 分隔线 ──
    auto *divider = new QFrame(m_dialog);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #E9ECEF; margin-bottom: 14px;");
    dialogLayout->addWidget(divider);

    // ── 内容 ──
    m_contentLabel = new QLabel(content, m_dialog);
    m_contentLabel->setObjectName("dialogContent");
    m_contentLabel->setStyleSheet(
        "font-size: 13px; color: #495057; line-height: 1.6; margin-bottom: 20px;");
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setTextFormat(Qt::RichText);
    m_contentLabel->setOpenExternalLinks(false);
    dialogLayout->addWidget(m_contentLabel);

    dialogLayout->addSpacing(6);

    // ── 按钮行 ──
    m_buttonRow = new QWidget(m_dialog);
    auto *btnLayout = new QHBoxLayout(m_buttonRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    for (int i = 0; i < buttons.size(); ++i) {
        const auto &[btnText, callback] = buttons[i];
        auto *btn = new QPushButton(btnText, m_buttonRow);
        btn->setFixedSize(90, 34);
        btn->setCursor(Qt::PointingHandCursor);

        // 最后一个按钮（主操作）用蓝色，其余用灰色
        if (i == buttons.size() - 1) {
            btn->setObjectName("dialogPrimaryButton");
            btn->setStyleSheet(
                "QPushButton#dialogPrimaryButton {"
                "  background-color: #4A90D9; color: #FFF;"
                "  border: none; border-radius: 6px;"
                "  font-size: 13px; font-weight: bold;"
                "}"
                "QPushButton#dialogPrimaryButton:hover  { background-color: #357ABD; }"
                "QPushButton#dialogPrimaryButton:pressed { background-color: #2C6EA6; }"
            );
        } else {
            btn->setObjectName("dialogSecondaryButton");
            btn->setStyleSheet(
                "QPushButton#dialogSecondaryButton {"
                "  background-color: #F8F9FA; color: #495057;"
                "  border: 1px solid #DEE2E6; border-radius: 6px;"
                "  font-size: 13px;"
                "}"
                "QPushButton#dialogSecondaryButton:hover  { background-color: #E9ECEF; }"
                "QPushButton#dialogSecondaryButton:pressed { background-color: #DEE2E6; }"
            );
        }

        // 捕获 callback 副本并在点击时执行
        connect(btn, &QPushButton::clicked, m_dialog, [cb = callback]{ cb(); });
        btnLayout->addWidget(btn);
    }

    dialogLayout->addWidget(m_buttonRow);

    m_dialog->adjustSize();
    m_dialog->show();
    m_dialog->raise();

    updateGeometry();

    // ── 入场动画：从中心略微向上弹出 + 淡入 ──
    auto *opacity = new QGraphicsOpacityEffect(m_dialog);
    m_dialog->setGraphicsEffect(opacity);
    opacity->setOpacity(0.0);

    auto *fadeIn = new QPropertyAnimation(opacity, "opacity", m_dialog);
    fadeIn->setDuration(180);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void AppDialog::close() {
    if (m_overlay) { m_overlay->deleteLater(); m_overlay = nullptr; }
    if (m_dialog)  { m_dialog->deleteLater();  m_dialog  = nullptr; }
    m_titleLabel   = nullptr;
    m_contentLabel = nullptr;
    m_buttonRow    = nullptr;
}

// ── 居中定位 ──────────────────────────────────────────────────────────────────
void AppDialog::updateGeometry() const {
    if (!m_dialog || !m_parentWidget) return;
    const QRect pr = m_parentWidget->rect();
    const QSize ds = m_dialog->sizeHint();
    m_dialog->move(
        pr.x() + (pr.width()  - ds.width())  / 2,
        pr.y() + (pr.height() - ds.height()) / 2 - 20  // 稍微偏上
    );
}

