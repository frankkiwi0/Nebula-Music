#include "NebulaCenter.h"
#include "core/NebulaPlayer.h"
#include "data/DataManager.h"
#include "mpris/MprisHandler.h"
#include <QDebug>
#include <QFileInfo>

NebulaCenter::NebulaCenter(QObject *parent) : QObject(parent) {
    m_player = new NebulaPlayer(this);
    m_data = new DataManager(this);
    m_mpris = new MprisHandler(this);

    connect(m_data, &DataManager::metadataReady, this, [=](const QVariantMap &meta){
        m_currentMetadata = meta;
        emit infoChanged();
        m_mpris->updateMetadata(meta, m_player->duration());
    });

    connect(m_player, &NebulaPlayer::stateChanged, this, [=](){
        emit stateChanged();
        m_mpris->updatePlaybackStatus(m_player->isPlaying());
    });
    
    connect(m_player, &NebulaPlayer::positionChanged, this, &NebulaCenter::posChanged);
    connect(m_player, &NebulaPlayer::durationChanged, this, [=](){
        emit infoChanged();
        m_mpris->updateMetadata(m_currentMetadata, m_player->duration());
    });

    connect(m_mpris, &MprisHandler::requestPlay, m_player, &NebulaPlayer::play);
    connect(m_mpris, &MprisHandler::requestPause, m_player, &NebulaPlayer::pause);
    connect(m_mpris, &MprisHandler::requestToggle, m_player, &NebulaPlayer::playPause);
    connect(m_mpris, &MprisHandler::requestNext, this, &NebulaCenter::next);
    connect(m_mpris, &MprisHandler::requestPrevious, this, &NebulaCenter::prev);
    connect(m_mpris, &MprisHandler::requestSeek, m_player, &NebulaPlayer::setPosition);

    QVariantMap saved = m_data->loadPlaylists();
    if (!saved.isEmpty()) {
        m_activeList = saved["currentPlaylist"].toString();
        QVariantMap lists = saved["playlists"].toMap();
        for (auto it = lists.begin(); it != lists.end(); ++it) {
            m_playlists[it.key()] = it.value().toStringList();
        }
    }

    if (m_playlists.isEmpty()) createPlaylist("General");
    if (m_activeList.isEmpty() || !m_playlists.contains(m_activeList)) {
        m_activeList = m_playlists.firstKey();
    }
    emit listsChanged();
}

QString NebulaCenter::title() const { return m_currentMetadata["title"].toString(); }
QString NebulaCenter::artist() const { return m_currentMetadata["artist"].toString(); }
QString NebulaCenter::coverArt() const { return m_currentMetadata["cover"].toString(); }
bool NebulaCenter::isPlaying() const { return m_player->isPlaying(); }
qint64 NebulaCenter::duration() const { return m_player->duration(); }
qint64 NebulaCenter::position() const { return m_player->position(); }
QString NebulaCenter::currentPlaylistName() const { return m_activeList; }
QStringList NebulaCenter::playlistNames() const { return m_playlists.keys(); }

QStringList NebulaCenter::currentTrackList() const {
    QStringList names;
    if (m_playlists.contains(m_activeList)) {
        for (const QString &path : m_playlists[m_activeList]) {
            names.append(QFileInfo(path).fileName());
        }
    }
    return names;
}

void NebulaCenter::playPause() { m_player->playPause(); }
void NebulaCenter::setPosition(qint64 pos) { m_player->setPosition(pos); }

void NebulaCenter::playIndex(int index) {
    QStringList currentList = m_playlists[m_activeList];
    if (index >= 0 && index < currentList.size()) {
        m_currentIndex = index;
        QString path = currentList[index];
        m_player->setSource(QUrl::fromLocalFile(path));
        m_player->play();
        m_data->extractMetadata(path);
    }
}

void NebulaCenter::next() {
    if (m_playlists[m_activeList].isEmpty()) return;
    int nextIdx = (m_currentIndex + 1) % m_playlists[m_activeList].size();
    playIndex(nextIdx);
}

void NebulaCenter::prev() {
    if (m_playlists[m_activeList].isEmpty()) return;
    int prevIdx = (m_currentIndex - 1 + m_playlists[m_activeList].size()) % m_playlists[m_activeList].size();
    playIndex(prevIdx);
}

void NebulaCenter::createPlaylist(QString name) {
    if (!name.isEmpty() && !m_playlists.contains(name)) {
        m_playlists[name] = QStringList();
        emit listsChanged();
        saveAll();
    }
}

void NebulaCenter::switchPlaylist(QString name) {
    if (m_playlists.contains(name)) {
        m_activeList = name;
        emit listsChanged();
        saveAll();
    }
}

void NebulaCenter::addToCurrentPlaylist(QUrl url) {
    QString path = url.toLocalFile();
    if (!path.isEmpty()) {
        m_playlists[m_activeList].append(path);
        emit listsChanged();
        saveAll();
    }
}

void NebulaCenter::saveAll() {
    QVariantMap data;
    data["currentPlaylist"] = m_activeList;
    QVariantMap lists;
    for (auto it = m_playlists.begin(); it != m_playlists.end(); ++it) {
        lists[it.key()] = it.value();
    }
    data["playlists"] = lists;
    m_data->savePlaylists(data);
}