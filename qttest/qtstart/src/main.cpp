#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget widget;
    widget.show();
    return a.exec();
}
