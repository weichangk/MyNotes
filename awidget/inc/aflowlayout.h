/*
 * @Author: weick
 * @Date: 2023-12-07 23:41:32
 * @Last Modified by: weick
 * @Last Modified time: 2024-01-14 22:08:41
 */

#pragma once
#include "awidget_global.h"
#include <QLayout>
#include <QStyle>

class AWIDGET_EXPORT AFlowLayout : public QLayout
{
    Q_OBJECT
public:
    explicit AFlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit AFlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~AFlowLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList;
    int m_hSpace;
    int m_vSpace;
};
