#pragma once
#include <QWidget>
#include <QComboBox>
#include <QAbstractItemView>

class MyComboBox : public QComboBox {
public:
    using QComboBox::QComboBox;

protected:
    void showPopup() override {
        QComboBox::showPopup();

        // 获取 view（通常是 QListView）
        QAbstractItemView* v = view();
        QPoint p = v->pos();
        // 偏移弹出位置，比如下移 10 像素
        v->move(p.x(), p.y() + 10);
    }
};

class QComboBoxTestWidget : public QWidget {
    Q_OBJECT

public:
    QComboBoxTestWidget(QWidget *parent = nullptr);
    ~QComboBoxTestWidget() {
    }

private:
    void createUi();
    void sigConnect();

private:
    MyComboBox *m_pComboBox = nullptr;
};