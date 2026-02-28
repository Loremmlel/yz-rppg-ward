#include "SettingsPage.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../../service/ConfigService.h"
#include "../../util/StyleLoader.h"
#include "../widgets/setting/NetworkSettingsGroup.h"
#include "../widgets/setting/BedSettingsGroup.h"

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    initUI();
    loadCurrentConfig();
    StyleLoader::apply(this, QStringLiteral(":/styles/settings.qss"));
}

void SettingsPage::initUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(40, 40, 40, 40);

    m_networkGroup = new NetworkSettingsGroup(this);
    m_bedGroup = new BedSettingsGroup(this);

    outerLayout->addWidget(m_networkGroup);
    outerLayout->addWidget(m_bedGroup);
    outerLayout->addStretch();

    // ——— 底部按钮栏 ———
    auto *bottomBar = new QHBoxLayout();
    bottomBar->addStretch();

    m_saveBtn = new QPushButton(QStringLiteral("保存"), this);
    m_saveBtn->setObjectName("PrimaryButton");
    m_saveBtn->setFixedSize(100, 36);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    bottomBar->addWidget(m_saveBtn);

    outerLayout->addLayout(bottomBar);

    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsPage::onSaveClicked);
}

void SettingsPage::loadCurrentConfig() {
    const AppConfig cfg = ConfigService::instance()->config();
    m_networkGroup->loadConfig(cfg);
    m_bedGroup->loadConfig(cfg);
}

void SettingsPage::onSaveClicked() {
    const AppConfig newConfig(
        m_networkGroup->host(),
        m_networkGroup->port(),
        m_bedGroup->wardCode(),
        m_bedGroup->roomNo(),
        m_bedGroup->bedId()
    );

    ConfigService::instance()->saveConfig(newConfig);

    QMessageBox::information(this,
                             QStringLiteral("保存成功"),
                             QStringLiteral("配置已保存并立即生效。"));
}
