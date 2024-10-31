#include <QApplication>
#include "mainWindow.h"
#include "helper/langhelper.h"
#include "helper/themehelper.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LangHelper::getInstance()->setSysLangLocale();
    ThemeHelper::setTheme(ThemeHelper::LIGHT);
    MainWindow w;
    w.show();
    return a.exec();
}
