#define APPID "GbiFvjqzJQPLGvjFAXjAZ2CwmA5bEnSbvPN1oHfdP6vY"
#define SDKKEYD "F3ZDFYTgw71JX34j7PNHDSbt2nmkLUsevguxtztikbws"
#define SDKKEYR "F3ZDFYTgw71JX34j7PNHDScNgPpSUVk7hh2GyHMaFTHW"
#define SDKKEYT "F3ZDFYTgw71JX34j7PNHDSbksPWXKXBZE4XdzA3256ky"
#define SDKKEYA "F3ZDFYTgw71JX34j7PNHDScd1CLmdrGDrehMWggXnusB"
#define SDKKEYG "F3ZDFYTgw71JX34j7PNHDSckAbbsp6ukhYAWwf9fDSgb"

#define WORKBUF_SIZE  (50*1024*1024)

/**
 * 图像比较参数
*/
#define COMPARE_IMAGE_FORMAT  ASVL_PAF_I420
#define INPUT_IMAGE_WIDTH   (640)
#define INPUT_IMAGE_HEIGHT  (480)

// 全局定义箱子长度
const int boxLength = 100;
// 全局摄像头箱子
static CameraBox cameraBox[boxLength] = {0};
// 全局摄像头当前index
static int boxIndex = 0;

// 全局定义人脸模型长度
const int faceModelLength = 1000;

// 全局定义mysql配置信息
const char *server = "localhost";
const char *user = "root";
const char *password = "888888";//“******”为你设置的密码
const unsigned int port = 3306;
const char *database = "face";
