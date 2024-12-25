#include <QApplication>

#include "mainWindow.h"
#include "core/lang.h"
#include "core/theme.h"
#include "core/font.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    qtmaterialcore::Lang::getInstance()->setSysLangLocale();
    qtmaterialcore::Theme::setTheme(qtmaterialcore::Theme::LIGHT);
    qtmaterialcore::Font::setFont(qtmaterialcore::Font::ROBOTO_REGULAR);
    qtmaterialcore::Font::setIconFont(qtmaterialcore::Font::IconFont);
    MainWindow w;
    w.show();
    return a.exec();
}
