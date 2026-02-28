#include "ToastManager.h"

#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

ToastManager::ToastManager(QObject *parent) : QObject(parent) {}

ToastManager *ToastManager::instance() {
    static ToastManager s_instance;
    return &s_instance;
}

void ToastManager::setParentWidget(QWidget *parent) {
    m_parentWidget = parent;
}

void ToastManager::showToast(const QString &text, const int durationMs) {
    if (!m_parentWidget) return;

    auto *label = new QLabel(text, m_parentWidget);
    label->setObjectName("Toast");
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setStyleSheet(
        QStringLiteral(
            "QLabel#Toast {"
            "  background-color: rgba(33, 37, 41, 220);"
            "  color: #FFFFFF;"
            "  font-size: 13px;"
            "  font-weight: bold;"
            "  padding: 10px 20px;"
            "  border-radius: 6px;"
            "  border: 1px solid rgba(255,255,255,30);"
            "}")
    );
    label->adjustSize();

    // 透明度效果
    auto *opacity = new QGraphicsOpacityEffect(label);
    opacity->setOpacity(1.0);
    label->setGraphicsEffect(opacity);

    m_activeToasts.append(label);
    repositionToasts();
    label->show();

    // 淡出动画
    QTimer::singleShot(durationMs, this, [this, label, opacity] {
        auto *fadeOut = new QPropertyAnimation(opacity, "opacity", label);
        fadeOut->setDuration(400);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);

        connect(fadeOut, &QPropertyAnimation::finished, this, [this, label] {
            m_activeToasts.removeOne(label);
            label->deleteLater();
            repositionToasts();
        });

        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void ToastManager::repositionToasts() {
    if (!m_parentWidget) return;

    constexpr int margin = 16;
    constexpr int spacing = 8;
    int y = margin;

    for (auto *toast : m_activeToasts) {
        toast->adjustSize();
        const int x = m_parentWidget->width() - toast->width() - margin;
        toast->move(x, y);
        y += toast->height() + spacing;
    }
}

