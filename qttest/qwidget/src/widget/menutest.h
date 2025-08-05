#pragma once
#include <QWidget>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include "widget/menu.h"

using namespace qtmaterialwidget;

class MenuWidget : public QWidget {
    Q_OBJECT

public:
    MenuWidget(QWidget *parent = nullptr);
    ~MenuWidget();

private:
    void createUi();
    void sigConnect();

    void openMenu1();

private:
    QMenuBar *p_mMenuBar = nullptr;

    Menu *m_pProjectMenu = nullptr;
    QAction *p_mOpenProjectAction = nullptr;
    QAction *p_mDeleteProjectAction = nullptr;

    Menu *m_pPreviewMenu = nullptr;
    Menu *m_pPreviewAreaMenu = nullptr;
    QAction *p_mFitAction = nullptr;
    QAction *p_mPercent10Action = nullptr;    
    Menu *m_pPreviewQualityMenu = nullptr;
    QAction *p_mFullAction = nullptr;
    QAction *p_mOne_HalfAction = nullptr;

    QPushButton *p_mBtn1 = nullptr;
    Menu *m_pMenu1 = nullptr;
    QAction *p_mAction1 = nullptr;
    QAction *p_mAction2 = nullptr;
};