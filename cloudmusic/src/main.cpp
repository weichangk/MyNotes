/*
 * @Author: weick
 * @Date: 2024-05-21 07:32:24
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-30 08:00:31
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    engine.addImportPath("qrc:/");
    engine.addImportPath(":/weick.com/imports");

    const QUrl url(u"qrc:/weick.com/imports/CloudMusic/src/main.qml"_qs);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
