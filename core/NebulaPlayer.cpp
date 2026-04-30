#include "NebulaPlayer.h"

#include <cmath>



NebulaPlayer::NebulaPlayer(QObject *parent) : QObject(parent) {

m_player = new QMediaPlayer(this);

m_audioOutput = new QAudioOutput(this);

m_player->setAudioOutput(m_audioOutput);




for(int i = 0; i < 32; ++i) m_audioLevels.append(0.0f);



connect(m_player, &QMediaPlayer::positionChanged, this, &NebulaPlayer::positionChanged);

connect(m_player, &QMediaPlayer::durationChanged, this, &NebulaPlayer::durationChanged);

connect(m_player, &QMediaPlayer::playbackStateChanged, this, &NebulaPlayer::stateChanged);

connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &NebulaPlayer::mediaStatusChanged);



m_visualizerTimer = new QTimer(this);

connect(m_visualizerTimer, &QTimer::timeout, this, &NebulaPlayer::updateVisualizer);

m_visualizerTimer->start(16);

}



void NebulaPlayer::updateVisualizer() {


if (m_audioLevels.isEmpty()) {

for (int i = 0; i < 32; ++i) m_audioLevels.append(0.0f);

}



if (isPlaying()) {

static float seed = 0;

seed += 0.1f;



for (int i = 0; i < 32; ++i) {

float frequency = sin(seed + (i * 0.3f)) * 0.5f + 0.5f;

float noise = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.4f;

float val = (frequency * 0.6f) + noise;


// Actualizamos el valor existente en lugar de hacer clear/append

m_audioLevels[i] = std::min(1.0f, std::max(0.0f, val));

}

} else {

for (int i = 0; i < m_audioLevels.size(); ++i) {

float val = m_audioLevels[i].toFloat();

m_audioLevels[i] = std::max(0.0f, val - 0.1f);

}

}

emit audioLevelsChanged();

}



void NebulaPlayer::setSource(const QUrl &source) {

m_player->setSource(source);

emit sourceChanged(source);

}



void NebulaPlayer::play() { m_player->play(); }

void NebulaPlayer::pause() { m_player->pause(); }

void NebulaPlayer::playPause() { isPlaying() ? pause() : play(); }

void NebulaPlayer::stop() { m_player->stop(); }

void NebulaPlayer::setPosition(qint64 pos) { m_player->setPosition(pos); }

bool NebulaPlayer::isPlaying() const { return m_player->playbackState() == QMediaPlayer::PlayingState; }

qint64 NebulaPlayer::duration() const { return m_player->duration(); }

qint64 NebulaPlayer::position() const { return m_player->position(); }