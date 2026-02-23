#pragma once

#include <QScrollArea>
#include <QVBoxLayout>

#include "VitalCard.h"
#include "../../model/VitalData.h"

/**
 * @brief 指标监控看板
 * 维护并管理一组健康指标卡片，并通过同步 Service 获取实时数据。
 */
class VitalsWidget : public QWidget {
    Q_OBJECT

public:
    explicit VitalsWidget(QWidget *parent = nullptr);

public slots:
    /**
     * @brief 更新体征指标视图
     * @param data 最新的体征指标快照
     */
    void updateData(const VitalData &data);

private:
    void addVitalCard(const QString &key, const QString &title, const QString &iconPath);

    QVBoxLayout *m_listLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_container;

    QMap<QString, VitalCard *> m_cards;
};
