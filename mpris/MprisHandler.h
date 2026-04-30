#ifndef MPRISHANDLER_H
#define MPRISHANDLER_H

#include <QObject>
#include <QVariantMap>
#include <QtDBus/QDBusObjectPath>

class MprisHandler : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    
    Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
    Q_PROPERTY(QVariantMap Metadata READ Metadata)
    Q_PROPERTY(bool CanGoNext READ CanGoNext CONSTANT)
    Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious CONSTANT)
    Q_PROPERTY(bool CanPlay READ CanPlay CONSTANT)
    Q_PROPERTY(bool CanPause READ CanPause CONSTANT)
    Q_PROPERTY(bool CanControl READ CanControl CONSTANT)

public:
    explicit MprisHandler(QObject *parent = nullptr);

    QString PlaybackStatus() const { return m_status; }
    QVariantMap Metadata() const { return m_metadata; }
    bool CanGoNext() const { return true; }
    bool CanGoPrevious() const { return true; }
    bool CanPlay() const { return true; }
    bool CanPause() const { return true; }
    bool CanControl() const { return true; }

public slots:
    void Play(); void Pause(); void PlayPause(); void Next(); void Previous(); void Stop();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);
    void updateMetadata(const QVariantMap &metadata, qint64 duration);
    void updatePlaybackStatus(bool isPlaying);

signals:
    void requestPlay(); void requestPause(); void requestToggle(); void requestNext(); void requestPrevious();
    void requestSeek(qint64 pos);

private:
    void emitPropertiesChanged(const QString &interface, const QVariantMap &changedProps);
    QString m_status = "Stopped";
    QVariantMap m_metadata;
};

#endif