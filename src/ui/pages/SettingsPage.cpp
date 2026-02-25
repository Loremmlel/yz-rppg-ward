#include "SettingsPage.h"

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    initUI();
}

void SettingsPage::initUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *placeholder = new QLabel(QStringLiteral("设置页面"), this);
    placeholder->setAlignment(Qt::AlignCenter);

    // 可以添加简单的样式让它看起来更正式一点
    placeholder->setStyleSheet("QLabel { font-size: 24px; color: #6c757d; }");

    mainLayout->addWidget(placeholder);
}

