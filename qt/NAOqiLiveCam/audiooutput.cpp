/**
 * @author Takuji Kawata
 * Updated 2015/05/07
 */

#include <QDebug>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QMutex>
#include "audiooutput.h"

static QMutex tmutex;

AudioOutput::AudioOutput()
    :   m_device(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_audioOutput(0)
    ,   m_outputDevice(0)
{
    initializeAudio();

    NaoInterface::instance()->setAudioInterface(this);
}

void AudioOutput::initializeAudio()
{

    qWarning()  << "AudioOutput::initializeAudio()";

    m_format.setFrequency(SAMPLERATE_OUT);
    m_format.setChannels(NBOFOUTPUTCHANNELS_OUT);
    m_format.setSampleSize(CHANNELBYTES*8);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }

    createAudioOutput();

    m_workderThread = new AudioOutputWorkerThread();
    m_workderThread->start();
}

void AudioOutput::createAudioOutput()
{
    qWarning() << "AudioPlayer::initializeAudio()";

    if (m_audioOutput)
        delete m_audioOutput;
    m_audioOutput = 0;
    m_audioOutput = new QAudioOutput(m_device, m_format, this);
}

AudioOutput::~AudioOutput()
{
    delete m_workderThread;
}

void AudioOutput::deviceChanged(int index)
{
    (void)index;
    m_audioOutput->stop();
    m_audioOutput->disconnect(this);
    createAudioOutput();
}


void AudioOutput::stateChanged(QAudio::State state)
{
    qWarning() << "state = " << state;
}

void AudioOutput::startPlay()
{
    m_workderThread->setOutputDevice(NULL);
    m_outputDevice = m_audioOutput->start();
    m_workderThread->setOutputDevice(m_outputDevice);
}

void AudioOutput::stopPlay()
{
    m_audioOutput->stop();
    m_audioOutput->disconnect(this);
}

void AudioOutput::writeData(const short *data, int samples)
{
    if (m_workderThread)
    {
        m_workderThread->writeAudioBuffer(data, samples);
    }
}



AudioOutputWorkerThread::AudioOutputWorkerThread() : m_quit(false), m_outputDevice(0)
{
    m_buffers[0] = new char[CHANNELBYTES * NBOFOUTPUTCHANNELS_OUT * SAMPLERATE_OUT * BUFFERSAMPLESIZEMSEC / 1000];
    m_buffers[1] = new char[CHANNELBYTES * NBOFOUTPUTCHANNELS_OUT * SAMPLERATE_OUT * BUFFERSAMPLESIZEMSEC / 1000];
    m_numSamples = 0;
}

AudioOutputWorkerThread::~AudioOutputWorkerThread()
{
    m_quit = true;
    m_sem.release();
    wait(5000);

    tmutex.lock();
    for (int i = 0; i < 2; i++)
    {
        if (m_buffers[i])
        {
            delete m_buffers[i];
            m_buffers[i] = NULL;
        }
    }
    tmutex.unlock();
}

void AudioOutputWorkerThread::run()
{
    while(!m_quit)
    {
        m_sem.acquire();
        if (!m_quit)
        {
            if (m_outputDevice)
            {
                memcpy(m_buffers[1], m_buffers[0], m_numSamples* NBOFOUTPUTCHANNELS_OUT * CHANNELBYTES);
                m_outputDevice->write(m_buffers[1],m_numSamples* NBOFOUTPUTCHANNELS_OUT * CHANNELBYTES);
            }
        }
    }
}


void AudioOutputWorkerThread::writeAudioBuffer(const short *buffer, int numSamples)
{
  tmutex.lock();

  if (m_quit)
  {
      tmutex.unlock();
      return;
  }

  int p = 0;
  int outSamples = 0;

  if (numSamples > SAMPLERATE_IN * BUFFERSAMPLESIZEMSEC / 1000)
  {
    qWarning() << "input sample is bigger than the internal buffer!! nbsamples:" << numSamples << "   internal buffer:" << (SAMPLERATE_IN * BUFFERSAMPLESIZEMSEC / 1000);
    numSamples = SAMPLERATE_IN * BUFFERSAMPLESIZEMSEC / 1000;
  }

  for (int i = 0 ; i < numSamples; i++)
  {
    short v = (short) buffer[i];
    char l = v & 0xFF;
    char h = (v & 0xFF00) >> 8;

    // Destination buffer (Qt) recieves sound with 48000 Hz, so triple the sample...
    for (int j = 0; j < SAMPLERATE_OUT/SAMPLERATE_IN ;j++)
    {
      m_buffers[0][p++] = l;
      m_buffers[0][p++] = h;
      outSamples++;
    }
  }

  m_numSamples = outSamples;
  if (m_sem.available()>1)
  {
      //qWarning() << "buffer delays..";
      return;
  }
  m_sem.release();

  tmutex.unlock();
}
