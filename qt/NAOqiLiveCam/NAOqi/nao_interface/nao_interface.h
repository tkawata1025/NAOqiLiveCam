/**
 * @author Takuji Kawata
 * Updated 2015/03/27
 */

#ifndef NAO_INTERFACE_H
#define NAO_INTERFACE_H

#include <string>
#include <vector>


class NaoInterface
{
	NaoInterface();

public:
	static NaoInterface* instance();

	~NaoInterface();
	void setNaoIp(const std::string ipAddress);
	void disconnect();
	bool isConnected() const;

	unsigned char*	updateCameraView();
	static int 		readAudioBuffer(char *data, int len);

};

#endif // NAO_INTERFACE_H
