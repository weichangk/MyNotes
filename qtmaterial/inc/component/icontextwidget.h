#pragma once
#include "qtmaterial_global.h"
#include "fourstateimagewidget.h"
#include <QWidget>
#include <QLabel>

class QTMATERIAL_EXPORT IconTextWidget : public QWidget {
    Q_OBJECT
public:
    IconTextWidget(QWidget *parent = nullptr);
    ~IconTextWidget();
    FourStateImageWidget *getIcon() {
        return m_Icon;
    }
    QLabel *getText() {
        return m_Text;
    }
    void setState(FourStateImageWidget::StyleStatus state);
    void setIconTextSpacing(int spacing) {
        m_IconTextSpacing = spacing;
    }

Q_SIGNALS:
    void sigClicked();

protected:
    void createUi();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    FourStateImageWidget::StyleStatus m_State = FourStateImageWidget::StyleStatus::Normal;
    FourStateImageWidget *m_Icon = nullptr;
    QLabel *m_Text = nullptr;
    int m_IconTextSpacing = 6;
};
