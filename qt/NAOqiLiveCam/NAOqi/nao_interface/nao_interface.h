/**
 * @author Takuji Kawata
 * Updated 2015/03/27
 * Updated 2015/05/07
 */

#ifndef NAO_INTERFACE_H
#define NAO_INTERFACE_H

#include <string>

const int SAMPLERATE_IN = 16000;      	// Input 16000 Hz
const int SAMPLERATE_OUT = 48000;      	// Output 48000 Hz
const int CHANNELBYTES = 2;        		// 16 bit signed short
const int NBOFOUTPUTCHANNELS_IN = 1;    // Mono
const int NBOFOUTPUTCHANNELS_OUT = 1;   // Mono
const int BUFFERSAMPLESIZEMSEC = 1000;  // Sample size with msec. 
const int CAMERA_FPS = 10;

class NAOqiToPCAudioInterface
{
public:
    virtual void writeData(const short *data, int samples) = 0;
};

class NaoInterface
{
	NaoInterface();

public:
	static NaoInterface* instance();

	~NaoInterface();
	void setNaoIp(const std::string ipAddress);
	void disconnect();
	bool isConnected() const;

    void setAudioInterface(NAOqiToPCAudioInterface *audioOutput) {m_audioOutput = audioOutput; }
	NAOqiToPCAudioInterface* getAudioInterface() { return m_audioOutput; } 

	unsigned char*	updateCameraView();

private:
	NAOqiToPCAudioInterface *m_audioOutput;

};

#endif // NAO_INTERFACE_H
