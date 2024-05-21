/*
 * @Author: weick 
 * @Date: 2024-05-21 07:32:24 
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-21 07:48:11
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    engine.addImportPath(":/scythestudio.com/imports");

    engine.load(QUrl(u"qrc:/scythestudio.com/imports/Superapp/main.qml"_qs));

    return app.exec();
}
