#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "NebulaCenter.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("NebulaSoft");
    app.setApplicationName("Nebula Music");

    NebulaCenter *backend = new NebulaCenter(&app);

    QQmlApplicationEngine engine;
    
    engine.rootContext()->setContextProperty("Backend", backend);

    const QUrl url(QString("qrc:/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    
    engine.load(url);

    return app.exec();
}