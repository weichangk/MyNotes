#pragma once
#include "qtmaterial_global.h"
#include "fourstateimage.h"
#include <QWidget>
#include <QLabel>

namespace widget {
class QTMATERIAL_EXPORT IconText : public QWidget {
    Q_OBJECT
public:
    IconText(QWidget *parent = nullptr);
    ~IconText();
    FourStateImage *getIcon() {
        return m_Icon;
    }
    QLabel *getText() {
        return m_Text;
    }
    void setState(FourStateImage::StyleStatus state);
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
    FourStateImage::StyleStatus m_State = FourStateImage::StyleStatus::Normal;
    FourStateImage *m_Icon = nullptr;
    QLabel *m_Text = nullptr;
    int m_IconTextSpacing = 6;
};
} // namespace widget