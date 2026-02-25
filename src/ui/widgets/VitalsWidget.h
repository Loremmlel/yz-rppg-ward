#pragma once

#include <QScrollArea>
#include <QVBoxLayout>

#include "VitalCard.h"
#include "../../model/VitalData.h"

/**
 * @brief 生命体征看板
 *
 * 管理一组 VitalCard，通过 updateData 槽接收 VitalService 分发的数据快照。
 * 卡片数量可扩展，布局通过 QScrollArea 容纳。
 */
class VitalsWidget : public QWidget {
    Q_OBJECT

public:
    explicit VitalsWidget(QWidget *parent = nullptr);

public slots:
    void updateData(const VitalData &data);

private:
    void addVitalCard(const QString &key, const QString &title, const QString &iconPath);

    QVBoxLayout *m_listLayout  {nullptr};
    QScrollArea *m_scrollArea  {nullptr};
    QWidget     *m_container   {nullptr};

    QMap<QString, VitalCard *> m_cards; ///< key 与 VitalData 字段名对应，便于 updateData 按 key 分发
};
