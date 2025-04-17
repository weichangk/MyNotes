#include <QApplication>

#include "mainWindow.h"
#include "core/lang.h"
#include "core/theme.h"
#include "core/font.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    qtmaterialcore::Lang::getInstance()->setSysLangLocale();
    QStringList prefixs;
    prefixs.append("");
    qtmaterialcore::Theme::setTheme(qtmaterialcore::Theme::LIGHT, prefixs);
    qtmaterialcore::Font::setFont(qtmaterialcore::Font::ROBOTO_REGULAR);
    qtmaterialcore::Font::setIconFont(":/font/QtMaterialIconFont.ttf");
    MainWindow w;
    w.show();
    return a.exec();
}
