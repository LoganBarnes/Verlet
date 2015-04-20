#include "wavreader.h"
#include <QIODevice>
#include <iostream>

#ifdef __APPLE__
#include <OpenAL/al.h>
//#include <OpenAL/alc.h>
#elif __linux
#include <AL/al.h>
//#include <AL/alc.h>
#endif

WAVReader::WAVReader()
{
}

WAVReader::~WAVReader()
{
}

void WAVReader::decodeFile(QIODevice *source, QByteArray &dest, ALenum &format, ALuint &freq)
{
    decodeHeader(source, format, freq);

    dest.clear();

    while (true)
    {
        QByteArray portion;
        portion = source->read(freq);
        if (portion.isEmpty())
            break;
        dest += portion;
    }
}

void WAVReader::decodeHeader(QIODevice *source, ALenum &format, ALuint &freq)
{
    findHeader(source, "RIFF");

    // size
    read<unsigned int>(source);

    findHeader(source, "WAVE");
    findHeader(source, "fmt ");

    // chunk size
    read<unsigned int>(source);
    // format type
    read<short>(source);

    short channels = read<short>(source);
    unsigned int sampleRate = read<unsigned int>(source);

    // average data per sec
    read<unsigned int>(source);
    // data per sample
    read<short>(source);

    short bitsPerSample = read<short>(source);

    findHeader(source, "data");

    // data size
    read<unsigned int>(source);

    freq = sampleRate;

    if(bitsPerSample == 8)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO8;
        else if(channels == 2)
            format = AL_FORMAT_STEREO8;
    }
    else if(bitsPerSample == 16)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO16;
        else if(channels == 2)
            format = AL_FORMAT_STEREO16;
    }

    if(!format)
        std::cout << "WAVReader::decodeHeader() failed. Unsupported format" << std::endl;
}

void WAVReader::findHeader(QIODevice *source, const char *header)
{
    int length = strlen(header);
    QByteArray data = read(source, length);

    int attempts = 100;
    while (attempts--)
    {
        if (memcmp(data.data(), header, length) == 0)
            return;
        else
        {
            data = read(source, length);
        }
    }

    std::cout << "WAVReader::findHeader() failed. No header" << std::endl;
}

template <typename T>
T WAVReader::read(QIODevice *source)
{
    T value;
    QByteArray data = read(source, sizeof(T));
    memcpy(&value, data.data(), data.size());
    return value;
}

QByteArray WAVReader::read(QIODevice *source, int length)
{
    QByteArray data = source->read(length);
    if (data.size() != length)
        std::cout << "WAVReader::read() failed. Cannot read from source." << std::endl;
    return data;
}
