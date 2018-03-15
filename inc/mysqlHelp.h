#include <mysql/mysql.h>
#include "utils.h"

/**
 * mysql_query(&mysql, i_query) 
 * mysql_store_result() //保存查询的结果 
 */

MYSQL mysql, *sock;
/**
 * 连接mysql数据库
 */
int connectMysql() {
    mysql_init(&mysql);
    sock = mysql_real_connect(&mysql, server, user, password, database, port, NULL, 0);
    if(sock == NULL) {
        printf("连接mysql失败：%s\n", mysql_error(sock));
        return -1;
    } else {
        printf("连接mysql成功！\n");
        return 0;
    }
}

/**
 * 断开mysql连接
 */
void freeMysql() {
    mysql_close(sock);
    printf("关闭mysql连接成功！\n");
}


//--------------------------------------------//
//-------------------以上为mysql基础函数--------//
//--------------------------------------------//


//*************************//
//    人脸识别的相关操作      //
//*************************//

int insertFaceModel(AFR_FSDK_FACEMODEL faceModel) {
    char i_query[1000], data[800], size[10];
    strcpy (i_query, "insert into face_data(people_id, model_data, data_count) values(1, ");
    
    char base64[800];
    base64_encode(faceModel.pbFeature, base64);
    sprintf (size, "%d", faceModel.lFeatureSize);
    strcat (i_query, "\'");
    strcat (i_query, base64);
    strcat (i_query, "\'");
    strcat (i_query, ", ");
    strcat (i_query, size);
    strcat (i_query, ")");
    printf("%s\n", i_query);
    if(mysql_query(&mysql, i_query) == 0) {
        printf("插入成功\n");
    }
    else printf("插入失败%s\n", mysql_error(sock));
}

/**
 * 检测人脸库是否更新
 */
int checkUpdate() {
    // 1. 查询所有的摄像头状态
    MYSQL_RES *result; //保存结果
    MYSQL_ROW row; // 代表的是结果集中的一行
    const char *i_query = "select isUpdate from config";
    if(mysql_query(&mysql, i_query) == 0) {
        result = mysql_store_result(&mysql);
        if(result) {
            if((row = mysql_fetch_row(result)) != NULL) {
                if(strcmp(row[0], "1") == 1) {
                    return 1;
                }
            }
            mysql_free_result(result);
        }
    }
    return 0;
}



// 全局缓存
static FaceModelResult *models = (FaceModelResult*)malloc(faceModelLength);
static int cnt = 0;
/**
 * 清空模型数组
 */
void freeModels() {
    for(int i = 0; i < 237; i++) {
        if (models[i].faceData) {
            free(models[i].faceData);
        }
    }
    memset(models, 0, faceModelLength);
}

/**
 * 获取人脸数据信息
 */
FaceModelResult* getFaceModel(int *len) {
    //int cnt = checkUpdate();
    cnt++;
    if(cnt == 1) {

        MYSQL_RES *result; //保存结果
        MYSQL_ROW row; // 代表的是结果集中的一行
        const char *i_query = "select model_data data_count from face_data";
        if(mysql_query(&mysql, i_query) == 0) {
            result = mysql_store_result(&mysql);
            if(result) {
                int count = 0;
                while((row = mysql_fetch_row(result)) != NULL) {
                    models[count].dataSize = 22020;
                    char dedata[800];
                    base64_decode(row[0], (unsigned char*)dedata);
                    models[count].faceData = (MByte*)malloc(models[count].dataSize);
                    for(int k=0;k<strlen((char *)dedata); k++) {
                        models[count].faceData[k] = dedata[k];
                    }  
                    count++;
                }
                // printf("%d\n", models[0].lFeatureSize);
                // printf("%s\n", models[0].pbFeature);
                *len = count;
                printf("%d\n", count);
            }
        }
    }
    return models;
}