#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QtDBus/QDBusConnection> 
#include "player.h"

int main(int argc, char *argv[])
{

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme("multimedia-audio-player"));
    app.setOrganizationName("NebulaOS");
    app.setApplicationName("NebulaTahoe");

Player player;
QDBusConnection bus = QDBusConnection::sessionBus();
bus.registerService("org.mpris.MediaPlayer2.nebula");
bus.registerObject("/org/mpris/MediaPlayer2", &player, QDBusConnection::ExportAllContents);

    QQmlApplicationEngine engine;


    engine.rootContext()->setContextProperty("Backend", &player);

    const QUrl url(QStringLiteral("qrc:/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
