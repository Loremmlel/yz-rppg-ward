#include "SettingsPage.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonObject>

#include "../../service/ConfigService.h"
#include "../../service/BedService.h"
#include "../../util/StyleLoader.h"

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    initUI();
    loadCurrentConfig();
    StyleLoader::apply(this, QStringLiteral(":/styles/settings.qss"));

    // ── 连接 BedService 信号 ──
    auto *bedSvc = BedService::instance();
    connect(bedSvc, &BedService::wardsFetched,   this, &SettingsPage::onWardsFetched);
    connect(bedSvc, &BedService::roomsFetched,   this, &SettingsPage::onRoomsFetched);
    connect(bedSvc, &BedService::bedsFetched,    this, &SettingsPage::onBedsFetched);
    connect(bedSvc, &BedService::errorOccurred,  this, &SettingsPage::onBedServiceError);
}

void SettingsPage::initUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(40, 40, 40, 40);

    // ——— 网络配置分组 ———
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

    // ——— 床位配置分组 ———
    auto *bedGroup = new QGroupBox(QStringLiteral("床位配置"), this);
    auto *bedForm = new QFormLayout(bedGroup);
    bedForm->setContentsMargins(20, 20, 20, 20);
    bedForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    bedForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    bedForm->setHorizontalSpacing(20);
    bedForm->setVerticalSpacing(16);

    m_wardCombo = new QComboBox(bedGroup);
    m_wardCombo->setObjectName("SettingsInput");
    m_wardCombo->setMinimumWidth(280);
    m_wardCombo->setPlaceholderText(QStringLiteral("请选择病区"));

    m_roomCombo = new QComboBox(bedGroup);
    m_roomCombo->setObjectName("SettingsInput");
    m_roomCombo->setMinimumWidth(280);
    m_roomCombo->setPlaceholderText(QStringLiteral("请先选择病区"));
    m_roomCombo->setEnabled(false);

    m_bedCombo = new QComboBox(bedGroup);
    m_bedCombo->setObjectName("SettingsInput");
    m_bedCombo->setMinimumWidth(280);
    m_bedCombo->setPlaceholderText(QStringLiteral("请先选择病房"));
    m_bedCombo->setEnabled(false);

    bedForm->addRow(QStringLiteral("病区："), m_wardCombo);
    bedForm->addRow(QStringLiteral("病房："), m_roomCombo);
    bedForm->addRow(QStringLiteral("床位："), m_bedCombo);

    // 刷新按钮 + 状态提示
    auto *bedActionLayout = new QHBoxLayout();
    m_refreshBtn = new QPushButton(QStringLiteral("刷新"), bedGroup);
    m_refreshBtn->setObjectName("PrimaryButton");
    m_refreshBtn->setFixedSize(80, 32);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);

    m_bedStatusLabel = new QLabel(bedGroup);
    m_bedStatusLabel->setObjectName("BedStatusLabel");

    bedActionLayout->addWidget(m_refreshBtn);
    bedActionLayout->addWidget(m_bedStatusLabel);
    bedActionLayout->addStretch();
    bedForm->addRow(QString(), bedActionLayout);

    outerLayout->addWidget(bedGroup);
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

    // ── 信号连接 ──
    connect(m_saveBtn,    &QPushButton::clicked, this, &SettingsPage::onSaveClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &SettingsPage::onRefreshClicked);

    connect(m_wardCombo, &QComboBox::currentTextChanged, this, &SettingsPage::onWardChanged);
    connect(m_roomCombo, &QComboBox::currentTextChanged, this, &SettingsPage::onRoomChanged);
    connect(m_bedCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPage::onBedChanged);
}

void SettingsPage::loadCurrentConfig() {
    const AppConfig cfg = ConfigService::instance()->config();
    m_hostEdit->setText(cfg.serverHost);
    m_portSpin->setValue(cfg.serverPort);

    // 暂存床位选择，等级联加载完成后恢复
    m_savedWardCode = cfg.wardCode;
    m_savedRoomNo   = cfg.roomNo;
    m_savedBedId    = cfg.bedId;

    // 开始级联加载
    loadBedSelection();
}

void SettingsPage::loadBedSelection() {
    m_isRestoringSelection = true;
    m_bedStatusLabel->setText(QStringLiteral("正在加载病区列表…"));
    BedService::instance()->fetchWards();
}

// ── 级联：病区变化 → 加载病房 ──
void SettingsPage::onWardChanged(const QString &wardCode) {
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

// ── 级联：病房变化 → 加载床位 ──
void SettingsPage::onRoomChanged(const QString &roomNo) {
    m_bedCombo->clear();
    m_bedCombo->setEnabled(false);
    m_bedCombo->setPlaceholderText(QStringLiteral("正在加载…"));

    const QString wardCode = m_wardCombo->currentText();
    if (!wardCode.isEmpty() && !roomNo.isEmpty()) {
        BedService::instance()->fetchBeds(wardCode, roomNo);
    }
}

void SettingsPage::onBedChanged(int /*index*/) {
    // 床位选择变化，无需额外操作，保存时读取
}

// ── BedService 回调 ──
void SettingsPage::onWardsFetched(const QStringList &wardCodes) {
    // 阻塞信号，防止 addItem 触发 currentTextChanged 级联
    m_wardCombo->blockSignals(true);
    m_wardCombo->clear();
    for (const auto &code : wardCodes) {
        m_wardCombo->addItem(code);
    }

    // 恢复已保存的选择
    if (m_isRestoringSelection && !m_savedWardCode.isEmpty()) {
        const int idx = m_wardCombo->findText(m_savedWardCode);
        if (idx >= 0) {
            m_wardCombo->setCurrentIndex(idx);
        }
    } else {
        m_wardCombo->setCurrentIndex(-1); // 不选中任何项
    }
    m_wardCombo->blockSignals(false);

    m_bedStatusLabel->clear();

    // 手动触发级联加载（因为 blockSignals 期间不会触发）
    if (m_wardCombo->currentIndex() >= 0) {
        onWardChanged(m_wardCombo->currentText());
    }
}

void SettingsPage::onRoomsFetched(const QJsonArray &rooms) {
    m_roomCombo->blockSignals(true);
    m_roomCombo->clear();
    for (const auto &val : rooms) {
        const auto obj = val.toObject();
        const QString roomNo = obj.value(QStringLiteral("roomNo")).toString();
        m_roomCombo->addItem(roomNo);
    }
    m_roomCombo->setEnabled(m_roomCombo->count() > 0);
    m_roomCombo->setPlaceholderText(
        m_roomCombo->count() > 0 ? QStringLiteral("请选择病房") : QStringLiteral("暂无病房"));

    if (m_isRestoringSelection && !m_savedRoomNo.isEmpty()) {
        const int idx = m_roomCombo->findText(m_savedRoomNo);
        if (idx >= 0) {
            m_roomCombo->setCurrentIndex(idx);
        }
    } else {
        m_roomCombo->setCurrentIndex(-1);
    }
    m_roomCombo->blockSignals(false);

    if (m_roomCombo->currentIndex() >= 0) {
        onRoomChanged(m_roomCombo->currentText());
    }
}

void SettingsPage::onBedsFetched(const QJsonArray &beds) {
    m_bedCombo->blockSignals(true);
    m_bedCombo->clear();
    for (const auto &val : beds) {
        const auto obj = val.toObject();
        const QString bedNo  = obj.value(QStringLiteral("bedNo")).toString();
        const qint64  bedId  = static_cast<qint64>(obj.value(QStringLiteral("id")).toDouble());
        const QString status = obj.value(QStringLiteral("status")).toString();

        // 显示文本：床位号 + 状态
        QString displayText = bedNo;
        if (status == QStringLiteral("OCCUPIED")) {
            // 显示在住患者姓名
            const auto patient = obj.value(QStringLiteral("currentPatient")).toObject();
            const QString patientName = patient.value(QStringLiteral("name")).toString();
            displayText += QStringLiteral("（在住：%1）").arg(patientName);
        } else if (status == QStringLiteral("MAINTAINING")) {
            displayText += QStringLiteral("（维护中）");
        } else if (status == QStringLiteral("RESERVED")) {
            displayText += QStringLiteral("（已预留）");
        } else {
            displayText += QStringLiteral("（空闲）");
        }

        m_bedCombo->addItem(displayText, QVariant::fromValue(bedId));
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
        m_isRestoringSelection = false; // 级联恢复完成
    } else {
        m_bedCombo->setCurrentIndex(-1);
        m_isRestoringSelection = false;
    }
    m_bedCombo->blockSignals(false);
}

void SettingsPage::onBedServiceError(const QString &error) {
    m_bedStatusLabel->setText(QStringLiteral("加载失败：%1").arg(error));
    m_isRestoringSelection = false;
}

void SettingsPage::onRefreshClicked() {
    // 清空已保存选择状态，完全刷新
    m_savedWardCode.clear();
    m_savedRoomNo.clear();
    m_savedBedId = -1;
    m_isRestoringSelection = false;

    m_wardCombo->clear();
    m_roomCombo->clear();
    m_roomCombo->setEnabled(false);
    m_bedCombo->clear();
    m_bedCombo->setEnabled(false);

    m_bedStatusLabel->setText(QStringLiteral("正在加载病区列表…"));
    BedService::instance()->fetchWards();
}

void SettingsPage::onSaveClicked() {
    const qint64 bedId = (m_bedCombo->currentIndex() >= 0)
                             ? m_bedCombo->currentData().toLongLong()
                             : -1;

    const AppConfig newConfig(
        m_hostEdit->text().trimmed(),
        static_cast<quint16>(m_portSpin->value()),
        m_wardCombo->currentText(),
        m_roomCombo->currentText(),
        bedId
    );

    ConfigService::instance()->saveConfig(newConfig);

    QMessageBox::information(this,
                             QStringLiteral("保存成功"),
                             QStringLiteral("配置已保存并立即生效。"));
}
