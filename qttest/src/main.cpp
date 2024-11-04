#include <QApplication>

#include "mainWindow.h"
#include "helper/langhelper.h"
#include "helper/themehelper.h"
#include "helper/fonthelper.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LangHelper::getInstance()->setSysLangLocale();
    ThemeHelper::setTheme(ThemeHelper::LIGHT);
    FontHelper::setFont(FontHelper::ROBOTO_REGULAR);
    FontHelper::setIconFont(FontHelper::IconFont);
    MainWindow w;
    w.show();
    return a.exec();
}
