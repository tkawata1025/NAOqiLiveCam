cmake_minimum_required(VERSION 2.8)
project(nao_interface)

find_package(qibuild)


qi_create_lib(NaoInterface SHARED 
	"nao_interface.h"
	"nao_interface.cpp"
	"audiocaptureremote.h"
	"audiocaptureremote.cpp"
	)


	
qi_use_lib(NaoInterface ALCOMMON ALVISION ALAUDIO ALPROXIES OPENCV2_VIDEO  OPENCV2_CORE OPENCV2_HIGHGUI OPENCV2_IMGPROC)
#qi_install_header("nao_interface.h")
