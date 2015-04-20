#ifndef WAVREADER_H
#define WAVREADER_H

#include <QBuffer>
#include <QByteArray>

typedef unsigned int ALuint;
typedef int ALenum;

class WAVReader
{
public:
    WAVReader();
    ~WAVReader();

    void decodeFile(QIODevice *source, QByteArray &dest, ALenum &format, ALuint &freq);

private:
    void decodeHeader(QIODevice *source, ALenum &format, ALuint &freq);
    template <typename T> T read(QIODevice *source);
    QByteArray read(QIODevice *source, int length);
    void findHeader(QIODevice *source, const char* header);
};

#endif // WAVREADER_H
