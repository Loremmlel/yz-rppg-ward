#include "MetricCard.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QFile>
#include <QPixmap>

MetricCard::MetricCard(const QString& title, const QString& icon, QWidget *parent)
    : QFrame(parent) {

    // 设置对象名称以便于 QSS 样式控制
    // 注意：具体实例的颜色样式将在 QSS 中通过 ID 选择器配置
    this->setObjectName("MetricCard");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->setFrameShape(QFrame::StyledPanel); // 确保样式表生效

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

    // 检查是否为资源文件或常规路径
    if (iconStr.startsWith(":/") || QFile::exists(iconStr)) {
        QPixmap pixmap(iconStr);
        if (!pixmap.isNull()) {
            // 针对一般指标卡片，图标大小设为 32x32 比较合适
            m_iconLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_iconLabel->setText(""); // 确保如果是图片，不要显示文字
            return;
        }
    }

    // 如果不是图片路径, 则作为文本（Emoji）显示
    m_iconLabel->setPixmap(QPixmap());
    m_iconLabel->setText(iconStr);
}

void MetricCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}
