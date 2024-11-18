#pragma once
#include "qtmaterial_global.h"
#include "widget/button.h"

#include <QLineEdit>
#include <QHBoxLayout>

namespace widget {
class QTMATERIAL_EXPORT LineEdit : public QLineEdit {
    Q_OBJECT
public:
    LineEdit(QWidget *parent = nullptr);
    ~LineEdit();

Q_SIGNALS:
    void sigFocusOut(const QString &);
    void sigEditingConfirm(const QString &);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
};

class QTMATERIAL_EXPORT SearchLineEdit : public QLineEdit {
    Q_OBJECT

public:
    SearchLineEdit(QWidget *parent = nullptr);
    ~SearchLineEdit() override {}

    void setObjectName(const QString &name);

    void setSearchBtnVisible(bool);
    bool searchBtnVisible() const;

    void setClearBtnVisible(bool);
    bool clearBtnVisible() const;

    void setButtonSize(int);
    int buttonSize() const;

    void setLayoutLeftRightMargin(int);
    int layoutLeftRightMargin() const;

    void setTextLeftRightMargin(int);
    int textLeftRightMargin() const;

    void setSearchVectorBtnFont(const QFont &);
    void setSearchVectorBtnText(const QString &);

    void setClearVectorBtnFont(const QFont &);
    void setClearVectorBtnText(const QString &);

protected:
    void showEvent(QShowEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void updateTextMargin();

    void slotTextChanged(const QString &);
    void slotTextClear();

private:
    QHBoxLayout *m_pLayout = nullptr;

	widget::VectorButton* m_pSearchVectorBtn = nullptr;
    widget::VectorButton* m_pClearVectorBtn = nullptr;

    int m_nBtnSize = 20;
    int m_nLayoutLeftRightMargin = 12;
    int m_nTextLeftRightMargin = 12;
};
} // namespace widget