#include "afd_engine.h"
#include "afr_engine.h"
#include "age_engine.h"
#include "gender_engine.h"
#include "job.h" 

#define CAMERA_IMAGE_FORMAT  ASVL_PAF_RGB24_B8G8R8

/**
 * 初始化所有的引擎 
 */
void initAllEngine() {
    initialFaceDEngine();
    initialFaceREngine();
    initialAgeEngine();
    initialGenderEngine();
}

/**
 * 释放所有引擎内存
 */
void freeAllEngine() {
    freeFaceREngine();
    freeFaceDEngine();
    freeAgeEngine();
    freeGenderEngine();
}

/**
 * 关闭一台摄像头
 */
int freeOneCamera(int cameraNum) {
    for(int i = 0; i < boxIndex; i++){
        if(cameraBox[i].cameraNum == cameraNum) {
            // 找到当前需要关闭的摄像头
            cameraBox[i].isOpen = false;
            cameraBox[i].isOperated = false;
            cameraBox[i].cameraNum = -1;
            return 1;
        }
    }
    return 0;
}

/**
 * 获取所有摄像头信息
 */
char* getAllCameraInfo() {
    char *result = (char*)malloc(1000);
    sprintf(result, "[");
    for(int i = 0; i < boxIndex; i++) {
        if(cameraBox[i].isOpen) {
            if (i==boxIndex-1) {
                sprintf(result, "{name: %d, isOpen: %ld, isOperate: %ld}", cameraBox[i].cameraNum, cameraBox[i].isOpen, cameraBox[i].isOperated);
            } else {
                sprintf(result, "{name: %d, isOpen: %ld, isOperate: %ld},", cameraBox[i].cameraNum, cameraBox[i].isOpen, cameraBox[i].isOperated);
            }
        }
    }
}

/**
 * 关闭所有摄像头
 */
 void freeAllCamera(){
    for (int i = 0; i < boxIndex; i++) {
        cameraBox[i].isOpen = false;
        cameraBox[i].cameraNum = -1;
        cameraBox[i].isOperated = false;
    }
    freeAllEngine(); // 释放引擎
 }


MBool swith = true;
/**
 * 线程识别未封装版本
 * 一个线程最多识别两台摄像头
 */
void *_checkFace(void *arg) {
    pthread_t checkID = threadId[threadIndex];
    threadIndex++;
    int len = 1, currentIndex = boxIndex-1, cBox[2], i, isOver;
    cBox[0] = boxIndex-1;
    cBox[1] = -1;
    cameraBox[cBox[0]].isOperated = true;
    // 此处标记： 查询当前处理摄像头状态
    while(checkOperateCamera(cBox) == 1) {
        //printf("开始检测\n");
        if (len == 1) {
            // 此处说明当前线程还能再处理一台摄像头的数据
            int num = checkIsNotOperate();
            if(num != -1 && !boxLock) {
                boxLock = true; // 锁定
                if (cBox[0] == -1) cBox[0] = num;
                else cBox[1] = num;
                cameraBox[num].isOperated = true;
                len = 2;
                boxLock = false; // 解锁
            }
        }
        ASVLOFFSCREEN inputImg = { 0 };
        if (currentIndex == cBox[0]) {
            currentIndex = cBox[1]; 
            if (cBox[0] != -1 && cameraBox[cBox[0]].isOpen) {
                if (cameraBox[cBox[0]].cam0Frame) {
                   // printf("成功进入检测1\n");
                    inputImg.i32Width = cameraBox[cBox[0]].cam0Frame->width;      
                    inputImg.i32Height = cameraBox[cBox[0]].cam0Frame->height;
                    inputImg.ppu8Plane[0] = (MUInt8*)&cameraBox[cBox[0]].cam0Frame->imageData[0];
                }
                else continue;
            } else {
                cBox[0] = -1;
                for(i=0, isOver=0; i<sizeof(cBox); i++) {
                    if (cBox[i] != -1) isOver++;
                }
                len = isOver;
                continue;
            };
        }
        else if(currentIndex == cBox[1]) {
            currentIndex = cBox[0];
            if (cBox[1] != -1 && cameraBox[cBox[1]].isOpen) {
                if (cameraBox[cBox[1]].cam0Frame) {
                   // printf("成功进入检测2\n");
                    inputImg.i32Width = cameraBox[cBox[1]].cam0Frame->width;      
                    inputImg.i32Height = cameraBox[cBox[1]].cam0Frame->height;
                    inputImg.ppu8Plane[0] = (MUInt8*)&cameraBox[cBox[1]].cam0Frame->imageData[0];
                }
                else continue;
            } else {
                cBox[1] = -1;
                for(i=0, isOver=0; i<sizeof(cBox); i++) {
                    if (cBox[i] != -1) isOver++;
                }
                len = isOver;
                continue;
            };
        }
        else {
            printf("发生错误修复\n");
            // 发生未知错误, 修复当前指向
            currentIndex = cBox[0];
            continue;
        }
        inputImg.u32PixelArrayFormat = CAMERA_IMAGE_FORMAT;
        
        inputImg = handleImage(inputImg);   
        
        LPAFD_FSDK_FACERES faceResult = getStillImage(inputImg);
        if(faceResult == NULL || faceResult->nFace <= 0){
           // printf("未检测到人脸\n");
            continue;
        }

        /**
         * 对获得数组做一些操作
         */
        MFloat total = 0.00f;
        int len, k, i;
        FaceModelResult *models = getFaceModel(&len);
        for (i = 0; i < faceResult->nFace; i++) {
            // 进入循环前需要将当前的人脸信息读取出来
            int orient = faceResult->lfaceOrient[i];
            if (!orient) orient = 0;
            AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[i], orient);

            if(faceModels.lFeatureSize <= 0) {
                continue;
            }
           // printf("获取到人脸图像\n");
            
            for(k=0; k<len; k++) {
                MFloat result = compareFace(faceModels, models[k]);
                // if(result != -1) {
                //     if(result > 0.80) {
                //         // 一旦发现一个大于0.8 结束循环
                //    /     // 此处更新数据库，记录当前开门者及其类别
                //         printf("%d\n", models[k].userId);
                //         count = 10;       
                //         break;
                //     }
                // }
            }
            if (faceModels.pbFeature) {
                free(faceModels.pbFeature);
            }
        }
    }
    pthread_join(checkID, NULL);
    printf("关闭检测线程成功\n");
    if (checkAllCamera() == 0) {
        freeAllEngine();
    }
}

/**
 * 开启检测
 **/
int checkFace() {
    printf("开启检测线程成功\n");
    int ret = pthread_create(&threadId[threadIndex], NULL, _checkFace, NULL);
    if(ret!=0){
        printf ("Create pthread error!\n"); 
        return -1;
    }
    return ret;
}

/**
 * 打开摄像头未封装版本
 */
void *_openCamera(void *arg) {
    pthread_t cameraID = threadId[threadIndex];
    threadIndex++;
    int cameraIndex = boxIndex;
    int camera = cameraBox[cameraIndex].cameraNum;
    CvCapture* cam = cvCaptureFromCAM(camera);
    if(!cam) {
        printf("Could not initialize opening of Camera %d..\n", camera);
    } else {
        boxIndex++;
        cameraBox[cameraIndex].isOpen = true;
        cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE); // create a window called "Camera"

        if(getOpenCameraCount()%2 != 0) {
            // 此处说明出现基数次的摄像头, 需要新开辟一个线程处理摄像头数据
            checkFace();
        }
        while(checkCameraType(camera) == 1) {
            cameraBox[cameraIndex].cam0Frame = cvQueryFrame(cam);
            if (cameraBox[cameraIndex].cam0Frame) {
                cvShowImage("Camera", cameraBox[cameraIndex].cam0Frame);
            }
            if (cvWaitKey(30) > 0) //wait for 'Esc' key press for 30ms. If 'Esc' key is pressed, break loop
            {
                swith = false;
                // cout << "Esc key is pressed by user" << endl;
                break;
            }
        }
        cameraBox[cameraIndex].cam0Frame = {0};
        cvReleaseCapture(&cam);
    }
    pthread_join(cameraID, NULL);
    printf("关闭摄像头线程成功\n");
}
/**
 * 打开摄像头
 */
int openCamera(int type) {
    cameraBox[boxIndex].cameraNum = type;  
    printf("开启摄像头线程成功\n");
    int ret = pthread_create(&threadId[threadIndex], NULL, _openCamera, NULL);
    if(ret!=0){
        printf ("Create pthread error!\n"); 
        return -1;
    }
    return ret;
}

/**
 * 根据图片获取人脸特征数据
 */
char* getRegisterFeature(const char* path) {
    char* result = (char *)"fail";
    toYuv(path, REGISTER_PATH);
    ASVLOFFSCREEN inputImg = getImage(REGISTER_PATH);
        
    inputImg = handleImage(inputImg);   
    
    LPAFD_FSDK_FACERES faceResult = getStillImage(inputImg);
    
    if(faceResult == NULL || faceResult->nFace <= 0){
        // printf("未检测到人脸\n");
        result = (char *)"fail";
    } else if(faceResult->nFace > 1) {
        result = (char *)"fail";
    } else {
        AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[0], faceResult->lfaceOrient[0]);
        if(strlen((char *)faceModels.pbFeature) > 200) {
            char *base64 = (char *)malloc(800);
            base64_encode(faceModels.pbFeature, base64);
            return base64;
        }
    }
    return result;
}

int main(int argc, char* argv[]) {

    connectMysql();
    initCameraBox();
    initAllEngine();

    int result = openCamera(0);
    while(swith) {}
    //freeModels();
    freeMysql();
    // const char* path = "/home/yanglin/yl/c++/arcsoft-arcface/arcface/recognitionImage/recognition.jpg";
    // char *result = getRegisterFeature(path);
    // printf("%s\n", result);
    freeAllEngine();
    return 0;
}




