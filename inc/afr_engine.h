#include "header.h"

#include "arcsoft_fsdk_face_recognition.h"
MHandle hEngineR;
MByte *pWorkMemR = NULL;


/**
 * 初始化人脸识别引擎
 */
MHandle initialFaceREngine() {
    pWorkMemR = (MByte *)malloc(WORKBUF_SIZE);
    if(pWorkMemR == NULL){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        return NULL;
    }
    int ret = AFR_FSDK_InitialEngine(APPID, SDKKEYR, pWorkMemR, WORKBUF_SIZE, &hEngineR);
    if (ret != 0) {
        fprintf(stderr, "fail to AFR_FSDK_InitialEngine(): 0x%x\r\n", ret);
        free(pWorkMemR);
        return NULL;
    }
    return hEngineR;
}

/**
 * 获取人脸特征值
 */
AFR_FSDK_FACEMODEL getFeature(ASVLOFFSCREEN inputImg, MRECT face, AFD_FSDK_OrientCode orient) {
    AFR_FSDK_FACEMODEL faceModels = { 0 };
    {
        int ret;
        AFR_FSDK_FACEINPUT faceResult;
        faceResult.lOrient = orient;
        faceResult.rcFace.left = face.left;
        faceResult.rcFace.top = face.top;
        faceResult.rcFace.right = face.right;
        faceResult.rcFace.bottom = face.bottom;
        AFR_FSDK_FACEMODEL LocalFaceModels = { 0 };
        ret = AFR_FSDK_ExtractFRFeature(hEngineR, &inputImg, &faceResult, &LocalFaceModels);
        if (ret != 0) {
            fprintf(stderr, "fail to AFR_FSDK_ExtractFRFeature in Image A\r\n");
        }
        else {
            faceModels.lFeatureSize = LocalFaceModels.lFeatureSize;
            faceModels.pbFeature = (MByte*)malloc(faceModels.lFeatureSize);
            memcpy(faceModels.pbFeature, LocalFaceModels.pbFeature, faceModels.lFeatureSize);
        }
    }
    return faceModels;
}

/**
 * 比较两张图片的相似度
 */
MFloat compareFace(AFR_FSDK_FACEMODEL faceModels1, FaceModelResult data) {

    AFR_FSDK_FACEMODEL faceModels2 = { 0 };
    {
        faceModels2.lFeatureSize = (MInt32)data.dataSize;
        faceModels2.pbFeature = (MByte*)malloc(faceModels2.lFeatureSize);
        memcpy(faceModels2.pbFeature, data.faceData, data.dataSize);
    }
    MFloat  fSimilScore = 0.0f;
    int ret = AFR_FSDK_FacePairMatching(hEngineR, &faceModels1, &faceModels2, &fSimilScore);
    printf("fSimilScore ==  %f\r\n", fSimilScore);
    free(faceModels2.pbFeature);
    return fSimilScore;
}

/**
 * 释放引擎内存
 */
void freeFaceREngine() {
    if(hEngineR != NULL) AFR_FSDK_UninitialEngine(hEngineR);
    if(pWorkMemR != NULL) free(pWorkMemR);
}