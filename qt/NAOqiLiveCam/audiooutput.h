/**
 * @author Takuji Kawata
 * Updated 2015/05/07
 */
#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QIODevice>
#include <QAudioOutput>
#include <QThread>
#include <QSemaphore>

#include "NAOqi/nao_interface/nao_interface.h"

class AudioOutputWorkerThread;

class AudioOutput : QObject, public NAOqiToPCAudioInterface
{
    Q_OBJECT
public:
    AudioOutput();
    ~AudioOutput();

    void startPlay();
    void stopPlay();

    virtual void writeData(const short *data, int samples);

private:
    void initializeAudio();
    void createAudioOutput();

private:
    QAudioDeviceInfo        m_device;
    QAudioOutput*           m_audioOutput;
    QIODevice*              m_outputDevice; // not owned
    QAudioFormat            m_format;
    AudioOutputWorkerThread *m_workderThread;

private slots:
    void stateChanged(QAudio::State state);
    void deviceChanged(int index);
};


class AudioOutputWorkerThread : public QThread
{
    Q_OBJECT

public:
    AudioOutputWorkerThread();
    virtual ~AudioOutputWorkerThread();

    void    run();
    void    setOutputDevice(QIODevice *output) { m_outputDevice = output; }
    void    writeAudioBuffer(const signed short *buffer, int numSamples);

private:
    bool                        m_quit;
    char                        *m_buffers[2];
    int                         m_numSamples;
    QIODevice                   *m_outputDevice;
    QSemaphore                  m_sem;
};
#endif

