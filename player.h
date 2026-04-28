#include <QtDBus/QDBusObjectPath>
#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QList>
#include <QMap>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <taglib/fileref.h>
#include <taglib/tag.h>

class Player : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(QString title READ title NOTIFY trackChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY trackChanged)
    Q_PROPERTY(QString coverArt READ coverArt NOTIFY trackChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY stateChanged)
    Q_PROPERTY(QString currentPlaylistName READ currentPlaylistName NOTIFY playlistChanged)
    Q_PROPERTY(QStringList playlistNames READ playlistNames NOTIFY playlistsUpdated)
    Q_PROPERTY(QStringList currentTrackList READ currentTrackList NOTIFY playlistChanged)

public:
    explicit Player(QObject *parent = nullptr) : QObject(parent) {
        player = new QMediaPlayer(this);
        audioOutput = new QAudioOutput(this);
        player->setAudioOutput(audioOutput);

        connect(player, &QMediaPlayer::durationChanged, this, &Player::durationChanged);
        connect(player, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
        connect(player, &QMediaPlayer::playbackStateChanged, this, [=](){
            emit stateChanged();
            updateMPRIS(); 
        });
        connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status){
            if(status == QMediaPlayer::EndOfMedia) Next();
        });

        tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/nebula_cache";
        QDir().mkpath(tempDir);

        setupMPRIS();
        loadPlaylists(); 
    }

    void setupMPRIS() {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.registerService("org.mpris.MediaPlayer2.nebula")) {
            qDebug() << "Error registrando servicio MPRIS";
        }

        bus.registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAllContents);
    }

    void updateMPRIS() {
        QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", 
                                                      "org.freedesktop.DBus.Properties", 
                                                      "PropertiesChanged");
        
        QVariantMap props;
        props["PlaybackStatus"] = isPlaying() ? "Playing" : "Paused";
        props["CanGoNext"] = true;
        props["CanGoPrevious"] = true;
        props["CanPlay"] = true;
        props["CanPause"] = true;
        props["CanControl"] = true;
        
        QVariantMap metadata;
        metadata["mpris:trackid"] = QVariant::fromValue(QDBusObjectPath("/org/nebula/track/" + QString::number(currentIndex)));
        metadata["mpris:length"] = duration() * 1000; 
        metadata["xesam:title"] = title();
        metadata["xesam:artist"] = artist();
        metadata["mpris:artUrl"] = coverArt();
        
        props["Metadata"] = metadata;

        msg << "org.mpris.MediaPlayer2.Player" << props << QStringList();
        QDBusConnection::sessionBus().send(msg);
    }


    Q_INVOKABLE void Next() { next(); }
    Q_INVOKABLE void Previous() { prev(); }
    Q_INVOKABLE void Play() { if (!isPlaying()) player->play(); }
    Q_INVOKABLE void Pause() { if (isPlaying()) player->pause(); }
    Q_INVOKABLE void Stop() { player->stop(); }
    Q_INVOKABLE void PlayPause() { playPause(); }


    Q_INVOKABLE void createPlaylist(const QString &name) {
        if (name.isEmpty() || m_playlists.contains(name)) return;
        m_playlists[name] = QList<QUrl>();
        emit playlistsUpdated();
        if (m_playlists.size() == 1) switchPlaylist(name);
        savePlaylists();
    }

    Q_INVOKABLE void switchPlaylist(const QString &name) {
        if (!m_playlists.contains(name)) return;
        m_currentPlaylistKey = name;
        emit playlistChanged();
        savePlaylists();
    }

    Q_INVOKABLE void addToCurrentPlaylist(const QUrl &path) {
        m_playlists[m_currentPlaylistKey].append(path);
        emit playlistChanged();
        if (m_playlists[m_currentPlaylistKey].size() == 1 && player->playbackState() == QMediaPlayer::StoppedState) {
            playIndex(0);
        }
        savePlaylists();
    }

    Q_INVOKABLE void playIndex(int index) {
        auto &list = m_playlists[m_currentPlaylistKey];
        if (index < 0 || index >= list.size()) return;
        currentIndex = index;
        player->setSource(list[index]);
        player->play();
        extractMetadata(list[index].toLocalFile());
        emit trackChanged();
        updateMPRIS();
    }

    Q_INVOKABLE void next() { 
        auto &list = m_playlists[m_currentPlaylistKey];
        if(!list.isEmpty()) playIndex((currentIndex + 1) % list.size()); 
    }
    Q_INVOKABLE void prev() { 
        auto &list = m_playlists[m_currentPlaylistKey];
        if(!list.isEmpty()) playIndex((currentIndex - 1 + list.size()) % list.size()); 
    }
    Q_INVOKABLE void playPause() {
        if (player->playbackState() == QMediaPlayer::PlayingState) player->pause();
        else player->play();
    }
    Q_INVOKABLE void setPosition(qint64 pos) { player->setPosition(pos); }
    
    // Getters
    QString title() const { return m_title.isEmpty() ? "Nebula Player" : m_title; }
    QString artist() const { return m_artist.isEmpty() ? "Desconocido" : m_artist; }
    QString coverArt() const { return m_cover; }
    qint64 duration() const { return player->duration(); }
    qint64 position() const { return player->position(); }
    bool isPlaying() const { return player->playbackState() == QMediaPlayer::PlayingState; }
    QString currentPlaylistName() const { return m_currentPlaylistKey; }
    QStringList playlistNames() const { return m_playlists.keys(); }
    QStringList currentTrackList() const {
        QStringList names;
        if (m_playlists.contains(m_currentPlaylistKey)) {
            for (const auto &url : m_playlists[m_currentPlaylistKey]) {
                names.append(QFileInfo(url.toLocalFile()).fileName());
            }
        }
        return names;
    }

signals:
    void trackChanged();
    void durationChanged();
    void positionChanged();
    void stateChanged();
    void playlistsUpdated();
    void playlistChanged();

private:
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QMap<QString, QList<QUrl>> m_playlists;
    QString m_currentPlaylistKey;
    int currentIndex = -1;
    QString m_title, m_artist, m_cover;
    QString tempDir;

    void savePlaylists() {
        QJsonObject root;
        root["currentPlaylist"] = m_currentPlaylistKey;
        QJsonObject playlistsObj;
        for(auto it = m_playlists.begin(); it != m_playlists.end(); ++it) {
            QJsonArray tracksArray;
            for(const QUrl &url : it.value()) tracksArray.append(url.toString());
            playlistsObj[it.key()] = tracksArray;
        }
        root["playlists"] = playlistsObj;
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(configPath);
        QFile file(configPath + "/config.json");
        if(file.open(QIODevice::WriteOnly)) file.write(QJsonDocument(root).toJson());
    }

    void loadPlaylists() {
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json";
        QFile file(configPath);
        if(!file.exists() || !file.open(QIODevice::ReadOnly)) {
            createPlaylist("General");
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject root = doc.object();
        m_playlists.clear();
        QJsonObject playlistsObj = root["playlists"].toObject();
        for(auto it = playlistsObj.begin(); it != playlistsObj.end(); ++it) {
            QList<QUrl> tracks;
            QJsonArray arr = it.value().toArray();
            for(const auto &val : arr) tracks.append(QUrl(val.toString()));
            m_playlists[it.key()] = tracks;
        }
        m_currentPlaylistKey = root["currentPlaylist"].toString();
        if(m_playlists.isEmpty()) createPlaylist("General");
        if(!m_playlists.contains(m_currentPlaylistKey)) m_currentPlaylistKey = m_playlists.firstKey();
        emit playlistsUpdated();
        emit playlistChanged();
    }

    void extractMetadata(const QString &path) {
        m_title = QFileInfo(path).baseName();
        m_artist = "Desconocido";
        
        if(!m_cover.isEmpty()) {
            QFile::remove(m_cover.mid(7)); 
        }

        TagLib::FileRef f(path.toStdString().c_str());
        if (!f.isNull() && f.tag()) {
            if (!f.tag()->title().isEmpty()) m_title = QString::fromStdString(f.tag()->title().to8Bit(true));
            if (!f.tag()->artist().isEmpty()) m_artist = QString::fromStdString(f.tag()->artist().to8Bit(true));
        }

        QString coverPath = tempDir + "/current_art.jpg";
        QProcess ffmpeg;
        QStringList args;
        args << "-y" << "-i" << path << "-an" << "-vframes" << "1" << "-ss" << "00:00:01" << coverPath;
        ffmpeg.start("ffmpeg", args);
        ffmpeg.waitForFinished();

        if (QFileInfo::exists(coverPath) && QFileInfo(coverPath).size() > 0) {
            m_cover = "file://" + coverPath;
        } else {
            m_cover = "";
        }
    }
};

#endif
