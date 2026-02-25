#include "SettingsPage.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

#include "../../service/ConfigService.h"

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    initUI();
    loadCurrentConfig();
}

void SettingsPage::initUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(40, 40, 40, 40);

    auto *networkGroup = new QGroupBox(QStringLiteral("网络配置"), this);
    auto *formLayout = new QFormLayout(networkGroup);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(16);

    m_hostEdit = new QLineEdit(networkGroup);
    m_hostEdit->setObjectName("SettingsInput");
    m_hostEdit->setPlaceholderText(QStringLiteral("例如：192.168.1.100"));
    m_hostEdit->setMinimumWidth(280);

    m_portSpin = new QSpinBox(networkGroup);
    m_portSpin->setObjectName("SettingsInput");
    m_portSpin->setRange(1, 65535);
    m_portSpin->setMinimumWidth(120);

    formLayout->addRow(QStringLiteral("服务器地址："), m_hostEdit);
    formLayout->addRow(QStringLiteral("端口："), m_portSpin);

    outerLayout->addWidget(networkGroup);
    outerLayout->addStretch();

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

void SettingsPage::loadCurrentConfig() const {
    const AppConfig cfg = ConfigService::instance()->config();
    m_hostEdit->setText(cfg.serverHost);
    m_portSpin->setValue(cfg.serverPort);
}

void SettingsPage::onSaveClicked() {
    const AppConfig newConfig(m_hostEdit->text().trimmed(),
                              static_cast<quint16>(m_portSpin->value()));

    ConfigService::instance()->saveConfig(newConfig);

    QMessageBox::information(this,
                             QStringLiteral("保存成功"),
                             QStringLiteral("配置已保存并立即生效。"));
}
