#include "VitalCard.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QFile>
#include <QPixmap>

#include "../../../util/StyleLoader.h"

VitalCard::VitalCard(const QString &title, const QString &icon, QWidget *parent)
    : QFrame(parent)
{
    // objectName 与 QSS 选择器绑定，修改时需同步更新样式文件
    this->setObjectName("VitalCard");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->setFrameShape(QFrame::StyledPanel);
    StyleLoader::apply(this, QStringLiteral(":/styles/vital_card.qss"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(15);

    auto *leftContainer = new QVBoxLayout();
    leftContainer->setSpacing(5);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("vitalIconLabel");
    setIcon(icon);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("vitalTitleLabel");

    leftContainer->addWidget(m_iconLabel, 0, Qt::AlignLeft);
    leftContainer->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setObjectName("vitalValueLabel");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addLayout(leftContainer);
    layout->addStretch();
    layout->addWidget(m_valueLabel);
}

void VitalCard::setIcon(const QString &iconStr) const {
    if (iconStr.isEmpty()) return;

    if (iconStr.startsWith(":/") || QFile::exists(iconStr)) {
        QPixmap pixmap(iconStr);
        if (!pixmap.isNull()) {
            // 统一缩放到 32px，保证不同尺寸图标资产在卡片中视觉一致
            m_iconLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_iconLabel->setText("");
            return;
        }
    }

    // 路径无效或图片加载失败时，将原始字符串作为文本（如 Emoji）渲染
    m_iconLabel->setPixmap(QPixmap());
    m_iconLabel->setText(iconStr);
}

void VitalCard::setValue(const QString &value) const {
    m_valueLabel->setText(value);
}
