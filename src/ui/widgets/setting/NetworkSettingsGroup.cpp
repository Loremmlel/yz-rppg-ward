#include "NetworkSettingsGroup.h"
#include <QFormLayout>

NetworkSettingsGroup::NetworkSettingsGroup(QWidget *parent)
    : QGroupBox(QStringLiteral("网络配置"), parent) {
    initUI();
}

void NetworkSettingsGroup::initUI() {
    auto *form = new QFormLayout(this);
    form->setContentsMargins(20, 20, 20, 20);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(20);
    form->setVerticalSpacing(16);

    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setObjectName("SettingsInput");
    m_hostEdit->setPlaceholderText(QStringLiteral("例如：192.168.1.100"));
    m_hostEdit->setMinimumWidth(280);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setObjectName("SettingsInput");
    m_portSpin->setRange(1, 65535);
    m_portSpin->setMinimumWidth(120);

    form->addRow(QStringLiteral("服务器地址："), m_hostEdit);
    form->addRow(QStringLiteral("端口："), m_portSpin);
}

void NetworkSettingsGroup::loadConfig(const AppConfig &cfg) {
    m_hostEdit->setText(cfg.serverHost);
    m_portSpin->setValue(cfg.serverPort);
}

QString NetworkSettingsGroup::host() const {
    return m_hostEdit->text().trimmed();
}

quint16 NetworkSettingsGroup::port() const {
    return static_cast<quint16>(m_portSpin->value());
}

