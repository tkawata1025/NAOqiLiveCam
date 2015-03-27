/**
 * @author Takuji Kawata
 * Updated 2015/03/27
 */

#ifndef AUDIOCAPTURE_AVCAPTUREREMOTE_H
#define AUDIOCAPTURE_AVCAPTUREREMOTE_H

#include <string>
#include <alcommon/almodule.h>
#include <alaudio/alsoundextractor.h>
#include <pthread.h>

class SimpleRingBuffer;

namespace AL
{
  class ALBroker;
}

/**
 * Remote module for synchronized audio and video capture.
 */
class AudioCaptureRemote : public AL::ALSoundExtractor
{
  public:
    /**
     * Default Constructor for modules.
     * @param broker the broker to which the module should register.
     * @param name the boadcasted name of the module.
     */
    AudioCaptureRemote(boost::shared_ptr<AL::ALBroker> broker, const std::string& name);

    /// Destructor.
    virtual ~AudioCaptureRemote();

    void init();

    /// Are we currently capturing ?
    /// @return whether we started capturing
    bool isCapturing();

    /**
     * Start capture.
     */
    void startCapture();

    /// Stop capture.
    void stopCapture();

    /**
     * Set buffer time.
     */
    void setBufferTime(const int& buffertime);

    // Audio processing =======================================================
protected:
    /// Are we currently capturing audio ?
    bool fCapturingAudio;

    /// Start audio capture.
    void xStartAudio();

    /// Stop audio capture.
    void xStopAudio();

    /// The maximum number of samples provided by ALAudioDevice.
    static int kMaxNofSamples;

    /// Our output buffer, allocated once.
    //AL_SOUND_FORMAT* fAudioBuffer;
    //unsigned short* fAudioBuffer;

public:

    static int readData(char *data, int len); 

    /**
     * Remote callback from ALAudioDevice providing sound buffers.
     * @param pNbOfInputChannels number of audio channels provided here.
     * @param pNbrSamples length of the audio buffer in samples.
     * @param pDataInterleaved raw interleaved audio buffer.
     * @param pTimeStamp The time stamp of the audio buffer
     */
    void process(const int &pNbOfInputChannels,
                 const int &pNbrSamples,
                 const AL_SOUND_FORMAT *pDataInterleaved,
                 const AL::ALValue &pTimeStamp);

    int readDataInternal(char *data, int len);

    /// Get the time in Us
    /// @return time in Us
    static long long xGetTime();

private:

    SimpleRingBuffer *fAudioRingBuffer; 

};

#endif  // AUDIOCAPTURE_AVCAPTUREREMOTE_H

