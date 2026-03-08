#include "AppDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMainWindow>
#include <QScrollArea>

// ── 单例 ─────────────────────────────────────────────────────────────────────
AppDialog *AppDialog::instance() {
    static AppDialog s_instance;
    return &s_instance;
}

AppDialog::AppDialog(QObject *parent) : QObject(parent) {
}

void AppDialog::setParentWidget(QWidget *parent) {
    // 若传入的是 QMainWindow，使用 centralWidget 作为实际父控件，
    // 这样 rect() 和 move() 坐标系与可视内容区完全一致。
    if (auto *mw = qobject_cast<QMainWindow *>(parent))
        m_parentWidget = mw->centralWidget();
    else
        m_parentWidget = parent;
}

// ── 公开接口 ─────────────────────────────────────────────────────────────────
void AppDialog::showInfo(const QString &title, const QString &content) {
    show(title, content, {
             {QStringLiteral("知道了"), [this] { close(); }}
         });
}

void AppDialog::showConfirm(const QString &title,
                            const QString &content,
                            std::function<void()> onConfirm,
                            const QString &confirmText,
                            const QString &cancelText) {
    show(title, content, {
             {cancelText, [this] { close(); }},
             {
                 confirmText, [this, onConfirm = std::move(onConfirm)] {
                     close();
                     onConfirm();
                 }
             }
         });
}

// ── 核心显示逻辑 ─────────────────────────────────────────────────────────────
void AppDialog::show(const QString &title,
                     const QString &content,
                     const QList<QPair<QString, std::function<void()> > > &buttons) {
    if (!m_parentWidget) return;

    // 关闭已有对话框
    close();

    // ── 遮罩层 ──
    m_overlay = new QWidget(m_parentWidget);
    m_overlay->setObjectName("dialogOverlay");
    m_overlay->setGeometry(m_parentWidget->rect());
    m_overlay->show();
    m_overlay->raise();

    // ── 对话框框架 ──
    m_dialog = new QFrame(m_parentWidget);
    m_dialog->setObjectName("AppDialog");
    m_dialog->setFixedWidth(420);

    auto *dialogLayout = new QVBoxLayout(m_dialog);
    dialogLayout->setContentsMargins(28, 24, 28, 20);
    dialogLayout->setSpacing(0);

    // ── 标题 ──
    m_titleLabel = new QLabel(title, m_dialog);
    m_titleLabel->setObjectName("dialogTitle");
    m_titleLabel->setWordWrap(true);
    dialogLayout->addWidget(m_titleLabel);

    // ── 分隔线 ──
    auto *divider = new QFrame(m_dialog);
    divider->setObjectName("dialogDivider");
    divider->setFrameShape(QFrame::HLine);
    dialogLayout->addWidget(divider);

    // ── 内容（可滚动，支持长文本） ──
    auto *contentScroll = new QScrollArea(m_dialog);
    contentScroll->setObjectName("dialogContentScroll");
    contentScroll->setWidgetResizable(true);
    contentScroll->setFrameShape(QFrame::NoFrame);
    contentScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    contentScroll->setMaximumHeight(360); // 超出时出现滚动条

    m_contentLabel = new QLabel(content, contentScroll);
    m_contentLabel->setObjectName("dialogContent");
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setTextFormat(Qt::RichText);
    m_contentLabel->setOpenExternalLinks(false);
    m_contentLabel->setFixedWidth(420 - 28 * 2 - 20); // 对话框宽 - 左右边距 - 滚动条预留
    m_contentLabel->adjustSize();
    contentScroll->setWidget(m_contentLabel);
    dialogLayout->addWidget(contentScroll);

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
        } else {
            btn->setObjectName("dialogSecondaryButton");
        }

        // 捕获 callback 副本并在点击时执行
        connect(btn, &QPushButton::clicked, m_dialog, [cb = callback] { cb(); });
        btnLayout->addWidget(btn);
    }

    dialogLayout->addWidget(m_buttonRow);

    m_dialog->show(); // show 先于 adjustSize，让 Qt 完成布局再计算尺寸
    m_dialog->adjustSize();
    m_dialog->raise();
    updateGeometry();

    // ── 入场动画：淡入 ──
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
    if (m_overlay) {
        m_overlay->deleteLater();
        m_overlay = nullptr;
    }
    if (m_dialog) {
        m_dialog->deleteLater();
        m_dialog = nullptr;
    }
    m_titleLabel = nullptr;
    m_contentLabel = nullptr;
    m_buttonRow = nullptr;
}

// ── 居中定位 ──────────────────────────────────────────────────────────────────
void AppDialog::updateGeometry() const {
    if (!m_dialog || !m_parentWidget) return;
    // m_dialog 是 m_parentWidget 的子 widget，坐标系相同
    const QRect pr = m_parentWidget->rect();
    const QSize ds = m_dialog->size(); // adjustSize 后已有正确尺寸
    const int x = (pr.width() - ds.width()) / 2;
    const int y = (pr.height() - ds.height()) / 2 - 20; // 稍微偏上
    m_dialog->move(x, y);
}