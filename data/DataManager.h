#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>

class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = nullptr);

    Q_INVOKABLE void savePlaylists(const QVariantMap &data);
    Q_INVOKABLE QVariantMap loadPlaylists();

    void extractMetadata(const QString &path);

signals:
    void metadataReady(const QVariantMap &metadata);

private:
    QString m_tempDir;
    void cleanCache();
};

#endif