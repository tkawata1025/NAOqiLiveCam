/**
 * @author Takuji Kawata
 * Updated 2015/03/27
 */

#include <iostream>
#include "audiocaptureremote.h"

#include <qi/os.hpp>
#include <alvalue/alvalue.h>
#include <alcommon/alproxy.h>
#include <alcommon/albroker.h>
#include "nao_interface.h"
//#include <alvision/alvisiondefinitions.h>

#include <pthread.h>

#define BUFFER_SIZE (AUDIOFREQUENCYHZ * 1)

static pthread_mutex_t  s_mutexAudioBuffer;

//static
static AudioCaptureRemote *s_instance = NULL;

class SimpleRingBuffer
{
public:
  SimpleRingBuffer(int size) : buffer(new char[size]), maxsize(size), top(0), bottom(0) {}
  ~SimpleRingBuffer() {
      if (buffer)
      {
          delete buffer;
      }
  }

  int top;
  int bottom;
  int maxsize;

  int size() { if (top == bottom) return 0; else return top < bottom ? bottom - top + 1: maxsize - top + bottom + 1; }

  void push(char c)
  {
      bottom++;
      if (bottom>=maxsize) bottom = 0;
      if (bottom == top)
      {
          top++;
          if (top>=maxsize) top = 0;
      }
      buffer[bottom] = c;
  }

  char pop()
  {
    if (size() == 0) return 0;

    char r = buffer[top];
    top++;
    if (top>=maxsize) top = 0;

    return r;
  }

  long long       timestamp;
  int             length;
  char            *buffer;
};

class ThreadLockHelper
{
  pthread_mutex_t *d_mutex;
public:
  ThreadLockHelper(pthread_mutex_t &mutex) : d_mutex(&mutex)
  {
    pthread_mutex_lock(d_mutex);
  }

  ~ThreadLockHelper()
  {
    pthread_mutex_unlock(d_mutex);
  }
};

#define LOCKER(mutex) ThreadLockHelper __locker(mutex);((void)__locker);



/**
 * Constructor for AVCaptureRemote object
 * @param broker The parent broker
 * @param name The name of the module
 */
AudioCaptureRemote::AudioCaptureRemote(
  boost::shared_ptr<AL::ALBroker> broker,
  const std::string& name)
: AL::ALSoundExtractor(broker, name)
, fCapturingAudio(false)
, fAudioRingBuffer(new SimpleRingBuffer(BUFFER_SIZE))
{
  pthread_mutex_init(&s_mutexAudioBuffer, NULL);

  setModuleDescription("Captures audio");

  functionName("isCapturing", "AudioCaptureRemote", "Says if the capture was started.");
  setReturn("capturing", "Whether capture was started.");
  BIND_METHOD(AudioCaptureRemote::isCapturing);

  functionName("startCapture", "AudioCaptureRemote", "Starts audio capture.");
  addParam("audio", "Whether to enable audio capture.");
  BIND_METHOD(AudioCaptureRemote::startCapture);

  functionName("stopCapture", "AVCaptureRemote", "Stops audiovisual capture.");
  BIND_METHOD(AudioCaptureRemote::stopCapture);

  std::cout << xGetTime() << ":construct AudioCaptureRemote!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

}

AudioCaptureRemote::~AudioCaptureRemote() {
  stopCapture();

  if (fAudioRingBuffer!=NULL)
  {
    delete fAudioRingBuffer;
    fAudioRingBuffer = NULL;
  }
}

void AudioCaptureRemote::init()
{
  std::cout << xGetTime() << ":init AudioCaptureRemote!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  startCapture();
}

bool AudioCaptureRemote::isCapturing()
{
  return fCapturingAudio;
}

void AudioCaptureRemote::startCapture()
{
  if(isCapturing())
    throw std::runtime_error("Capture already started.");
  xStartAudio();
}

void AudioCaptureRemote::stopCapture()
{
  if(fCapturingAudio)
    xStopAudio();

  s_instance = NULL;
}

// Audio ======================================================================
void AudioCaptureRemote::xStartAudio()
{
  try
  {
    audioDevice->callVoid("setClientPreferences",
                          getName(),                //Name of this module
                          AUDIOFREQUENCYHZ,         //48000 Hz requested
                          (int)AL::FRONTCHANNEL,    //Front Channels requested
                          0                         //Deinterleaving not requested
                          );

    this->startDetection();
    fCapturingAudio = true;
    s_instance = this;
  }
  catch(const std::exception &error)
  {
    std::cerr << "Cannot subscribe audio: " << error.what() << std::endl;
    fCapturingAudio = false;
  }
}

void AudioCaptureRemote::xStopAudio()
{
  s_instance = NULL;  
  try
  {
    this->stopDetection();
  }
  catch(const std::exception &error)
  {
    std::cerr << "Cannot unsubscribe audio." << error.what() << std::endl;
  }
  fCapturingAudio = false;
}

void AudioCaptureRemote::process(const int &pNbOfInputChannels,
                              const int &pNbrSamples,
                              const AL_SOUND_FORMAT *pData,
                              const AL::ALValue &pTimeStamp)
{
  LOCKER(s_mutexAudioBuffer);
  for (int i = 0 ; i < pNbrSamples; i++)
  {
    unsigned short v = (unsigned short) pData[i]; 
    fAudioRingBuffer->push(v  & 0xFF);
    fAudioRingBuffer->push((v >> 8) & 0xFF);
  }
}

//static
int AudioCaptureRemote::readData(char *data, int len)
{
  if (s_instance)
  {
    return s_instance->readDataInternal(data, len);
  }
  else
  {
    return 0;
  }
}

int AudioCaptureRemote::readDataInternal(char *data, int len)
{
  int bufSize = 0;
  {
    LOCKER(s_mutexAudioBuffer);
    if (fAudioRingBuffer->size() < len) len = fAudioRingBuffer->size();
    if (len % 2 == 1) len--;

    for (int i = 0; i < len ; i++)
    {
       data[i] = fAudioRingBuffer->pop();
    }
    bufSize = fAudioRingBuffer->size();
  }

  return len;
}


long long AudioCaptureRemote::xGetTime() {
  qi::os::timeval lTimeStruct;
  qi::os::gettimeofday(&lTimeStruct);
  long long lTime = (long long)lTimeStruct.tv_sec * 1000000; // Seconds
  lTime += (long long)lTimeStruct.tv_usec;                   // Microseconds
  return lTime;
}

