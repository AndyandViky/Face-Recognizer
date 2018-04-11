#define APPID "GbiFvjqzJQPLGvjFAXjAZ2CwmA5bEnSbvPN1oHfdP6vY"
#define SDKKEYD "F3ZDFYTgw71JX34j7PNHDSbt2nmkLUsevguxtztikbws"
#define SDKKEYR "F3ZDFYTgw71JX34j7PNHDScNgPpSUVk7hh2GyHMaFTHW"
#define SDKKEYT "F3ZDFYTgw71JX34j7PNHDSbksPWXKXBZE4XdzA3256ky"
#define SDKKEYA "F3ZDFYTgw71JX34j7PNHDScd1CLmdrGDrehMWggXnusB"
#define SDKKEYG "F3ZDFYTgw71JX34j7PNHDSckAbbsp6ukhYAWwf9fDSgb"

#define WORKBUF_SIZE  (50*1024*1024)

#define REGISTER_PATH "/home/yanglin/yl/c++/arcsoft-arcface/arcface/recognitionImage/register.yuv"
#define CHECK_PATH "/home/yanglin/yl/c++/arcsoft-arcface/arcface/recognitionImage/check.yuv"
#define UPDATE_PATH "/home/yanglin/yl/c++/arcsoft-arcface/arcface/recognitionImage/secondImage.yuv"

/**
 * 图像比较参数
*/
#define COMPARE_IMAGE_FORMAT  ASVL_PAF_I420
#define INPUT_IMAGE_WIDTH   (640)
#define INPUT_IMAGE_HEIGHT  (480)

char ROOTPATH[100] =  "/home/yanglin/yl/c++/arcsoft-arcface/face-api/";
// 全局定义箱子长度
const int boxLength = 100;
// 全局摄像头箱子
static CameraBox cameraBox[boxLength] = {0};
// 全局摄像头当前index
static int boxIndex = 0;
// 全局锁定摄像头箱子, 防止多线程同时操作
static MBool boxLock = false;

// 全局定义人脸模型长度
const int faceModelLength = 1000;

// 全局定义线程池
static pthread_t threadId[boxLength];
static int threadIndex = 0;

// 全局定义mysql配置信息
const char *server = "localhost";
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
