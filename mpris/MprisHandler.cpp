#include "MprisHandler.h"
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusObjectPath>
#include <QDebug>

MprisHandler::MprisHandler(QObject *parent) : QObject(parent) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    
    if (!bus.registerService("org.mpris.MediaPlayer2.NebulaMusic")) {
        qDebug() << "Aviso: Servicio MPRIS ya registrado.";
    }

    bus.registerObject("/org/mpris/MediaPlayer2", this, 
                       QDBusConnection::ExportAllSlots | 
                       QDBusConnection::ExportAllSignals | 
                       QDBusConnection::ExportAllProperties);
}

void MprisHandler::Play() { emit requestPlay(); }
void MprisHandler::Pause() { emit requestPause(); }
void MprisHandler::PlayPause() { emit requestToggle(); }
void MprisHandler::Next() { emit requestNext(); }
void MprisHandler::Previous() { emit requestPrevious(); }
void MprisHandler::Stop() { emit requestPause(); }

void MprisHandler::Seek(qlonglong Offset) { emit requestSeek(Offset / 1000); }

void MprisHandler::SetPosition(const QDBusObjectPath &TrackId, qlonglong Position) {
    Q_UNUSED(TrackId);
    emit requestSeek(Position / 1000);
}

void MprisHandler::updateMetadata(const QVariantMap &metadata, qint64 duration) {
    m_metadata.clear();
    m_metadata["mpris:trackid"] = QVariant::fromValue(QDBusObjectPath("/org/nebula/track/0"));
    m_metadata["mpris:length"] = (qlonglong)(duration * 1000);
    m_metadata["xesam:title"] = metadata["title"].toString();
    m_metadata["xesam:artist"] = QStringList() << metadata["artist"].toString();
    m_metadata["mpris:artUrl"] = metadata["cover"].toString();

    QVariantMap changedProps;
    changedProps["Metadata"] = m_metadata;
    emitPropertiesChanged("org.mpris.MediaPlayer2.Player", changedProps);
}

void MprisHandler::updatePlaybackStatus(bool isPlaying) {
    m_status = isPlaying ? "Playing" : "Paused";
    QVariantMap changedProps;
    changedProps["PlaybackStatus"] = m_status;
    emitPropertiesChanged("org.mpris.MediaPlayer2.Player", changedProps);
}

void MprisHandler::emitPropertiesChanged(const QString &interface, const QVariantMap &changedProps) {
    QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    msg << interface << changedProps << QStringList();
    QDBusConnection::sessionBus().send(msg);
}