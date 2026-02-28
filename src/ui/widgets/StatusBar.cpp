#include "StatusBar.h"
#include "../../util/StyleLoader.h"
#include <QStyle>

StatusBar::StatusBar(QWidget *parent) : QWidget(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);

    // 初始无横幅时不占空间
    setVisible(false);

    StyleLoader::apply(this, QStringLiteral(":/styles/status_bar.qss"));
}

void StatusBar::showBanner(const QString &id, const QString &text, Severity severity) {
    auto *label = getOrCreateLabel(id);
    label->setText(text);
    label->setProperty("severity", severityClass(severity));
    label->style()->unpolish(label);
    label->style()->polish(label);
    label->setVisible(true);
    setVisible(true);
}

void StatusBar::hideBanner(const QString &id) {
    if (const auto it = m_banners.find(id); it != m_banners.end()) {
        (*it)->setVisible(false);
    }
    // 如果所有横幅都隐藏了，隐藏整个容器
    if (!hasBanners()) {
        setVisible(false);
    }
}

bool StatusBar::hasBanners() const {
    for (const auto *label: m_banners) {
        if (label->isVisible()) return true;
    }
    return false;
}

QLabel *StatusBar::getOrCreateLabel(const QString &id) {
    if (const auto it = m_banners.find(id); it != m_banners.end()) {
        return *it;
    }
    auto *label = new QLabel(this);
    label->setObjectName("StatusBanner");
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    m_layout->addWidget(label);
    m_banners.insert(id, label);
    return label;
}

QString StatusBar::severityClass(Severity severity) {
    switch (severity) {
        case Warning: return QStringLiteral("warning");
        case Error: return QStringLiteral("error");
        default: return QStringLiteral("info");
    }
}
