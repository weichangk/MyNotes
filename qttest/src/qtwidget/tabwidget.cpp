#include "tabwidget.h"
#include <QVBoxLayout>

TabWidget::TabWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

TabWidget::~TabWidget() {
}

void TabWidget::createUi() {
    setWindowTitle("Tab Widget Test");
    setFixedSize(800, 600);

    auto layout = new QVBoxLayout(this);

    tab_widget_ = new QTabWidget(this);
    tab_widget_->addTab(new QWidget(), "Upscayl");
    tab_widget_->addTab(new QWidget(), "Settings");

    QString tabBarStyle = R"(
        QTabWidget::pane {
            border: none; /* No border around the tab pane */
        }

        QTabBar {
            background: #1e2430; /* Background color for the tab bar */
            border-radius: 15px; /* Rounded corners for the tab bar */
            padding: 2px; /* Padding to ensure rounded corners are visible */
        }
        
        QTabBar::tab {
            background: none;  /* No background for unselected tabs */
            color: #d4d4d4;    /* Text color */
            padding: 5px 10px; /* Padding for the tab */
            border: none;      /* No border */
            border-radius: 10px; /* Rounded corners */
            min-width: 80px;   /* Minimum width for the tabs */
        }

        QTabBar::tab:selected {
            background: #4e5a75;  /* Background color when selected */
            color: #ffffff;       /* Text color when selected */
        }

        QTabBar::tab:hover {
            background: #3e485a;  /* Background color when hovered */
            color: #ffffff;       /* Text color when hovered */
        }
    )";

    tab_widget_->setStyleSheet(tabBarStyle);
    layout->addWidget(tab_widget_);
}

void TabWidget::sigConnect() {
}