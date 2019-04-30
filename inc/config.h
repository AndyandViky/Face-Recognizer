#define APPID "GbiFvjqzJQPLGvjFAXjAZ2D4vZLnqA8exPYyZHQCTHaq"
#define SDKKEYD "3NC7uc9Qi4QJCQdmVuQ7tphJB33yo4Mpt2Nu4urbs4td"
#define SDKKEYR "3NC7uc9Qi4QJCQdmVuQ7tphnpe6gGgbFwjtwyMsPdkJE"
#define SDKKEYT "3NC7uc9Qi4QJCQdmVuQ7tphB1dnopkK6ycZ8nbPgs3kF"
#define SDKKEYA "3NC7uc9Qi4QJCQdmVuQ7tpi39SczpmJoiR94yufvmEwN"
#define SDKKEYG "3NC7uc9Qi4QJCQdmVuQ7tpiAJqt95HmzywNLEx9zQwUE"

#define WORKBUF_SIZE  (50*1024*1024)
#define REGISTER_PATH "/home/andy/workspace/arcface/recognitionImage/register.yuv"
#define CHECK_PATH "/home/andy/workspace/arcface/recognitionImage/check.yuv"
#define UPDATE_PATH "/home/andy/workspace/arcface/recognitionImage/secondImage.yuv"

/**
 * 图像比较参数
*/
#define COMPARE_IMAGE_FORMAT  ASVL_PAF_I420
#define INPUT_IMAGE_WIDTH   (640)
#define INPUT_IMAGE_HEIGHT  (480)

char ROOTPATH[100] =  "/home/andy/workspace/arcface/";
// 全局定义箱子长度
const int boxLength = 100;
// 全局摄像头箱子
static CameraBox cameraBox[boxLength] = {0};
// 全局摄像头当前index
static int boxIndex = 0;
// 全局锁定摄像头箱子, 防止多线程同时操作
static MBool boxLock = false;

// 全局定义人脸模型长度
const int faceModelLength = 30000;

// 全局定义线程池
static pthread_t threadId[boxLength];
static int threadIndex = 0;

// 全局定义mysql配置信息
const char *server = "192.168.118.1";
const char *user = "root";
const char *password = "888888";//“******”为你设置的密码
const unsigned int port = 3306;
const char *database = "face";

// -----------------串口变量配置-------   ------//
static int fd = -1; // 全局串口设置
static int nSpeed = 9600; // 全局波特率
static int nBits = 8; // 全局数据位选择
static char nEvent = 'N'; // 全局奇偶校验
static int nStop = 1; // 全局停止位
static char *fdPath = (char*)"/dev/ttyUSB0"; // 串口路径配置
// -------------------------------------------//
