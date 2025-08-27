#include <QApplication>

#include "mainWindow.h"
#include "core/lang.h"
#include "core/theme.h"
#include "core/font.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QtmCore::Lang::getInstance()->setSysLangLocale();
    QStringList prefixs;
    prefixs.append("");
    QtmCore::Theme::setTheme(QtmCore::Theme::LIGHT, prefixs);
    QtmCore::Font::setFont(QtmCore::Font::ROBOTO_REGULAR);
    QtmCore::Font::setIconFont(":/font/QtMaterialIconFont.ttf");
    MainWindow w;
    w.show();
    return a.exec();
}
