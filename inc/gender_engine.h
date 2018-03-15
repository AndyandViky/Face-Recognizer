#include "header.h"

#include "arcsoft_fsdk_gender_estimation.h"


MHandle hEngineG;
MByte *pWorkMemG = NULL;

/**
 * 初始化性别引擎
 */
MHandle initialGenderEngine() {
    pWorkMemG = (MByte *)malloc(WORKBUF_SIZE);
    if(pWorkMemG == NULL){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        return NULL;
    }
    int ret = ASGE_FSDK_InitGenderEngine(APPID, SDKKEYG, pWorkMemG, WORKBUF_SIZE, &hEngineG);
    if (ret != 0) {
        fprintf(stderr, "fail to AFR_FSDK_InitialEngine(): 0x%x\r\n", ret);
        free(pWorkMemG);
        return NULL;
    }
    return hEngineG;
}

/**
 * 检测性别 
 */
int checkGender(ASVLOFFSCREEN inputImg, MRECT faceRect, AFD_FSDK_OrientCode orient) {

    ASGE_FSDK_GENDERFACEINPUT genderFaceInput = { 0 };
    genderFaceInput.lFaceNumber = 1;
    MRECT faceRectArray[1];
    MInt32 faceOrientArray[1];
    genderFaceInput.pFaceRectArray = faceRectArray;
    genderFaceInput.pFaceOrientArray = faceOrientArray;

    genderFaceInput.pFaceRectArray[0].left = faceRect.left;
    genderFaceInput.pFaceRectArray[0].top = faceRect.top;
    genderFaceInput.pFaceRectArray[0].right = faceRect.right;
    genderFaceInput.pFaceRectArray[0].bottom = faceRect.bottom;
    genderFaceInput.pFaceOrientArray[0] = orient;

    ASGE_FSDK_GENDERRESULT  genderResult = { 0 };
    int ret = ASGE_FSDK_GenderEstimation_StaticImage(hEngineG, &inputImg, &genderFaceInput, &genderResult);
    if (ret != 0) {
        fprintf(stderr, "fail to ASGE_FSDK_GenderEstimation_StaticImage(): 0x%x\r\n", ret);
        return -1;
    }
 
    if (genderResult.pGenderResultArray[0] == 0) {
        // 男
        return 0;
    } else if (genderResult.pGenderResultArray[0] == 1) {
        // 女
        return 1;
    } else {
        // 错误
        return -1;
    }
}

/**
 * 释放引擎内存
 */
void freeGenderEngine() {
    if(hEngineG != NULL) ASGE_FSDK_UninitGenderEngine(hEngineG);
    if(pWorkMemG != NULL) free(pWorkMemG);
}