#include "header.h"

#include "arcsoft_fsdk_age_estimation.h"

MHandle hEngineA;
MByte *pWorkMemA = NULL;

/**
 * 初始化年龄引擎
 */
MHandle initialAgeEngine() {
    pWorkMemA = (MByte *)malloc(WORKBUF_SIZE);
    if(pWorkMemA == NULL){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        return NULL;
    }
    int ret = ASAE_FSDK_InitAgeEngine(APPID, SDKKEYA, pWorkMemA, WORKBUF_SIZE, &hEngineA);
    if (ret != 0) {
        fprintf(stderr, "fail to AFR_FSDK_InitialEngine(): 0x%x\r\n", ret);
        free(pWorkMemA);
        return NULL;
    }
    return hEngineA;
}

/**
 * 检测年龄 
 */
int checkAge(ASVLOFFSCREEN inputImg, MRECT faceRect, AFD_FSDK_OrientCode orient) {

    ASAE_FSDK_AGEFACEINPUT ageFaceInput = { 0 };
    ageFaceInput.lFaceNumber = 1;
    MRECT faceRectArray[1];
    MInt32 faceOrientArray[1];
    ageFaceInput.pFaceRectArray = faceRectArray;
    ageFaceInput.pFaceOrientArray = faceOrientArray;

    ageFaceInput.pFaceRectArray[0].left = faceRect.left;
    ageFaceInput.pFaceRectArray[0].top = faceRect.top;
    ageFaceInput.pFaceRectArray[0].right = faceRect.right;
    ageFaceInput.pFaceRectArray[0].bottom = faceRect.bottom;
    ageFaceInput.pFaceOrientArray[0] = orient;

    ASAE_FSDK_AGERESULT  ageResult = { 0 };
    int ret = ASAE_FSDK_AgeEstimation_StaticImage(hEngineA, &inputImg, &ageFaceInput, &ageResult);
    if (ret != 0) {
        fprintf(stderr, "fail to ASAE_FSDK_AgeEstimation_StaticImage(): 0x%x\r\n", ret);
        return -1;
    }
    printf("年龄%d\n", ageResult.pAgeResultArray[0]);
    return ageResult.pAgeResultArray[0];
}

/**
 * 释放引擎内存
 */
void freeAgeEngine() {
    if(hEngineA != NULL) ASAE_FSDK_UninitAgeEngine(hEngineA);
    if(pWorkMemA != NULL) free(pWorkMemA);
}