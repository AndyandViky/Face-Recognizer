#include "afd_engine.h"
#include "afr_engine.h"
#include "age_engine.h"
#include "gender_engine.h"
#include "job.h" 
#include "usb_help.h"

#define CAMERA_IMAGE_FORMAT  ASVL_PAF_RGB24_B8G8R8


/**
 * 释放所有引擎内存
 */
void freeAllEngine() {
    freeFaceREngine();
    freeFaceDEngine();
    freeAgeEngine();
    freeGenderEngine();
    freeMysql();
    closeFd();
}

MBool swith = true;
static int saveCount = 0; // 暂存检测到的人脸数
static MBool timeLimit = false; // 计时器锁

// 暂存人数计时器
void save_count_fn(int sig)    
{  
    saveCount = 0;
    timeLimit = false;
}  
/**
 * 线程识别未封装版本
 * 一个线程最多识别两台摄像头
 */
void *_checkFace(void *arg) {
    // 初始化相关数据
    pthread_t checkID = threadId[threadIndex];
    threadIndex++;
    int len = 1, currentIndex = boxIndex-1, cBox[2], i, isOver;
    cBox[0] = boxIndex-1;
    cBox[1] = -1;
    cameraBox[cBox[0]].isOperated = true;
    MBool isSuccess = false; // 判断此次循环是否开门成功

    // 此处标记： 查询当前处理摄像头状态 开始循环
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
            if (cBox[0] != -1 && cameraBox[cBox[0]].isOpen) {
                if (cameraBox[cBox[0]].cam0Frame) {
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

        // 检测人脸
        inputImg.u32PixelArrayFormat = CAMERA_IMAGE_FORMAT;
        inputImg = handleImage(inputImg);   
        LPAFD_FSDK_FACERES faceResult = getStillImage(inputImg);
        if(faceResult == NULL || faceResult->nFace <= 0){
            // 未检测到人脸, 关门;
            isSuccess = false;
            printf("未检测到人脸\n");
            continue;
        }

        // 如果 暂存的人脸数长时间不变动 并且 时间锁为关闭状态
        if (saveCount != 0 && !timeLimit) {
            timeLimit = true;
            signal(SIGALRM, save_count_fn);  //后面的函数必须是带int参数的
            alarm(10); // 10秒之后刷新暂存人脸数
        }

        /**
         * 获取模型数据, 开始识别
         */
        MFloat total = 0.00f;
        int len, k, i;
        FaceModelResult *models = getFaceModel(&len);
        for (i = 0; i < faceResult->nFace; i++) {
            if (isSuccess) break; // 判断是否成功, 成功结束循环

            // 进入循环前需要将当前的人脸信息读取出来
            int orient = faceResult->lfaceOrient[i];
            if (!orient) orient = 0;
            AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[i], orient);
            printf("人脸数据长度%d------------------------\n", (int)strlen((char *)faceModels.pbFeature));
            // 如果长度不正确, continue;
            if(faceModels.lFeatureSize <= 0) {
                continue;
            }
            // 获取性别以及年龄
            int gender = checkGender(inputImg, faceResult->rcFace[i], orient);
            // int age = checkAge(inputImg, faceResult->rcFace[i], orient);
            MBool isChangeGender = false;
            for(k=0; k<len; k++) {
                if (gender != -1 && gender != models[k].gender) continue; // 首先匹配检测出来的性别
                MFloat result = compareFace(faceModels, models[k]);
                if(result == -1)  continue;
                if(result > 0.65) {
                    // 此处更新数据库，记录当前开门者及其类别
                    struct timeval tv;
                    gettimeofday(&tv,NULL);
                    long int times = tv.tv_sec*1000000 + tv.tv_usec;
                    char path[255];
                    sprintf(path, "/home/yanglin/yl/c++/arcsoft-arcface/arcface/recognitionImage/%dimage%ld.jpg", models[k].userId, times);
                    if (currentIndex == cBox[0]) {
                        // 1
                        cvSaveImage(path , cameraBox[cBox[1]].cam0Frame);
                    } else {
                        // 0
                        cvSaveImage(path , cameraBox[cBox[0]].cam0Frame);
                    }
                    printf("检测成功\n");
                    insertRecord(models[k].userId, faceResult->nFace, (char*)path, result);
                    updateFaceData(models[k].id, result, models[k].passCount+1);
                    // 相似度极高, 替换此模型
                    if (result > 0.7) {
                        // 相似度极高, 替换此模型
                        char *base64 = (char *)malloc(800);
                        base64_encode(faceModels.pbFeature, base64);
                        updateFaceModel(models[k].userId, base64, path, result, faceModels.lFeatureSize, 2);
                        free(base64);
                    }
                    // 发送串口
                    const char *buffer = "10101010";
                    int result = writFd(fd, buffer);
                    if (result == -1) {
                        printf("写入串口失败\n");
                        break;
                    }
                    isSuccess = true;
                    break;
                }
                // 此处判断是否处于当前循环的最后一组
                if (k == len-1 && gender != -1 && !isChangeGender) {
                    isChangeGender = true;
                    k=0;
                    gender == 0 ? 1 : 0;
                }
            }
            if (faceModels.pbFeature) {
                free(faceModels.pbFeature);
            }
        }
        if (!isSuccess && faceResult->nFace > 0 && saveCount != faceResult->nFace) {
            saveCount = faceResult->nFace;
            printf("插入记录\n");
            // 判断是否成功
            struct timeval tv;
            gettimeofday(&tv,NULL);
            long int times = tv.tv_sec*1000000 + tv.tv_usec;
            char path[255];
            if (currentIndex == cBox[0]) {
                // 1
                sprintf(path, "/home/yanglin/yl/c++/arcsoft-arcface/arcface/recognitionImage/%dimage%ld.jpg", cameraBox[cBox[1]].cameraNum, times);
                cvSaveImage(path , cameraBox[cBox[1]].cam0Frame);
            } else {
                // 0
                sprintf(path, "/home/yanglin/yl/c++/arcsoft-arcface/arcface/recognitionImage/%dimage%ld.jpg", cameraBox[cBox[1]].cameraNum, times);
                cvSaveImage(path , cameraBox[cBox[0]].cam0Frame);
            }
            insertRecord(-1, faceResult->nFace, (char*)path, 0);
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
        cvDestroyWindow("Camera");
        cameraBox[cameraIndex].cam0Frame = {0};
        cvReleaseCapture(&cam);
    }
    pthread_join(cameraID, NULL);
    printf("关闭摄像头线程成功\n");
}

extern "C" {  
    /**
     * 初始化所有的引擎 
     */
    void initAllEngine() {
        connectMysql();
        initCameraBox();
        initialFaceDEngine();
        initialFaceREngine();
        initialAgeEngine();
        initialGenderEngine();
        openFd(&fd, fdPath);
        set_serial(fd, nSpeed, nBits, nEvent, nStop);
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
        return result;
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
    int checkFeature(const int imageId) {
        Attachment attachment =  getAttachment(imageId);
        if (strlen(attachment.path) <= 0) return -1;
        toYuv(attachment.path, EHECK_PATH);
        ASVLOFFSCREEN inputImg = getImage(EHECK_PATH, attachment.width, attachment.height);
        
        LPAFD_FSDK_FACERES faceResult = getStillImage(inputImg);
        
        if(faceResult != NULL && faceResult->nFace == 1){
            AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[0], faceResult->lfaceOrient[0]);
            if(strlen((char *)faceModels.pbFeature) > 200) {
                return 1;
            }
        }
        return -1;
    }
    /**
     * 增加模型
     */
    int addModel(const int id, const int imageId, const int isActivte) {
        Attachment attachment =  getAttachment(imageId);
        if (strlen(attachment.path) <= 0) return -1;
        char paths[300];
        strcpy(paths, ROOTPATH);
        strcat(paths, attachment.path);
        toYuv(paths, REGISTER_PATH);
        ASVLOFFSCREEN inputImg = getImage(REGISTER_PATH, attachment.width, attachment.height);
        
        LPAFD_FSDK_FACERES faceResult = getStillImage(inputImg);
        printf("人脸数量%d------------------------\n", faceResult->nFace);
        printf("%d -- %d -- %d -- %d", faceResult->rcFace[0].top, faceResult->rcFace[0].right, faceResult->rcFace[0].left, faceResult->rcFace[0].bottom);
        if(faceResult != NULL && faceResult->nFace == 1){
            AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[0], faceResult->lfaceOrient[0]);
            printf("人脸数据长度%d------------------------\n", (int)strlen((char *)faceModels.pbFeature));
            char *base64 = (char *)malloc(faceModels.lFeatureSize);
            base64_encode(faceModels.pbFeature, base64);
            int result = addFaceModel(id, base64, faceModels.lFeatureSize, attachment.path, isActivte);
            free(base64);
            return result;
        }
        return -1;
    }
    /**
     * 发送串口
     */
    int writeFd() {
        // 发送串口
        if (fd > 0) {
            printf("进入串口\n");
            const char *buffer = "OPENDOOR2\r\n";
            int result = writFd(fd, buffer);
            if (result == -1) {
                printf("写入串口失败\n");
                return -1;
            }
            return 1;
        }
        return -1;
    }
    /**
     * 更新第二张人脸图片
     */
    int updateSecondFaceModel(const int id, const int recordId) {
        MFloat score = 0.00f;
        const char* path = getRecordImage(recordId, &score);
        if (path == "fail") return -1;
        toYuv(path, UPDATE_PATH);
        ASVLOFFSCREEN inputImg = getImage(UPDATE_PATH, INPUT_IMAGE_WIDTH, INPUT_IMAGE_HEIGHT);
        
        LPAFD_FSDK_FACERES faceResult = getStillImage(inputImg);
        if(faceResult != NULL && faceResult->nFace == 1){
            AFR_FSDK_FACEMODEL faceModels = getFeature(inputImg, faceResult->rcFace[0], faceResult->lfaceOrient[0]);
            char *base64 = (char *)malloc(800);
            base64_encode(faceModels.pbFeature, base64);
            int result = updateFaceModel(id, base64, path, score, faceModels.lFeatureSize, 1);
            free(base64);
            return result;
        }
        return -1;
    }
}   

int main(int argc, char* argv[]) {

    initAllEngine();
    writeFd();
    // int result = openCamera(0);
    // //addModel(2, 72, 1);
    // while(swith) {
    //     // if (!swith) {
    //     //     freeOneCamera(0);
    //     // }
    // }
    freeAllEngine();
    return 0;
}
// g++ arcsoft_resource.cpp -fPIC -std=c++11 -L/home/yanglin/yl/c++/arcsoft-arcface/arcface/lib/linux_x64 -I/home/yanglin/yl/c++/arcsoft-arcface/arcface/inc -L/usr/local/lib -lhiredis -lmysqlclient -lpthread -larcsoft_fsdk_face_detection -larcsoft_fsdk_face_recognition -larcsoft_fsdk_age_estimation -larcsoft_fsdk_gender_estimation -lopencv_core -lopencv_highgui -lopencv_imgproc -shared -o libface.so




