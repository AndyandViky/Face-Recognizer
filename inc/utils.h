#include "header.h"
#include <fcntl.h>
#include <linux/kd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "arcsoft_fsdk_face_detection.h"


/**
 * 读取图片文件 
 */
int fu_ReadFile(const char* path, uint8_t **raw_data, size_t* pSize) {
    int res = 0;
    FILE *fp = 0;
    uint8_t *data_file = 0;
    size_t size = 0;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        res = -1;
        goto exit;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data_file = (uint8_t *)malloc(sizeof(uint8_t)* size);
    if (data_file == NULL) {
        res = -2;
        goto exit;
    }

    if (size != fread(data_file, sizeof(uint8_t), size, fp)) {
        res = -3;
        goto exit;
    }

    *raw_data = data_file;
    data_file = NULL;
exit:
    if (fp != NULL) {
        fclose(fp);
    }

    if (data_file != NULL) {
        free(data_file);
    }

    if (NULL != pSize) {
        *pSize = size;
    }
    
    return res;
}

/**
 * 处理图片
 */
ASVLOFFSCREEN handleImage(ASVLOFFSCREEN inputImg) {
    if (ASVL_PAF_I420 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width/2;
        inputImg.pi32Pitch[2] = inputImg.i32Width/2;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
        inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height/2;
    } else if (ASVL_PAF_NV12 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
    } else if (ASVL_PAF_NV21 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
    } else if (ASVL_PAF_YUYV == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width*2;
    } else if (ASVL_PAF_I422H == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width / 2;
        inputImg.pi32Pitch[2] = inputImg.i32Width / 2;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
        inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height;
    } else if (ASVL_PAF_LPI422H == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
    } else if (ASVL_PAF_RGB24_B8G8R8 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width*3;
    } else {
        fprintf(stderr, "unsupported Image format: 0x%x\r\n",inputImg.u32PixelArrayFormat);
    }
    return inputImg;
}

/**
 * 读取图片
 */
ASVLOFFSCREEN getImage(const char* path, const int width, const int height) {
    ASVLOFFSCREEN inputImg = { 0 };
    inputImg.u32PixelArrayFormat = COMPARE_IMAGE_FORMAT;
    inputImg.i32Width = width;
    inputImg.i32Height = height;
    inputImg.ppu8Plane[0] = NULL;
    fu_ReadFile(path, (uint8_t**)&inputImg.ppu8Plane[0], NULL);
    if (inputImg.ppu8Plane[0]) {
        inputImg = handleImage(inputImg);
    }
    return inputImg;
}

/**
 * 获取当前时间秒数
 */
long int getTime() {
    time_t t;
    struct tm *pt;
    char *pc;
    time(&t);
    return t;
}

/**
 * 替换字符串中的字符
 * @params s1 传入的源串
 * @params p1 目标字符
 * @params p2 替换字符
 */
char* replace_char(char *source, char s1, char s2)
{
    int i = 0;
    char *q = NULL;

    q = source;
    for(i=0; i<strlen(q); i++)
    {
        if(q[i] == s1)
        {
            q[i] = s2;
        }
    }
    printf("长度%d\n", i);
    return q;
}

/**
 * 改变图片格式
 */
void toYuv(const char* path, const char* target) {
    char cmdline[300];
    sprintf(cmdline, "bash /home/andy/workspace/arcface/bash/changeImage.sh %s %s", path, target);
    
    system(cmdline);
}




// 全局常量定义
const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char padding_char = '=';

/*编码代码
* const unsigned char * sourcedata， 源数组
* char * base64 ，码字保存
*/
int base64_encode(const unsigned char * sourcedata, char * base64)
{
    int i=0, j=0;
    unsigned char trans_index=0;    // 索引是8位，但是高两位都为0
    const int datalength = 22020;
    for (; i < datalength; i += 3){
        // 每三个一组，进行编码
        // 要编码的数字的第一个
        trans_index = ((sourcedata[i] >> 2) & 0x3f);
        base64[j++] = base64char[(int)trans_index];
        // 第二个
        trans_index = ((sourcedata[i] << 4) & 0x30);
        if (i + 1 < datalength){
            trans_index |= ((sourcedata[i + 1] >> 4) & 0x0f);
            base64[j++] = base64char[(int)trans_index];
        }else{
            base64[j++] = base64char[(int)trans_index];

            base64[j++] = padding_char;

            base64[j++] = padding_char;

            break;   // 超出总长度，可以直接break
        }
        // 第三个
        trans_index = ((sourcedata[i + 1] << 2) & 0x3c);
        if (i + 2 < datalength){ // 有的话需要编码2个
            trans_index |= ((sourcedata[i + 2] >> 6) & 0x03);
            base64[j++] = base64char[(int)trans_index];

            trans_index = sourcedata[i + 2] & 0x3f;
            base64[j++] = base64char[(int)trans_index];
        }
        else{
            base64[j++] = base64char[(int)trans_index];

            base64[j++] = padding_char;

            break;
        }
    }

    base64[j] = '\0'; 

    return 0;
}

/** 在字符串中查询特定字符位置索引
* const char *str ，字符串
* char c，要查找的字符
*/
inline int num_strchr(const char *str, char c) // 
{
    const char *pindex = strchr(str, c);
    if (NULL == pindex){
        return -1;
    }
    return pindex - str;
}
/* 解码
* const char * base64 码字
* unsigned char * dedata， 解码恢复的数据
*/
int base64_decode(const char * base64, unsigned char * dedata)
{
    int i = 0, j=0;
    int trans[4] = {0,0,0,0};
    for (;base64[i]!='\0';i+=4){
        // 每四个一组，译码成三个字符
        trans[0] = num_strchr(base64char, base64[i]);
        trans[1] = num_strchr(base64char, base64[i+1]);
        // 1/3
        dedata[j++] = ((trans[0] << 2) & 0xfc) | ((trans[1]>>4) & 0x03);

        if (base64[i+2] == '='){
            continue;
        }
        else{
            trans[2] = num_strchr(base64char, base64[i + 2]);
        }
        // 2/3
        dedata[j++] = ((trans[1] << 4) & 0xf0) | ((trans[2] >> 2) & 0x0f);

        if (base64[i + 3] == '='){
            continue;
        }
        else{
            trans[3] = num_strchr(base64char, base64[i + 3]);
        }

        // 3/3
        dedata[j++] = ((trans[2] << 6) & 0xc0) | (trans[3] & 0x3f);
    }

    dedata[j] = '\0';

    return 0;
}

/**
 * 初始化摄像头箱子
 */
void initCameraBox() {
    for(int i = 0; i < boxLength; i++) {
        cameraBox[i].isOpen = false;
        cameraBox[i].cameraNum = -1;
        cameraBox[i].cam0Frame = {0};
        cameraBox[i].isOperated = false;
    }
}

/**
 * 查询单台摄像头的状态
 */
int checkCameraType(int cameraNum) {
    for(int i = 0; i < boxIndex; i++){
        if(cameraBox[i].cameraNum == cameraNum) {
            // 找到当前需要查询的摄像头
            if(cameraBox[i].isOpen == true) return 1;
        }
    }
    return 0;
}

/**
 * 查询所处理摄像头状态
 */
int checkOperateCamera(int *num) {
    for(int i=0; i<sizeof(num)/sizeof(num[0]); i++) {
        if(num[i] == -1) continue;
        if(cameraBox[num[i]].isOpen == true) return 1;
    }
    return 0;
}

/**
 * 查询当前开启的摄像头中有哪台的数据没有被处理
 */
int checkIsNotOperate() {
    for(int i = 0; i < boxIndex; i++) {
        if(cameraBox[i].isOpen && !cameraBox[i].isOperated) {
            return i;
        }
    }
    return -1;
}

/**
 * 查询所有摄像头的状态
 */
int checkAllCamera() {
    for(int i = 0; i < boxIndex; i++) {
        if(cameraBox[i].isOpen) {
            return 1;
        }
    }
    return 0;
}

/**
 * 查询摄像头所属位置
 */
int getCameraIndex(int cameraNum) {
    for(int i = 0; i < boxIndex; i++) {
        if(cameraBox[i].cameraNum == cameraNum) {
            return i;
        }
    }
    return -1;
}

/**
 * 获得当前开启的摄像头数量
 */
int getOpenCameraCount() {
    int count = 0;
    for(int i = 0; i < boxIndex; i++) {
        if (cameraBox[i].isOpen) count++;
    }
    return count;
}

/**
 * 打开声音
 */
int openVoice() {
    printf("\a");
}
