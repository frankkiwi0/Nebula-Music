#ifndef NEBULAPLAYER_H
#define NEBULAPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QVariantList>
#include <QTimer>

class NebulaPlayer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList audioLevels READ audioLevels NOTIFY audioLevelsChanged)

public:
    explicit NebulaPlayer(QObject *parent = nullptr);
    void play();
    void pause();
    void playPause();
    void stop();
    void setSource(const QUrl &source);
    void setPosition(qint64 pos);
    bool isPlaying() const;
    qint64 duration() const;
    qint64 position() const;

    QVariantList audioLevels() const { return m_audioLevels; }

signals:
    void positionChanged();
    void durationChanged();
    void stateChanged();
    void mediaStatusChanged();
    void sourceChanged(const QUrl &source);
    void audioLevelsChanged(); 
private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QVariantList m_audioLevels;
    QTimer *m_visualizerTimer;
    void updateVisualizer();
};

#endif