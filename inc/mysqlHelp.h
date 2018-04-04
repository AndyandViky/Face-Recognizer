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
        const char *i_query = "select model_data data_count people_id id from face_data where is_active=1";
        if(mysql_query(&mysql, i_query) == 0) {
            result = mysql_store_result(&mysql);
            if(result) {
                int count = 0;
                while((row = mysql_fetch_row(result)) != NULL) {
                    models[count].userId = atoi(row[2]);
                    models[count].id = atoi(row[3]);
                    models[count].dataSize = atoi(row[1]);
                    char dedata[800];
                    base64_decode(row[0], (unsigned char*)dedata);
                    models[count].faceData = (MByte*)malloc(models[count].dataSize);
                    for(int k=0;k<strlen((char *)dedata); k++) {
                        models[count].faceData[k] = dedata[k];
                    }  
                    count++;
                }
                *len = count;
            }
        }
        // 获取之后更新
        const char *u_query = "update config set isUpdate=0";
        mysql_query(&mysql, u_query);
    }
    return models;
}

/**
 * 增加用户人脸模型 
 */

int addFaceModel(const int id, const char* model, const int size, const char* path, const int isActived) {
    char i_query[1000];
    sprintf(i_query, "insert into face_data(people_id, model_data, data_count, model_image, is_active) values(%d, '%s', %d, '%s', %d)", id, model, size, path, isActived);
    printf("%s\n", i_query);
    if(mysql_query(&mysql, i_query) == 0) {
        return 1;
    } else {
        printf("插入失败%s\n", mysql_error(sock));
        return -1;
    }
}

/**
 * 根据imageId获取 path
 */
char* getImagePath(const int id) {
    MYSQL_RES *result; //保存结果
    MYSQL_ROW row; // 代表的是结果集中的一行
    char i_query[200];
    sprintf(i_query, "select path from attachment where id=%d", id);
    if(mysql_query(&mysql, i_query) == 0) {
        result = mysql_store_result(&mysql);
        if(result) {
            if((row = mysql_fetch_row(result)) != NULL) {
                return row[0];
            }
            mysql_free_result(result);
        }
    }
    char *fail = (char*)"fail";
    return fail;
}

/**
 * 记录门禁
 */
int insertRecord(const int id, const int count, const char* path) {
    char i_query[1000];
    sprintf(i_query, "insert into camera_record(people_id, face_img, count) values(%d, '%s', '%d')", id, path, count);
    printf("%s\n", i_query);
    if(mysql_query(&mysql, i_query) == 0) {
        return 1;
    } else {
        printf("插入失败%s\n", mysql_error(sock));
        return -1;
    }
}

/**
 * 更新模型相似度
 */
int updateSemblance(const int id, const MFloat result) {
    char i_query[1000];
    sprintf(i_query, "update face_data set semblance=%f where id=%d", result, id);
    printf("%s\n", i_query);
    if(mysql_query(&mysql, i_query) == 0) {
        return 1;
    } else {
        printf("插入失败%s\n", mysql_error(sock));
        return -1;
    }
}

/**
 * 相似度极高, 替换模型
 */
int updateFaceModel(const int id, const char* model, const char* path, const MFloat similScore) {
    MYSQL_RES *result; // 保存结果
    MYSQL_ROW row; // 代表的是结果集中的一行
    char i_query[200], u_query[800];
    int ids[3];
    MFloat semblances[3];
    int count = 0;
    sprintf(i_query, "select id semblance from face_data where is_active=1 and people_id=%d", id);
    if(mysql_query(&mysql, i_query) == 0) {
        result = mysql_store_result(&mysql);
        if(result) {
            while((row = mysql_fetch_row(result)) != NULL) {
                ids[count] = atoi(row[0]);
                semblances[count] = atof(row[1]);
                count++;
            }
            mysql_free_result(result);
        }
    }
    // 查询完毕, 开始处理
    if (count == 0) return -1;
    MFloat min = 0.00f;
    int minId = 0;
    for (int i=0; i<count; i++) {
        // 找到最小的那个相似度
        if (min <= semblances[i]) {
            min = semblances[i];
            minId = ids[i];
        }
    }
    // 找到最小的那个值, 判断最小的那个值是否小于当前获得的那个值
    if (min >= similScore || minId == 0) return -1;
    
    // 替换数据
    sprintf(u_query, "update face_data set model_data='%s' semblance=%f model_image='%s' where id=%d", model, similScore, path, minId);
    if(mysql_query(&mysql, u_query) == 0) {
        return 1;
    }
    return -1;
}