#include "menutest.h"
#include <QVBoxLayout>

MenuWidget::MenuWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

MenuWidget::~MenuWidget() {
}

void MenuWidget::createUi() {
    setObjectName("MenuWidget");
    setAttribute(Qt::WA_StyledBackground);
    QString style = R"(
        #MenuWidget { 
            background-color: #211c34;
        }
    )";
    setStyleSheet(style);

    setMinimumSize(1096, 680);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QIcon icon(":/qtmaterial/img/vcu/dark/v16/icon16/icon16_Checkmark_blue.png");

    p_mMenuBar = new QMenuBar(this);
    layout->addWidget(p_mMenuBar);
    layout->addStretch();

    m_pProjectMenu = new Menu(this);
    m_pProjectMenu->setTitle(tr("Poject"));
    m_pProjectMenu->setMinimumWidth(200);
    p_mMenuBar->addMenu(m_pProjectMenu);

    p_mOpenProjectAction = new QAction(tr("Open Project"), this);
    p_mOpenProjectAction->setCheckable(true);
    p_mOpenProjectAction->setChecked(false);
    p_mDeleteProjectAction = new QAction(tr("Delete Project"), this);
    p_mDeleteProjectAction->setCheckable(true);
    m_pProjectMenu->addAction(p_mOpenProjectAction);
    m_pProjectMenu->addSeparator();
    m_pProjectMenu->addAction(p_mDeleteProjectAction);

    m_pPreviewMenu = new Menu(this);
    m_pPreviewMenu->setTitle(tr("Preview"));
    m_pPreviewMenu->setMinimumWidth(300);
    p_mMenuBar->addMenu(m_pPreviewMenu);

    m_pPreviewAreaMenu = new Menu(m_pPreviewMenu);
    m_pPreviewAreaMenu->setTitle(tr("Preview Area"));
    m_pPreviewAreaMenu->setMinimumWidth(300);
    m_pPreviewMenu->addMenu(m_pPreviewAreaMenu);

    p_mFitAction = new QAction(tr("Fit"), this);
    p_mPercent10Action = new QAction(tr("10%"), this);
    m_pPreviewAreaMenu->addAction(p_mFitAction);
    m_pPreviewAreaMenu->addAction(p_mPercent10Action);

    m_pPreviewQualityMenu = new Menu(m_pPreviewMenu);
    m_pPreviewQualityMenu->setTitle(tr("Preview Quality"));
    m_pPreviewQualityMenu->setMinimumWidth(300);
    m_pPreviewMenu->addMenu(m_pPreviewQualityMenu);

    p_mFullAction = new QAction(tr("Full"), this);
    p_mOne_HalfAction = new QAction(tr("1/2"), this);
    m_pPreviewQualityMenu->addAction(p_mFullAction);
    m_pPreviewQualityMenu->addAction(p_mOne_HalfAction);


    p_mBtn1 = new QPushButton(this);
    p_mBtn1->setFixedSize(100, 32);
    p_mBtn1->setText("open menu1");
    layout->addWidget(p_mBtn1);

    m_pMenu1 = new Menu(this);
    p_mAction1 = new QAction(tr("menu test111"), this);
    p_mAction2 = new QAction(tr("menu test1111111111"), this);
    m_pMenu1->addAction(p_mAction1);
    m_pMenu1->addAction(p_mAction2);
}

void MenuWidget::sigConnect() {
    connect(p_mBtn1, &QPushButton::clicked, this, &MenuWidget::openMenu1);
}

void MenuWidget::openMenu1() {
    m_pMenu1->exec(QCursor::pos());
}