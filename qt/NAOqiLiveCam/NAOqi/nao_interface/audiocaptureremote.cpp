/**
 * @author Takuji Kawata
 * Updated 2015/03/27
 * Updated 2015/05/07
 */

#include <iostream>
#include "audiocaptureremote.h"

#include <qi/os.hpp>
#include <alvalue/alvalue.h>
#include <alcommon/alproxy.h>
#include <alcommon/albroker.h>
#include "nao_interface.h"

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
{
  setModuleDescription("Captures audio");

  functionName("isCapturing", "AudioCaptureRemote", "Says if the capture was started.");
  setReturn("capturing", "Whether capture was started.");
  BIND_METHOD(AudioCaptureRemote::isCapturing);

  functionName("startCapture", "AudioCaptureRemote", "Starts audio capture.");
  addParam("audio", "Whether to enable audio capture.");
  BIND_METHOD(AudioCaptureRemote::startCapture);

  functionName("stopCapture", "AVCaptureRemote", "Stops audiovisual capture.");
  BIND_METHOD(AudioCaptureRemote::stopCapture);

  std::cout << xGetTime() << ":construct AudioCaptureRemote" << std::endl;

}

AudioCaptureRemote::~AudioCaptureRemote() {
  stopCapture();
}

void AudioCaptureRemote::init()
{
  std::cout << xGetTime() << ":AudioCaptureRemote::init()" << std::endl;
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
  std::cout << xGetTime() << ":AudioCaptureRemote::stopCapture()" << std::endl;
  if(fCapturingAudio)
    xStopAudio();

}

void AudioCaptureRemote::xStartAudio()
{
  try
  {
    audioDevice->callVoid("setClientPreferences",
                          getName(),                //Name of this module
                          SAMPLERATE_IN,            //16000 Hz requested  For NAOqi 2.0.6, it only can provide buffer with 16000Hz when you listen single channel. 
                          (int)AL::FRONTCHANNEL,    //Front Channels requested
                          0                         //Deinterleaving not requested
                          );

    this->startDetection();
    fCapturingAudio = true;
  }
  catch(const std::exception &error)
  {
    std::cerr << "Cannot subscribe audio: " << error.what() << std::endl;
    fCapturingAudio = false;
  }
}

void AudioCaptureRemote::xStopAudio()
{
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
  if (NaoInterface::instance() && NaoInterface::instance()->getAudioInterface()) 
  {
    NaoInterface::instance()->getAudioInterface()->writeData(pData, pNbrSamples);
  }
}


long long AudioCaptureRemote::xGetTime() {
  qi::os::timeval lTimeStruct;
  qi::os::gettimeofday(&lTimeStruct);
  long long lTime = (long long)lTimeStruct.tv_sec * 1000000; // Seconds
  lTime += (long long)lTimeStruct.tv_usec;                   // Microseconds
  return lTime;
}

