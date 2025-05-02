#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>

namespace ns {
    class NsPushButton : public QPushButton {
        Q_OBJECT

    public:
        NsPushButton(QWidget *parent = nullptr);
        ~NsPushButton() override = default;
    };
}

class MyPushButton : public QPushButton {
    Q_OBJECT

public:
    MyPushButton(QWidget *parent = nullptr);
    ~MyPushButton() override = default;
};

class HasChildPushButton : public QPushButton {
    Q_OBJECT

public:
    HasChildPushButton(QWidget *parent = nullptr);
    ~HasChildPushButton() override = default;

    void setObjectName(const QString &name);

    void setText(const QString &text);

private:
    QLabel *m_pText = nullptr;
};

class QssTestWidget : public QWidget {
    Q_OBJECT

public:
    QssTestWidget(QWidget *parent = nullptr);
    ~QssTestWidget() override = default;

private:
    ns::NsPushButton *m_pNsPushButton = nullptr;
    ns::NsPushButton *m_pNsPushButton1 = nullptr;
    MyPushButton *m_pPushButton1 = nullptr;

    HasChildPushButton *m_pHasChildPushButton = nullptr;
};