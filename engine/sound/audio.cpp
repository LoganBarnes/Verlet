#include "audio.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QByteArray>
#include <QBuffer>
#include <stdlib.h>
#include <iostream>

#include "wavreader.h"
//#include <stdint.h>

//typedef uint32_t DWORD;
//typedef unsigned char BYTE;

//#include "debugprinting.h"

Audio::Audio()
{
}


Audio::~Audio()
{
    foreach(ALuint source, m_sources)
        alDeleteSources(1, &source);                                                //Delete the OpenAL Source

    foreach(ALuint buffer, m_buffers)
        alDeleteBuffers(1, &buffer);                                                 //Delete the OpenAL Buffer

    alcMakeContextCurrent(NULL);                                                //Make no context current
    alcDestroyContext(m_context);                                                 //Destroy the OpenAL Context
    alcCloseDevice(m_device);                                                     //Close the OpenAL Device
}


void Audio::initAudio()
{
    // Initialize Open AL
    m_device = alcOpenDevice(NULL); // open default device
    if (m_device != NULL) {
        m_context = alcCreateContext(m_device, NULL); // create context
        if (m_context != NULL) {
            alcMakeContextCurrent(m_context); // set active context
        }
    }

    loadSoundsFromDirectory();
}


void Audio::setListener(glm::vec4 pos, glm::vec3 vel, glm::vec4 look, glm::vec4 up)
{
    float scaleFactor = .2f;
    pos = pos * scaleFactor;
    vel = vel * scaleFactor;

    ALfloat listenerOri[] = { look.x, look.y, look.z, up.x, up.y, up.z };

    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
    alListenerfv(AL_ORIENTATION, listenerOri);
}


void Audio::setSource(ALuint source, QString file, glm::vec3 pos, glm::vec3 vel, bool loop)
{
    ALuint bufferID = m_buffers.value(file, 0);
    if (!bufferID)
        return;

    float scaleFactor = .2f;
    pos = pos * scaleFactor;
    vel = vel * scaleFactor;

    //Source
    alSourcei(source, AL_BUFFER,   bufferID);                                 //Link the buffer to the source
    alSourcef(source, AL_PITCH,    1.0f     );                                 //Set the pitch of the source
    alSourcef(source, AL_GAIN,     1.0f     );                                 //Set the gain of the source
    alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);                                 //Set the position of the source
    alSource3f(source, AL_VELOCITY, vel.x, vel.y, vel.z);                                 //Set the velocity of the source
    if (loop)
        alSourcei(source, AL_LOOPING, AL_TRUE);
    else
        alSourcei(source, AL_LOOPING,  AL_FALSE );                                 //Set if source is looping sound
}


ALuint Audio::getSourceID()
{
    ALuint sourceID;
    alGenSources(1, &sourceID);
    m_sources.insert(sourceID);
    return sourceID;
}


void Audio::playSource(ALuint source)
{
    alSourcePlay(source);                                                       //Play the sound buffer linked to the source
    if (alGetError() != AL_NO_ERROR)
    {
        std::cout << "Error playing sound" << std::endl; //Error when playing sound
    }
//    system("PAUSE");                                                            //Pause to let the sound play
}


void Audio::loadSoundsFromDirectory()
{
    WAVReader *wr = new WAVReader();

    QDir imageDir(":/audio");
    QFileInfoList fileList = imageDir.entryInfoList();

    foreach (QFileInfo fileInfo, fileList)
    {
        QString filename = fileInfo.fileName();
        loadSound(":/audio/" + filename, filename, wr);
    }

    std::cout << "Loaded sounds:" << std::endl;

    QList<QString> keys = m_buffers.keys();
    foreach(QString filename, keys)
        std::cout << filename.toStdString() << std::endl;
}


void Audio::loadSound(const QString &filename, const QString &key, WAVReader *wr)
{
    //Loading of the WAVE file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly) || m_buffers.contains(key))
    {
        std::cout << "no sound" << std::endl;
        return;
    }

    QByteArray bytes = file.readAll();
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);

    QByteArray data;

//    decoder->setSource(&buffer);
//    decoder->decodeHeader();
//    decoder->decodeAll(mData);

    ALenum format;
    ALuint freq;
    wr->decodeFile(&buffer, data, format, freq);

    ALuint bufferID;
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, format, data.data(), data.size(), freq);
    m_buffers.insert(key, bufferID);


//    FILE *fp = NULL;                                                            //Create FILE pointer for the WAVE file
//    std::cout << file.open(fp, QIODevice::ReadOnly) << std::endl;
////    int fd = file.handle();

////    FILE *fp = NULL;                                                            //Create FILE pointer for the WAVE file
////    fp = fdopen(dup(fd),"rb");                                            //Open the WAVE file
//    if (!fp)
//    {
//        std::cout << "Failed to open file" << std::endl;                        //Could not open file
//        return;
//    }

//    //Variables to store info about the WAVE file (all of them is not needed for OpenAL)
//    char type[4];
//    DWORD size,chunkSize;
//    short formatType,channels;
//    DWORD sampleRate,avgBytesPerSec;
//    short bytesPerSample,bitsPerSample;
//    DWORD dataSize;

//    //Check that the WAVE file is OK
//    fread(type,sizeof(char),4,fp);                                              //Reads the first bytes in the file
//    if(type[0]!='R' || type[1]!='I' || type[2]!='F' || type[3]!='F')            //Should be "RIFF"
//    {
//        std::cout << "No RIFF" << std::endl;                                            //Not RIFF
//        return;
//    }

//    fread(&size, sizeof(DWORD),1,fp);                                           //Continue to read the file
//    fread(type, sizeof(char),4,fp);                                             //Continue to read the file
//    if (type[0]!='W' || type[1]!='A' || type[2]!='V' || type[3]!='E')           //This part should be "WAVE"
//    {
//        std::cout << "not WAVE" << std::endl;                                            //Not WAVE
//        return;
//    }

//    fread(type,sizeof(char),4,fp);                                              //Continue to read the file
//    if (type[0]!='f' || type[1]!='m' || type[2]!='t' || type[3]!=' ')           //This part should be "fmt "
//    {
//        std::cout << "not fmt " << std::endl;                                            //Not fmt
//        return;
//    }

//    //Now we know that the file is a acceptable WAVE file
//    //Info about the WAVE data is now read and stored
//    fread(&chunkSize,sizeof(DWORD),1,fp);
//    fread(&formatType,sizeof(short),1,fp);
//    fread(&channels,sizeof(short),1,fp);
//    fread(&sampleRate,sizeof(DWORD),1,fp);
//    fread(&avgBytesPerSec,sizeof(DWORD),1,fp);
//    fread(&bytesPerSample,sizeof(short),1,fp);
//    fread(&bitsPerSample,sizeof(short),1,fp);

//    fread(type,sizeof(char),4,fp);
//    if (type[0]!='d' || type[1]!='a' || type[2]!='t' || type[3]!='a')           //This part should be "data"
//    {
//        std::cout << "Missing DATA" << std::endl;
//        return;
//    }

//    fread(&dataSize,sizeof(DWORD),1,fp);                                        //The size of the sound data is read

//    //Display the info about the WAVE file
//    cout << "Chunk Size: " << chunkSize << endl;
//    cout << "Format Type: " << formatType << endl;
//    cout << "Channels: " << channels << endl;
//    cout << "Sample Rate: " << sampleRate << endl;
//    cout << "Average Bytes Per Second: " << avgBytesPerSec << endl;
//    cout << "Bytes Per Sample: " << bytesPerSample << endl;
//    cout << "Bits Per Sample: " << bitsPerSample << endl;
//    cout << "Data Size: " << dataSize << endl;

//    unsigned char* buf= new unsigned char[dataSize];                            //Allocate memory for the sound data
//    cout << fread(buf,sizeof(BYTE),dataSize,fp) << " bytes loaded\n";           //Read the sound data and display the
//                                                                                //number of bytes loaded.
//                                                                                //Should be the same as the Data Size if OK

//    ALuint bufferID;                                                              //Stores the sound data
//    ALuint frequency=sampleRate;                                                //The Sample Rate of the WAVE file
//    ALenum format=0;                                                            //The audio format (bits per sample, number of channels)

//    alGenBuffers(1, &bufferID);                                                    //Generate one OpenAL Buffer and link to "buffer"
//    if(alGetError() != AL_NO_ERROR)
//    {
//        std::cout << "Error GenSource" << std::endl;     //Error during buffer/source generation                                                           //The audio format (bits per sample, number of channels)
//        return;
//    }

//    //Figure out the format of the WAVE file
//    if(bitsPerSample == 8)
//    {
//        if(channels == 1)
//            format = AL_FORMAT_MONO8;
//        else if(channels == 2)
//            format = AL_FORMAT_STEREO8;
//    }
//    else if(bitsPerSample == 16)
//    {
//        if(channels == 1)
//            format = AL_FORMAT_MONO16;
//        else if(channels == 2)
//            format = AL_FORMAT_STEREO16;
//    }
//    if (!format)
//    {
//        std::cout << "Wrong BitPerSample" << std::endl;
//        return;
//    }

//    alBufferData(bufferID, format, buf, dataSize, frequency);                    //Store the sound data in the OpenAL Buffer
//    if(alGetError() != AL_NO_ERROR)
//    {
//        std::cout << "Error loading ALBuffer" << std::endl;                              //Error during buffer loading
//        return;
//    }

//    m_buffers.insert(key, bufferID);

//    //Clean-up
//    fclose(fp);                                                                 //Close the WAVE file
//    delete[] buf;
}
