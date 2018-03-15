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
 * 打开摄像头
 */
int openCamera(int camera) {
    int cameraIndex = boxIndex;
    boxIndex++;
    cameraBox[cameraIndex].cameraNum = camera;
    cameraBox[cameraIndex].isOpen = true;
    CvCapture* cam = cvCaptureFromCAM(camera);
    if(!cam) {
        printf("Could not initialize opening of Camera %d..\n", camera);
        return -1;
    }
    cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE); // create a window called "Camera"
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
    cameraBox[cameraIndex].isOpen = false;
    cvReleaseCapture(&cam);
    return 0;
}

/**
 * 线程识别
 * 一个线程最多识别两台摄像头
 */
int checkFace(int index) {
    if(index <= 1 || index > boxIndex+2 || index%2!=0) {
        // 参数错误
        return -1;
    }
    int currentIndex = index-2;
    const int begin = index-2;
    const int end = index-1;
    // 此处标记： 查询当前处理摄像头状态
    while(checkOperateCamera(index) == 1) {
        ASVLOFFSCREEN inputImg = { 0 };
        if (currentIndex == begin) {
            currentIndex = end;
            if (cameraBox[currentIndex-1].cam0Frame) {
                inputImg.i32Width = cameraBox[currentIndex-1].cam0Frame->width;      
                inputImg.i32Height = cameraBox[currentIndex-1].cam0Frame->height;
                inputImg.ppu8Plane[0] = (MUInt8*)&cameraBox[currentIndex-1].cam0Frame->imageData[0];
            }
            else continue;
        }
        else if(currentIndex == end) {
            currentIndex = begin;
            if (cameraBox[currentIndex+1].cam0Frame) {
                inputImg.i32Width = cameraBox[currentIndex+1].cam0Frame->width;      
                inputImg.i32Height = cameraBox[currentIndex+1].cam0Frame->height;
                inputImg.ppu8Plane[0] = (MUInt8*)&cameraBox[currentIndex+1].cam0Frame->imageData[0];
            }
            else continue;
        }
        else {
            // 发生未知错误, 修复当前指向
            currentIndex = begin;
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
        int *param, len, k, i;
        param = &len;
        FaceModelResult *models = getFaceModel(param);
        for (i = 0; i < faceResult->nFace; i++) {
            // 进入循环前需要将当前的人脸信息读取出来
            AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[i], faceResult->lfaceOrient[i]);
            if(faceModels.lFeatureSize != 22020) {
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
    //freeModels();
    freeAllEngine();
    return 0;
}

int main(int argc, char* argv[]) {

    connectMysql();
    initCameraBox();
    //initAllEngine();
    int *param, len;
    param = &len;
    
    for(int i=0; i<5; i++) {
        FaceModelResult *models = getFaceModel(param);
    }

    //int result = openCamera(0);
    //freeAllEngine();
    freeModels();
    freeMysql();
    return 0;
}




