#include <QApplication>

#include "mainWindow.h"
#include "core/lang.h"
#include "core/theme.h"
#include "core/font.h"

using namespace core;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Lang::getInstance()->setSysLangLocale();
    Theme::setTheme(Theme::LIGHT);
    Font::setFont(Font::ROBOTO_REGULAR);
    Font::setIconFont(Font::IconFont);
    MainWindow w;
    w.show();
    return a.exec();
}
