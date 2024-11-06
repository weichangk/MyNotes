#include <QApplication>

#include "mainWindow.h"
#include "core/lang.h"
#include "core/theme.h"
#include "core/font.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    core::Lang::getInstance()->setSysLangLocale();
    core::Theme::setTheme(core::Theme::LIGHT);
    core::Font::setFont(core::Font::ROBOTO_REGULAR);
    core::Font::setIconFont(core::Font::IconFont);
    MainWindow w;
    w.show();
    return a.exec();
}
