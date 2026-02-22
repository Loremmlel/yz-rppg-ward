#include "MetricCard.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QFile>
#include <QPixmap>

MetricCard::MetricCard(const QString& title, const QString& icon, QWidget *parent)
    : QFrame(parent) {

    // 绑定对象名称以精准适配外部 QSS 样式定义
    this->setObjectName("MetricCard");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->setFrameShape(QFrame::StyledPanel);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(15);

    auto* leftContainer = new QVBoxLayout();
    leftContainer->setSpacing(5);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("MetricIcon");
    setIcon(icon);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("MetricTitle");

    leftContainer->addWidget(m_iconLabel, 0, Qt::AlignLeft);
    leftContainer->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setObjectName("MetricValue");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addLayout(leftContainer);
    layout->addStretch();
    layout->addWidget(m_valueLabel);
}

void MetricCard::setIcon(const QString& iconStr) {
    if (iconStr.isEmpty()) return;

    // 解析资源路径或本地文件系统路径
    if (iconStr.startsWith(":/") || QFile::exists(iconStr)) {
        QPixmap pixmap(iconStr);
        if (!pixmap.isNull()) {
            // 设置 32px 的标准尺寸，保障不同图标资产的视图连贯性
            m_iconLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_iconLabel->setText("");
            return;
        }
    }

    // 若无效路径则将原始字符串作为字符标识渲染
    m_iconLabel->setPixmap(QPixmap());
    m_iconLabel->setText(iconStr);
}

void MetricCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}
