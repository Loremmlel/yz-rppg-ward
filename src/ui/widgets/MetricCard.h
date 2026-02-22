#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

class MetricCard : public QFrame {
    Q_OBJECT
public:
    explicit MetricCard(const QString& title, const QString& emoji, const QString& color, QWidget *parent = nullptr);

    void setValue(const QString& value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
    QString m_baseColor;
};
