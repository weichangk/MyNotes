#include <QApplication>
#include "mainWindow.h"
#include "helper/langhelper.h"
#include "helper/stylehelper.h"
#include "helper/themehelper.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LangHelper::getInstance()->setSysLangLocale();
    StyleHelper::setStyleToApp(":/qss/" + ThemeHelper::currentTheme());
    MainWindow w;
    w.show();
    return a.exec();
}
