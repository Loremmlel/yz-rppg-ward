#include "MetricCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QFile>
#include <QPixmap>

#include "../../../util/StyleLoader.h"

MetricCard::MetricCard(const QString &title, const QString &icon, QWidget *parent)
    : QFrame(parent) {
    // objectName 与 QSS 选择器绑定，修改时需同步更新样式文件
    this->setObjectName("MetricCard");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setFrameShape(QFrame::StyledPanel);
    StyleLoader::apply(this, QStringLiteral(":/styles/metric_card.qss"));

    // ── 整体纵向布局：上部信息行 + 下部趋势图区域 ──
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 15, 20, 15);
    mainLayout->setSpacing(10);

    // ── 上部：图标+名称 | 数值 ──
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(10);

    // 左侧：图标 + 名称（纵向排列）
    auto *leftGroup = new QVBoxLayout();
    leftGroup->setSpacing(4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("metricIconLabel");
    setIcon(icon);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("metricTitleLabel");

    leftGroup->addWidget(m_iconLabel, 0, Qt::AlignLeft);
    leftGroup->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    // 右侧：数值
    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setObjectName("metricValueLabel");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    topRow->addLayout(leftGroup);
    topRow->addStretch();
    topRow->addWidget(m_valueLabel);

    mainLayout->addLayout(topRow);

    // ── 下部：预留趋势折线图区域 ──
    m_trendArea = new QWidget(this);
    m_trendArea->setObjectName("trendChartPlaceholder");
    m_trendArea->setMinimumHeight(60);
    m_trendArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addWidget(m_trendArea, 1); // stretch=1 让趋势区域占据剩余空间
}

void MetricCard::setIcon(const QString &iconStr) const {
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

void MetricCard::setValue(const QString &value) const {
    m_valueLabel->setText(value);
}

