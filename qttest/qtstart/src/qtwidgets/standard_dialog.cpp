#include "standard_dialog.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QErrorMessage>
#include <QWizardPage>
#include <QLineEdit>
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>

/*
Qt 标准对话框:
    - 颜色对话框
    - 文件对话框
    - 字体对话框
    - 输入对话框
    - 消息对话框
    - 进度对话框
    - 错误信息对话框
    - 向导页面
*/

void standard_dialog() {
    StandardDialogWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &StandardDialogWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

StandardDialogWidget::StandardDialogWidget(QWidget *parent) :
    QWidget(parent) {
    m_pOpenColorDialogBtn = new QPushButton(this);
    m_pOpenFileDialogBtn = new QPushButton(this);
    m_pOpenFontDialogBtn = new QPushButton(this);
    m_pOpenInputDialogBtn = new QPushButton(this);
    m_pOpenMessageBoxBtn = new QPushButton(this);
    m_pOpenProgressDialogBtn = new QPushButton(this);
    m_pOpenErrorMessageBtn = new QPushButton(this);
    m_pOpenWizardPageBtn = new QPushButton(this);

    m_pOpenColorDialogBtn->setText("颜色对话框");
    m_pOpenFileDialogBtn->setText("文件对话框");
    m_pOpenFontDialogBtn->setText("字体对话框");
    m_pOpenInputDialogBtn->setText("输入对话框");
    m_pOpenMessageBoxBtn->setText("消息对话框");
    m_pOpenProgressDialogBtn->setText("进度对话框");
    m_pOpenErrorMessageBtn->setText("错误信息对话框");
    m_pOpenWizardPageBtn->setText("向导页面");

    m_pOpenColorDialogBtn->setGeometry(10, 10, 200, 30);
    m_pOpenFileDialogBtn->setGeometry(10, 50, 200, 30);
    m_pOpenFontDialogBtn->setGeometry(10, 90, 200, 30);
    m_pOpenInputDialogBtn->setGeometry(10, 130, 200, 30);
    m_pOpenMessageBoxBtn->setGeometry(10, 170, 200, 30);
    m_pOpenProgressDialogBtn->setGeometry(10, 210, 200, 30);
    m_pOpenErrorMessageBtn->setGeometry(10, 250, 200, 30);
    m_pOpenWizardPageBtn->setGeometry(10, 290, 200, 30);

    connect(m_pOpenColorDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openColorDialog);
    connect(m_pOpenFileDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openFileDialog);
    connect(m_pOpenFontDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openFontDialog);
    connect(m_pOpenInputDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openInputDialog);
    connect(m_pOpenMessageBoxBtn, &QPushButton::clicked, this, &StandardDialogWidget::openMessageBox);
    connect(m_pOpenProgressDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openProgressDialog);
    connect(m_pOpenErrorMessageBtn, &QPushButton::clicked, this, &StandardDialogWidget::openErrorMessage);
    connect(m_pOpenWizardPageBtn, &QPushButton::clicked, this, &StandardDialogWidget::openWizardPage);
}

StandardDialogWidget::~StandardDialogWidget() {
}

void StandardDialogWidget::openColorDialog() {
    // QColor color = QColorDialog::getColor(Qt::red, this, "颜色对话框", QColorDialog::ShowAlphaChannel);

    QColorDialog dialog(Qt::red, this);               // 创建对象
    dialog.setOption(QColorDialog::ShowAlphaChannel); // 显示alpha选项
    dialog.exec();                                    // 以模态方式运行对话框
    QColor color = dialog.currentColor();             // 获取当前颜色
    qDebug() << "color: " << color;
}

void StandardDialogWidget::openFileDialog() {
    // QString fileName = QFileDialog::getOpenFileName(this, "文件对话框"), "D:", "图片文件(*png *jpg);;文本文件(*txt)"));
    // qDebug() << "fileName:" << fileName;

    QStringList fileNames = QFileDialog::getOpenFileNames(this, "文件对话框", "D:", "图片文件(*png *jpg)");
    qDebug() << "fileNames:" << fileNames;
}

void StandardDialogWidget::openFontDialog() {
    // ok用于标记是否按下了“OK”按钮
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    // 如果按下“OK”按钮，那么让“字体对话框”按钮使用新字体
    // 如果按下“Cancel”按钮，那么输出信息
    if (ok) {
        m_pOpenFontDialogBtn->setFont(font);
    } else {
        qDebug() << "没有选择字体！";
    }
}

void StandardDialogWidget::openInputDialog() {
    bool ok;
    // 获取字符串
    QString string = QInputDialog::getText(this, "输入字符串对话框", "请输入用户名：", QLineEdit::Normal, "admin", &ok);
    if (ok) {
        qDebug() << "string:" << string;
    }
    // 获取整数
    int value1 = QInputDialog::getInt(this, "输入整数对话框", "请输入-1000到1000之间的数值", 100, -1000, 1000, 10, &ok);
    if (ok) {
        qDebug() << "value1:" << value1;
    }
    // 获取浮点数
    double value2 = QInputDialog::getDouble(this, "输入浮点数对话框", "请输入-1000到1000之间的数值", 0.00, -1000, 1000, 2, &ok);
    if (ok) {
        qDebug() << "value2:" << value2;
    }
    QStringList items;
    items << "条目1" << "条目2";
    // 获取条目
    QString item = QInputDialog::getItem(this, "输入条目对话框", "请选择或输入一个条目", items, 0, true, &ok);
    if (ok) {
        qDebug() << "item:" << item;
    }
}

void StandardDialogWidget::openMessageBox() {
    // 问题对话框
    int ret1 = QMessageBox::question(this, "问题对话框", "你了解Qt吗？", QMessageBox::Yes, QMessageBox::No);
    if (ret1 == QMessageBox::Yes) {
        qDebug() << "问题！";
    }
    // 提示对话框
    int ret2 = QMessageBox::information(this, "提示对话框", "这是Qt书籍！", QMessageBox::Ok);
    if (ret2 == QMessageBox::Ok) {
        qDebug() << "提示！";
    }
    // 警告对话框
    int ret3 = QMessageBox::warning(this, "警告对话框", "不能提前结束！", QMessageBox::Abort);
    if (ret3 == QMessageBox::Abort) {
        qDebug() << "警告！";
    }
    // 错误对话框
    int ret4 = QMessageBox::critical(this, "严重错误对话框", "发现一个严重错误！现在要关闭所有文件！", QMessageBox::YesAll);
    if (ret4 == QMessageBox::YesAll) {
        qDebug() << "错误";
    }
    // 关于对话框
    QMessageBox::about(this, "关于对话框", "Qt 标准对话框的学习案例！");
}

void StandardDialogWidget::openProgressDialog() {
    QProgressDialog dialog("文件复制进度", "取消", 0, 50000, this);
    dialog.setWindowTitle("进度对话框");       // 设置窗口标题
    dialog.setWindowModality(Qt::WindowModal); // 将对话框设置为模态
    dialog.show();
    for (int i = 0; i < 50000; i++) {      // 演示复制进度
        dialog.setValue(i);                // 设置进度条的当前值
        QCoreApplication::processEvents(); // 避免界面冻结
        if (dialog.wasCanceled()) break;   // 按下取消按钮则中断
    }
    dialog.setValue(50000); // 这样才能显示100%，因为for循环中少加了一个数
    qDebug() << "复制结束！";
}

void StandardDialogWidget::openErrorMessage() {
    auto errordlg = new QErrorMessage(this);
    errordlg->setWindowTitle(tr("错误信息对话框"));
    errordlg->showMessage(tr("这里是出错信息！"));
}

void StandardDialogWidget::openWizardPage() {
    auto createPage1 = []() {
        QWizardPage *page = new QWizardPage;
        page->setTitle(tr("介绍"));
        return page;
    };

    auto createPage2 = []() {
        QWizardPage *page = new QWizardPage;
        page->setTitle(tr("用户选择信息"));
        return page;
    };

    auto createPage3 = []() {
        QWizardPage *page = new QWizardPage;
        page->setTitle(tr("结束"));
        return page;
    };

    QWizard wizard(this);
    wizard.setWindowTitle(tr("向导对话框"));
    wizard.addPage(createPage1());
    wizard.addPage(createPage2());
    wizard.addPage(createPage3());
    wizard.exec();
}
