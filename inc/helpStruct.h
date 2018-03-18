
#include "amcomdef.h"
#include <opencv2/opencv.hpp>
/**
 * 获取人脸数据信息数据模型
 */
typedef struct{
	MInt32  userId;
    MByte   *faceData;
    MInt32  dataSize;
}FaceModelResult;


/**
 * 管理摄像头开关模型
 */
typedef struct{
    MBool isOpen;
    MInt32 cameraNum;
    IplImage *cam0Frame;
    MBool isOperated;
}CameraBox;