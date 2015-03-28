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

// BUffer size for 0.3 sec 
#define BUFFER_SIZE (16000 * 3 * 2 * 0.3)    

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

  int   top;
  int   bottom;
  int   maxsize;
  char  *buffer;

  int size() { return top < bottom ? bottom - top: maxsize - top + bottom; }

  void push(char c)
  {
    buffer[bottom] = c;
    bottom++;
    if (bottom>=maxsize) bottom = 0;
    if (bottom == top)
    {
        //move top 2 bytes for keeping the 2 byte align
        top++;
        if (top>=maxsize) top = 0;
        top++;
        if (top>=maxsize) top = 0;
    }
  }

  char pop()
  {
    if (size() == 0) return 0;

    char r = buffer[top];
    top++;
    if (top>=maxsize) top = 0;

    return r;
  }

  bool isBufferFull() { return size() == maxsize - 1;}

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
                          16000,                    //16000 Hz requested  For NAOqi 2.0.6, it only can provide buffer with 16000Hz when you listen single channel. 48000 Hz is only supported !!
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
 
  //printf("%lld write before len:%d top:%d bottom:%d   ", xGetTime(),pNbrSamples , fAudioRingBuffer->top, fAudioRingBuffer->bottom);
  for (int i = 0 ; i < pNbrSamples; i++)
  {
    short v = (short) pData[i];
    char l = v & 0xFF;
    char h = (v & 0xFF00) >> 8; 

    // Destination buffer (Qt) recieves sound with 48000 Hz, so triple the sample...
    for (int j = 0; j < 3;j++)
    {
      fAudioRingBuffer->push(l);
      fAudioRingBuffer->push(h);
    }
  }
  //printf("write after len:%d top:%d bottom:%d size:%d", pNbrSamples , fAudioRingBuffer->top, fAudioRingBuffer->bottom, fAudioRingBuffer->size());
  //if (fAudioRingBuffer->isBufferFull())
  //{
  //  printf("!!!!!!!! buffer full\n");
  //}
  //std::cout << std::endl;  
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
  {
    LOCKER(s_mutexAudioBuffer);
    //printf("read buffer before:len:%d top:%d bottom:%d   ",  len, fAudioRingBuffer->top, fAudioRingBuffer->bottom);
    if (fAudioRingBuffer->size() < len) len = fAudioRingBuffer->size();
    if (len % 2 == 1) len--;

    for (int i = 0; i < len ; i++)
    {
       data[i] = fAudioRingBuffer->pop();
    }
    int bufSize = fAudioRingBuffer->size();
    //printf("read buffer after:%d len:%d top:%d bottom:%d", bufSize, len, fAudioRingBuffer->top, fAudioRingBuffer->bottom);
    //std::cout << std::endl;  
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

