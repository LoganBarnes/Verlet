#ifndef AUDIO_H
#define AUDIO_H

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif __linux
#include <AL/al.h>
#include <AL/alc.h>
#elif __WIN32
#include <al.h>
#include <alc.h>
#endif

#include <glm.hpp>
#include <QHash>
#include <QSet>

class WAVReader;

class Audio
{
public:
    Audio();
    virtual ~Audio();

    void initAudio();

    ALuint getSourceID();

    void setListener(glm::vec4 pos, glm::vec3 vel, glm::vec4 look, glm::vec4 up);
    void setSource(ALuint source, QString file, glm::vec3 pos, glm::vec3 vel, bool loop);
    void playSource(ALuint source);

private:
    void loadSoundsFromDirectory();
    void loadSound(const QString &filename, const QString &key, WAVReader *wr);

    ALCdevice *m_device;
    ALCcontext *m_context;

    QHash<QString, ALint> m_buffers;
    QSet<ALuint> m_sources;

};

#endif // AUDIO_H
