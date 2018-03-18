#include "afd_engine.h"
#include "afr_engine.h"
#include "age_engine.h"
#include "gender_engine.h"
#include "job.h" 
#include <pthread.h>

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
            return 1;
        }
    }
    return 0;
}

/**
 * 关闭所有摄像头
 */
 void freeAllCamera(){
    for(int i = 0; i < boxIndex; i++){
        cameraBox[i].isOpen = false;
        cameraBox[i].isOperated = false;
    }
    freeAllEngine(); // 释放引擎
 }

/**
 * 打开摄像头未封装版本
 */
int camera = 0;
void *_openCamera(void *arg) {
    CvCapture* cam = cvCaptureFromCAM(camera);
    if(!cam) {
        printf("Could not initialize opening of Camera %d..\n", camera);
    } else {
        int cameraIndex = boxIndex;
        boxIndex++;
        cameraBox[cameraIndex].cameraNum = camera;
        cameraBox[cameraIndex].isOpen = true;
        cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE); // create a window called "Camera"
        if(boxIndex%2 != 0) {
            // 此处说明出现基数次的摄像头, 需要新开辟一个线程处理摄像头数据

        }
        while(checkCameraType(camera) == 1) {
            cameraBox[cameraIndex].cam0Frame = cvQueryFrame(cam);
            if (cameraBox[cameraIndex].cam0Frame) {
                cvShowImage("Camera", cameraBox[cameraIndex].cam0Frame);
            }
            if (cvWaitKey(30) > 0) //wait for 'Esc' key press for 30ms. If 'Esc' key is pressed, break loop
            {
                // cout << "Esc key is pressed by user" << endl;
                break;
            }
        }
        cameraBox[cameraIndex].cam0Frame = {0};
        cvReleaseCapture(&cam);
    }
}
/**
 * 打开摄像头
 */
int openCamera() {
    pthread_t id;
    printf("开启线程成功\n");
    int ret = pthread_create(&id, NULL, _openCamera, NULL);
    if(ret!=0){
        printf ("Create pthread error!\n"); 
        return -1;
    }
    while(true) {
        if(cameraBox[0].cam0Frame) {
            printf("%d\n", cameraBox[0].cam0Frame->width);
            break;
        }
    };
    pthread_join(id,NULL);
    printf("关闭线程成功\n");
    
    return ret;
}

/**
 * 线程识别
 * 一个线程最多识别两台摄像头
 */
void checkFace() {
    int len = 1, currentIndex = boxIndex-1, cBox[2];
    cBox[0] = boxIndex-1;
    cBox[1] = -1;
    // 此处标记： 查询当前处理摄像头状态
    while(checkOperateCamera(cBox) == 1) {

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
            if (cBox[0] != -1 && cameraBox[cBox[0]].cam0Frame && cameraBox[cBox[0]].isOpen) {
                inputImg.i32Width = cameraBox[cBox[0]].cam0Frame->width;      
                inputImg.i32Height = cameraBox[cBox[0]].cam0Frame->height;
                inputImg.ppu8Plane[0] = (MUInt8*)&cameraBox[cBox[0]].cam0Frame->imageData[0];
            } else {
                cBox[0] = -1;
                len--;
                continue;
            };
        }
        else if(currentIndex == cBox[1]) {
            currentIndex = cBox[0];
            if (cBox[1] != -1 && cameraBox[cBox[1]].cam0Frame && cameraBox[cBox[1]].isOpen) {
                inputImg.i32Width = cameraBox[cBox[1]].cam0Frame->width;      
                inputImg.i32Height = cameraBox[cBox[1]].cam0Frame->height;
                inputImg.ppu8Plane[0] = (MUInt8*)&cameraBox[cBox[1]].cam0Frame->imageData[0];
            } else {
                cBox[1] = -1;
                len--;
                continue;
            };
        }
        else {
            // 发生未知错误, 修复当前指向
            currentIndex = cBox[0];
            continue;
        }
        inputImg.u32PixelArrayFormat = CAMERA_IMAGE_FORMAT;
        
        inputImg = handleImage(inputImg);   
        
        LPAFD_FSDK_FACERES faceResult = getStillImage(inputImg);
        if(faceResult == NULL){
            continue;
        }
        
        //     cvSaveImage("/home/yanglin/yl/c++/arcsoft-arcface/first_demo/ArcFace_opencv/recognitionImage/recognition.jpg", cam0Frame);
        //     system("bash /home/yanglin/yl/c++/arcsoft-arcface/first_demo/ArcFace_opencv/bash/changeImage.sh");

        /**
         * 对获得数组做一些操作
         */
        MFloat total = 0.00f;
        int len, k, i;
        FaceModelResult *models = getFaceModel(&len);
        for (i = 0; i < faceResult->nFace; i++) {
            // 进入循环前需要将当前的人脸信息读取出来
            AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[i], faceResult->lfaceOrient[i]);
            if(faceModels.lFeatureSize <= 0) {
                continue;
            }
            // if(strlen((char *)faceModels.pbFeature) > 500) {
            //     insertFaceModel(faceModels);
            // }
            for(k=0; k<len; k++) {
                MFloat result = compareFace(faceModels, models[k]);
                // if(result != -1) {
                //     if(result > 0.80) {
                //         // 一旦发现一个大于0.8 结束循环
                //         // 此处更新数据库，记录当前开门者及其类别
                //         printf("%d\n", models[k].userId);
                //         count = 10;       
                //         //isSuccess = true;
                //         break;
                //     }
                // }
            }
            free(faceModels.pbFeature);
        }
    }
    if (checkAllCamera() == 0) {
        freeAllEngine();
    }
}

int main(int argc, char* argv[]) {

    connectMysql();
    initCameraBox();
    //initAllEngine();

    int result = openCamera();
    //freeAllEngine();
    //freeModels();
    freeMysql();
    return 0;
}




