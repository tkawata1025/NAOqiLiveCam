/**
 * @author Takuji Kawata
 * Updated 2015/03/27
 */

#define DLLAPI __declspec(dllexport)

#include "nao_interface.h"

#include <signal.h>
#include <alcommon/albroker.h>
#include <alcommon/almodule.h>
#include <alcommon/albrokermanager.h>
#include "audiocaptureremote.h"

#ifdef AVCAPTURE_IS_REMOTE
# define ALCALL
#else
# ifdef _WIN32
#  define ALCALL __declspec(dllexport)
# else
#  define ALCALL
# endif
#endif

#include <iostream>
#include <alproxies/alvideodeviceproxy.h>
#include <alvision/alimage.h>
#include <alvision/alvisiondefinitions.h>
#include <alerror/alerror.h>
#include <opencv2/core/core.hpp>
#include <opencv/cv.h>
#include <qi/os.hpp>
#include <pthread.h>

static AL::ALVideoDeviceProxy	*s_cameraProxy = NULL;
static std::string				s_cameraClientName;
static cv::Mat					s_cameraImage;
static cv::Mat					s_cameraImageClone;

static std::string s_robotIpAddress = "";

static pthread_mutex_t	s_mutex;
static pthread_mutex_t	s_mutexCamUpdate;

const static float	QVGA_WIDTH	= 320;
const static float	QVGA_HEIGHT= 240;

ALCALL int _createModule(boost::shared_ptr<AL::ALBroker> pBroker)
{
  // init broker with the main broker instance
  // from the parent executable
  AL::ALBrokerManager::setInstance(pBroker->fBrokerManager.lock());
  AL::ALBrokerManager::getInstance()->addBroker(pBroker);
  AL::ALModule::createModule<AudioCaptureRemote>(pBroker, "AudioCaptureRemote");
  return 0;
}

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

//static
NaoInterface* NaoInterface::instance()
{
	static NaoInterface sInstance;

	return &sInstance;
}

NaoInterface::NaoInterface()
{
	pthread_mutex_init(&s_mutex, NULL);
	pthread_mutex_init(&s_mutexCamUpdate, NULL);
}

NaoInterface::~NaoInterface()
{
	disconnect();
}

void NaoInterface::setNaoIp(const std::string ipAddress)
{
	static boost::shared_ptr<AL::ALBroker> broker;

	if (s_robotIpAddress != ipAddress)
	{

		int parentBrokerPort = 9559;

		// Need this to for SOAP serialization of floats to work
		setlocale(LC_NUMERIC, "C");

		// A broker needs a name, an IP and a port:
		const std::string brokerName = "mybroker";
		// FIXME: would be a good idea to look for a free port first
		int brokerPort = 54000;
		const std::string brokerIp   = "0.0.0.0";  // listen to anything

		try
		{
			broker = AL::ALBroker::createBroker(
				brokerName,
				brokerIp,
				brokerPort,
				ipAddress,
				parentBrokerPort,
				0    // you can pass various options for the broker creation,
					// but default is fine
			);
		}
		catch(const AL::ALError& /* e */)
		{
			std::cerr << "Faild to connect broker to: "
			<< ipAddress
			<< ":"
			<< parentBrokerPort
			<< std::endl;
			AL::ALBrokerManager::getInstance()->killAllBroker();
			AL::ALBrokerManager::kill();
			return;
		}

		// Deal with ALBrokerManager singleton:
		AL::ALBrokerManager::setInstance(broker->fBrokerManager.lock());
		AL::ALBrokerManager::getInstance()->addBroker(broker);


		// Now it's time to load your module with
		AL::ALModule::createModule<AudioCaptureRemote>(broker, "AudioCaptureRemote");

		if (s_cameraProxy)
		{
			disconnect();
		}
		
        try
        {
			s_cameraProxy = new AL::ALVideoDeviceProxy();

			LOCKER(s_mutexCamUpdate);
			s_cameraClientName = s_cameraProxy->subscribe("cam1", AL::kQVGA, AL::kRGBColorSpace, 100);
			s_cameraImage  = cv::Mat(cv::Size((int)QVGA_WIDTH, (int)QVGA_HEIGHT), CV_8UC3);

		}
		catch( AL::ALError e)
		{
			std::string msg = e.what();
			throw msg;
		}
		s_robotIpAddress = ipAddress;

    }
}

void NaoInterface::disconnect()
{
	LOCKER(s_mutex);

	if (s_cameraProxy)
	{
		try
		{
			if (s_cameraClientName.length() > 0)
			{
				s_cameraProxy->unsubscribe(s_cameraClientName);
				s_cameraClientName = "";
			}
		}
		catch( AL::ALError e)
		{
		}
		
		delete s_cameraProxy;
	}
	s_cameraProxy = NULL;

	AL::ALBrokerManager::getInstance()->killAllBroker();
	AL::ALBrokerManager::kill();

	s_robotIpAddress = "0.0.0.0";

}

bool NaoInterface::isConnected() const
{
	LOCKER(s_mutex);

    return s_cameraProxy != NULL;
}

unsigned char* NaoInterface::updateCameraView()
{
	LOCKER(s_mutexCamUpdate);

	if (s_cameraProxy == NULL)
		return NULL;

	/** Retrieve an image from the camera.
	 * The image is returned in the form of a container object, with the
	 * following fields:
	 * 0 = width
	 * 1 = height
	 * 2 = number of layers
	 * 3 = colors space index (see alvisiondefinitions.h)
	 * 4 = time stamp (seconds)
	 * 5 = time stamp (micro seconds)
	 * 6 = image buffer (size of width * height * number of layers)
	 */
	AL::ALValue img = s_cameraProxy->getImageRemote(s_cameraClientName);
	/** Access the image buffer (6th field) and assign it to the opencv image
		* container. */
	s_cameraImage.data = (uchar*) img[6].GetBinary();
	s_cameraImageClone = s_cameraImage.clone();
	s_cameraProxy->releaseImage(s_cameraClientName);

	return (unsigned char*) s_cameraImageClone.data;
}

//static
int NaoInterface::readAudioBuffer(char *data, int len)
{
	return AudioCaptureRemote::readData(data, len);
}


