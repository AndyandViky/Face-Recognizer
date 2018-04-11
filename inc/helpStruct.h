
#include "amcomdef.h"
#include <opencv2/opencv.hpp>
#include <pthread.h>
/**
 * 获取人脸数据信息数据模型
 */
typedef struct{
    MInt32  id;
	MInt32  userId;
    MInt32  gender;
    MInt32  passCount;
    MInt32  age;
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

/**
 * 附件模型
 */
typedef struct{
    char    path[100];
    MInt32  width;
    MInt32  height;
}Attachment;
