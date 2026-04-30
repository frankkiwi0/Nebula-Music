#include "DataManager.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QDebug>

DataManager::DataManager(QObject *parent) : QObject(parent) {
    m_tempDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/cache";
    QDir().mkpath(m_tempDir);
}

void DataManager::extractMetadata(const QString &path) {
    QVariantMap metadata;
    QFileInfo fileInfo(path);
    
  
    QString cleanName = fileInfo.baseName().replace("_", " ").trimmed();
    metadata["title"] = cleanName;
    metadata["artist"] = "Artista Desconocido";
    metadata["cover"] = "qrc:/assets/default_cover.png"; // Default inicial
    metadata["path"] = path;

    QString extension = fileInfo.suffix().toLower();

  
    TagLib::FileRef f(path.toStdString().c_str());
    if (!f.isNull() && f.tag()) {
        QString t = QString::fromStdString(f.tag()->title().to8Bit(true)).trimmed();
        QString a = QString::fromStdString(f.tag()->artist().to8Bit(true)).trimmed();
        if (!t.isEmpty()) metadata["title"] = t;
        if (!a.isEmpty()) metadata["artist"] = a;
    }

   
    bool coverFound = false;

    // Intento A: TagLib para MP3
    if (extension == "mp3") {
        TagLib::MPEG::File mpegFile(path.toStdString().c_str());
        if (mpegFile.isValid() && mpegFile.ID3v2Tag()) {
            auto list = mpegFile.ID3v2Tag()->frameListMap()["APIC"];
            if (!list.isEmpty()) {
                auto *pic = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(list.front());
                QString id3Cover = m_tempDir + "/id3_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";
                QFile file(id3Cover);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(reinterpret_cast<const char*>(pic->picture().data()), pic->picture().size());
                    file.close();
                    metadata["cover"] = "file://" + id3Cover;
                    coverFound = true;
                }
            }
        }
    }


    if (!coverFound) {
        cleanCache();
        QString coverPath = m_tempDir + "/art_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";
        QProcess *ffmpeg = new QProcess();
        QStringList args;

        if (extension == "mp3") {
    
            args << "-y" << "-i" << path << "-an" << "-vcodec" << "mjpeg" << "-frames:v" << "1" << coverPath;
        } else {
   
            args << "-y" << "-ss" << "7" << "-i" << path << "-vframes" << "1" << "-q:v" << "2" << coverPath;
        }

        QObject::connect(ffmpeg, &QProcess::finished, this, [=](int exitCode) {
            QVariantMap finalMeta = metadata;
            if (exitCode == 0 && QFile::exists(coverPath)) {
                finalMeta["cover"] = "file://" + coverPath;
            }
            emit metadataReady(finalMeta);
            ffmpeg->deleteLater();
        });
        ffmpeg->start("ffmpeg", args);
    } else {
        emit metadataReady(metadata);
    }
}


void DataManager::cleanCache() {
    QDir dir(m_tempDir);
    for (const QString &file : dir.entryList(QDir::Files)) {
        if (file.endsWith(".jpg")) dir.remove(file);
    }
}

void DataManager::savePlaylists(const QVariantMap &data) {
    QJsonObject root;
    root["currentPlaylist"] = data["currentPlaylist"].toString();
    QJsonObject playlistsObj;
    QVariantMap playlists = data["playlists"].toMap();
    for (auto it = playlists.begin(); it != playlists.end(); ++it) {
        QJsonArray tracksArray;
        for (const QString &url : it.value().toStringList()) tracksArray.append(url);
        playlistsObj[it.key()] = tracksArray;
    }
    root["playlists"] = playlistsObj;
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(configPath);
    QFile file(configPath + "/config.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
        file.close();
    }
}

QVariantMap DataManager::loadPlaylists() {
    QVariantMap result;
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json";
    QFile file(configPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return result;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    result["currentPlaylist"] = root["currentPlaylist"].toString();
    QVariantMap playlists;
    QJsonObject playlistsObj = root["playlists"].toObject();
    for (auto it = playlistsObj.begin(); it != playlistsObj.end(); ++it) {
        QStringList tracks;
        QJsonArray arr = it.value().toArray();
        for (const auto &val : arr) tracks.append(val.toString());
        playlists[it.key()] = tracks;
    }
    result["playlists"] = playlists;
    file.close();
    return result;
}