#ifndef NEBULACENTER_H
#define NEBULACENTER_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QFileInfo>

class NebulaPlayer;
class DataManager;
class MprisHandler;

class NebulaCenter : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY infoChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY infoChanged)
    Q_PROPERTY(QString coverArt READ coverArt NOTIFY infoChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY stateChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY infoChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY posChanged)
    Q_PROPERTY(QStringList playlistNames READ playlistNames NOTIFY listsChanged)
    Q_PROPERTY(QStringList currentTrackList READ currentTrackList NOTIFY listsChanged)
    Q_PROPERTY(QString currentPlaylistName READ currentPlaylistName NOTIFY listsChanged)

public:
    explicit NebulaCenter(QObject *parent = nullptr);
    Q_INVOKABLE void playIndex(int index);
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void prev();
    Q_INVOKABLE void setPosition(qint64 pos);
    Q_INVOKABLE void createPlaylist(QString name);
    Q_INVOKABLE void switchPlaylist(QString name);
    Q_INVOKABLE void addToCurrentPlaylist(QUrl url);

    QString title() const;
    QString artist() const;
    QString coverArt() const;
    bool isPlaying() const;
    qint64 duration() const;
    qint64 position() const;
    QStringList playlistNames() const;
    QString currentPlaylistName() const;
    QStringList currentTrackList() const;

signals:
    void infoChanged();
    void stateChanged();
    void posChanged();
    void listsChanged();

private:
    void saveAll();
    NebulaPlayer *m_player;
    DataManager *m_data;
    MprisHandler *m_mpris;
    QMap<QString, QStringList> m_playlists;
    QString m_activeList = "General";
    int m_currentIndex = -1;
    QVariantMap m_currentMetadata;
};
#endif