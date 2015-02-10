#include "lwip/opt.h"
#include <sys/time.h>
#include <sys/types.h>
#include "lwip/sockets.h"
/*u-boot很多地方用到了延时操作，主要是cpu下面的interrupts.c文件中的udelay和get_timer函数get_timer_masked函数以及表示当前时间的全局变量timestamp和上一次访问定时器的时间lastdec实现的。
 
1 实现延时要用到一个定时器，u-boot采用查询定时器TC的方法得到当前的时间点，所以需要根据定时器原理实现READ_TIMER宏，从代码get_timer_maskd代码风格上判断大部分定时器都应该是TC递减的，但LPC2468的定时器是TC增加的；
2 根据u-boot的最小时间粒度1us算出CFG_HZ参数，保证TC增1小于1us且1us为TC值的整数倍。参考5
3 u-boot默认延时时间不会超过TC复位周期的2倍大小，所以根据lastdec和now的TC值大小判断出TC是否循环过一次，来得到timestamp，并更新lastdec，见get_timer_masked；
4 get_timer的用法是先ts = gettimer(0)得到当前的timestamp，并以此为基点，然后循环判断要求的等待条件是否满足，循环内则比较gettimer(ts)与要求的延时量的大小，如超时则退出程序；
5 udelay的最小粒度是1us,可以根据其最小粒度选择合适的CFG_HZ参数是其满足1us延时增加TC的这个整数倍。同时根据最大值是32bit,以及定时器TC的数据宽度为16位还是32位估算出定时器最大延迟时间。*/

u32_t
sys_now(void)
{
	//ulong start, now, last;
//	start = get_timer(0);

//    struct timeval tv;
//    if (gettimeofday(&tv, NULL))
//        return 0;
//    else
//        return tv.tv_sec * 1000 + tv.tv_usec / 1000;	
}

