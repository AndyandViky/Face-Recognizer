
#include "header.h"

#include "arcsoft_fsdk_face_detection.h"

#define MAX_FACE_NUM        (60)
MHandle hEngineD;
MByte *pWorkMemD = NULL;
/**
 * 初始化人脸检测引擎 
 */
MHandle initialFaceDEngine() {

    pWorkMemD = (MByte *)malloc(WORKBUF_SIZE);
    if(pWorkMemD == NULL){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        return NULL;
    }
    
    int ret = AFD_FSDK_InitialFaceEngine(APPID, SDKKEYD, pWorkMemD, WORKBUF_SIZE, 
                                         &hEngineD, AFD_FSDK_OPF_0_HIGHER_EXT, 16, MAX_FACE_NUM);
    if (ret != 0) {
        fprintf(stderr, "fail to AFD_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
        free(pWorkMemD);
        return NULL;
    }
    const AFD_FSDK_Version*pVersionInfo = AFD_FSDK_GetVersion(hEngineD);
    printf("%d %d %d %d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
                                 pVersionInfo->lMinor, pVersionInfo->lBuild);
    printf("%s\r\n", pVersionInfo->Version);
    printf("%s\r\n", pVersionInfo->BuildDate);
    printf("%s\r\n", pVersionInfo->CopyRight);
    return hEngineD;
}

/**
 * 获取到人脸在图片中的数据
 */
LPAFD_FSDK_FACERES getStillImage(ASVLOFFSCREEN inputImg) {
    if(hEngineD != NULL) {
        LPAFD_FSDK_FACERES faceResult;
        int ret = AFD_FSDK_StillImageFaceDetection(hEngineD, &inputImg, &faceResult);
        if (ret != 0) {
            fprintf(stderr, "fail to AFD_FSDK_StillImageFaceDetection(): 0x%x\r\n", ret);
            return NULL;
        }
        return faceResult;
    }
    // printf("引擎是空的!\n");
    return NULL;
}

/**
 * 释放引擎内存
 */
void freeFaceDEngine() {
    if(hEngineD != NULL) AFD_FSDK_UninitialFaceEngine(hEngineD);
    if(pWorkMemD != NULL) free(pWorkMemD);
}