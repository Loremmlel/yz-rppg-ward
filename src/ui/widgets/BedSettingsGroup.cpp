#include "BedSettingsGroup.h"
#include "../../service/BedService.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonObject>

BedSettingsGroup::BedSettingsGroup(QWidget *parent)
    : QGroupBox(QStringLiteral("床位配置"), parent) {
    initUI();

    auto *svc = BedService::instance();
    connect(svc, &BedService::wardsFetched,  this, &BedSettingsGroup::onWardsFetched);
    connect(svc, &BedService::roomsFetched,  this, &BedSettingsGroup::onRoomsFetched);
    connect(svc, &BedService::bedsFetched,   this, &BedSettingsGroup::onBedsFetched);
    connect(svc, &BedService::errorOccurred, this, &BedSettingsGroup::onBedServiceError);
}

void BedSettingsGroup::initUI() {
    auto *form = new QFormLayout(this);
    form->setContentsMargins(20, 20, 20, 20);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(20);
    form->setVerticalSpacing(16);

    m_wardCombo = new QComboBox(this);
    m_wardCombo->setObjectName("SettingsInput");
    m_wardCombo->setMinimumWidth(280);
    m_wardCombo->setPlaceholderText(QStringLiteral("请选择病区"));

    m_roomCombo = new QComboBox(this);
    m_roomCombo->setObjectName("SettingsInput");
    m_roomCombo->setMinimumWidth(280);
    m_roomCombo->setPlaceholderText(QStringLiteral("请先选择病区"));
    m_roomCombo->setEnabled(false);

    m_bedCombo = new QComboBox(this);
    m_bedCombo->setObjectName("SettingsInput");
    m_bedCombo->setMinimumWidth(280);
    m_bedCombo->setPlaceholderText(QStringLiteral("请先选择病房"));
    m_bedCombo->setEnabled(false);

    form->addRow(QStringLiteral("病区："), m_wardCombo);
    form->addRow(QStringLiteral("病房："), m_roomCombo);
    form->addRow(QStringLiteral("床位："), m_bedCombo);

    // 刷新按钮 + 状态提示
    auto *actionLayout = new QHBoxLayout();
    m_refreshBtn = new QPushButton(QStringLiteral("刷新"), this);
    m_refreshBtn->setObjectName("PrimaryButton");
    m_refreshBtn->setFixedSize(80, 32);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("BedStatusLabel");

    actionLayout->addWidget(m_refreshBtn);
    actionLayout->addWidget(m_statusLabel);
    actionLayout->addStretch();
    form->addRow(QString(), actionLayout);

    connect(m_refreshBtn, &QPushButton::clicked,           this, &BedSettingsGroup::onRefreshClicked);
    connect(m_wardCombo,  &QComboBox::currentTextChanged,  this, &BedSettingsGroup::onWardChanged);
    connect(m_roomCombo,  &QComboBox::currentTextChanged,  this, &BedSettingsGroup::onRoomChanged);
}

// ── 公共接口 ──

void BedSettingsGroup::loadConfig(const AppConfig &cfg) {
    m_savedWardCode = cfg.wardCode;
    m_savedRoomNo   = cfg.roomNo;
    m_savedBedId    = cfg.bedId;
    startLoading();
}

QString BedSettingsGroup::wardCode() const {
    return m_wardCombo->currentText();
}

QString BedSettingsGroup::roomNo() const {
    return m_roomCombo->currentText();
}

qint64 BedSettingsGroup::bedId() const {
    return (m_bedCombo->currentIndex() >= 0)
               ? m_bedCombo->currentData().toLongLong()
               : -1;
}

// ── 内部 ──

void BedSettingsGroup::startLoading() {
    m_isRestoringSelection = true;
    m_statusLabel->setText(QStringLiteral("正在加载病区列表…"));
    BedService::instance()->fetchWards();
}

void BedSettingsGroup::onWardChanged(const QString &wardCode) {
    m_roomCombo->clear();
    m_roomCombo->setEnabled(false);
    m_roomCombo->setPlaceholderText(QStringLiteral("正在加载…"));
    m_bedCombo->clear();
    m_bedCombo->setEnabled(false);
    m_bedCombo->setPlaceholderText(QStringLiteral("请先选择病房"));

    if (!wardCode.isEmpty()) {
        BedService::instance()->fetchRooms(wardCode);
    }
}

void BedSettingsGroup::onRoomChanged(const QString &roomNo) {
    m_bedCombo->clear();
    m_bedCombo->setEnabled(false);
    m_bedCombo->setPlaceholderText(QStringLiteral("正在加载…"));

    const QString ward = m_wardCombo->currentText();
    if (!ward.isEmpty() && !roomNo.isEmpty()) {
        BedService::instance()->fetchBeds(ward, roomNo);
    }
}

void BedSettingsGroup::onWardsFetched(const QStringList &wardCodes) {
    m_wardCombo->blockSignals(true);
    m_wardCombo->clear();
    for (const auto &code : wardCodes) {
        m_wardCombo->addItem(code);
    }
    if (m_isRestoringSelection && !m_savedWardCode.isEmpty()) {
        const int idx = m_wardCombo->findText(m_savedWardCode);
        if (idx >= 0) m_wardCombo->setCurrentIndex(idx);
    } else {
        m_wardCombo->setCurrentIndex(-1);
    }
    m_wardCombo->blockSignals(false);
    m_statusLabel->clear();

    if (m_wardCombo->currentIndex() >= 0) {
        onWardChanged(m_wardCombo->currentText());
    }
}

void BedSettingsGroup::onRoomsFetched(const QJsonArray &rooms) {
    m_roomCombo->blockSignals(true);
    m_roomCombo->clear();
    for (const auto &val : rooms) {
        m_roomCombo->addItem(
            val.toObject().value(QStringLiteral("roomNo")).toString());
    }
    m_roomCombo->setEnabled(m_roomCombo->count() > 0);
    m_roomCombo->setPlaceholderText(
        m_roomCombo->count() > 0 ? QStringLiteral("请选择病房") : QStringLiteral("暂无病房"));

    if (m_isRestoringSelection && !m_savedRoomNo.isEmpty()) {
        const int idx = m_roomCombo->findText(m_savedRoomNo);
        if (idx >= 0) m_roomCombo->setCurrentIndex(idx);
    } else {
        m_roomCombo->setCurrentIndex(-1);
    }
    m_roomCombo->blockSignals(false);

    if (m_roomCombo->currentIndex() >= 0) {
        onRoomChanged(m_roomCombo->currentText());
    }
}

void BedSettingsGroup::onBedsFetched(const QJsonArray &beds) {
    m_bedCombo->blockSignals(true);
    m_bedCombo->clear();
    for (const auto &val : beds) {
        const auto obj    = val.toObject();
        const QString bedNo  = obj.value(QStringLiteral("bedNo")).toString();
        const qint64  id     = static_cast<qint64>(obj.value(QStringLiteral("id")).toDouble());
        const QString status = obj.value(QStringLiteral("status")).toString();

        QString text = bedNo;
        if (status == QStringLiteral("OCCUPIED")) {
            const auto patient = obj.value(QStringLiteral("currentPatient")).toObject();
            text += QStringLiteral("（在住：%1）")
                        .arg(patient.value(QStringLiteral("name")).toString());
        } else if (status == QStringLiteral("MAINTAINING")) {
            text += QStringLiteral("（维护中）");
        } else if (status == QStringLiteral("RESERVED")) {
            text += QStringLiteral("（已预留）");
        } else {
            text += QStringLiteral("（空闲）");
        }
        m_bedCombo->addItem(text, QVariant::fromValue(id));
    }
    m_bedCombo->setEnabled(m_bedCombo->count() > 0);
    m_bedCombo->setPlaceholderText(
        m_bedCombo->count() > 0 ? QStringLiteral("请选择床位") : QStringLiteral("暂无床位"));

    if (m_isRestoringSelection && m_savedBedId > 0) {
        for (int i = 0; i < m_bedCombo->count(); ++i) {
            if (m_bedCombo->itemData(i).toLongLong() == m_savedBedId) {
                m_bedCombo->setCurrentIndex(i);
                break;
            }
        }
        m_isRestoringSelection = false;
    } else {
        m_bedCombo->setCurrentIndex(-1);
        m_isRestoringSelection = false;
    }
    m_bedCombo->blockSignals(false);
}

void BedSettingsGroup::onBedServiceError(const QString &error) {
    m_statusLabel->setText(QStringLiteral("加载失败：%1").arg(error));
    m_isRestoringSelection = false;
}

void BedSettingsGroup::onRefreshClicked() {
    m_savedWardCode.clear();
    m_savedRoomNo.clear();
    m_savedBedId = -1;
    m_isRestoringSelection = false;

    m_wardCombo->clear();
    m_roomCombo->clear();
    m_roomCombo->setEnabled(false);
    m_bedCombo->clear();
    m_bedCombo->setEnabled(false);

    m_statusLabel->setText(QStringLiteral("正在加载病区列表…"));
    BedService::instance()->fetchWards();
}

