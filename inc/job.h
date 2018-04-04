#include "header.h"
#include <signal.h>
#include <sys/time.h>
#include "mysqlHelp.h"

typedef void (*FUNC)();
void init_sigaction(FUNC a)
{
    struct sigaction act;
        
    act.sa_handler = (__sighandler_t)a; //设置处理信号的函数
    act.sa_flags  = 0;

    sigemptyset(&act.sa_mask);
    sigaction(SIGPROF, &act, NULL);//时间到发送SIGROF信号
}
void init_time(int second)
{
    struct itimerval val;
        
    val.it_value.tv_sec = second; //1秒后启用定时器
    val.it_value.tv_usec = 0;

    val.it_interval = val.it_value; //定时器间隔为1s

    setitimer(ITIMER_PROF, &val, NULL);
} 

//--------------------------------------------//
//-------------------以上为定时器基础函数--------//
//--------------------------------------------//

/**
 * 每间隔5秒扫描数据库获取摄像头状态
 */
void _cameraTypeJob() {
    printf("正在扫描\n");
}
void cameraTypeJob() {
    init_sigaction(_cameraTypeJob);
    init_time(1);
}   
