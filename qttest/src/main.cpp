#include <QApplication>
#include "mainWindow.h"
#include "core/langhelper.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LangHelper::getInstance()->setSysLangLocale();
    MainWindow w;
    w.show();
    return a.exec();
}
