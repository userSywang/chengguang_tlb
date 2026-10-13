#ifndef __APP_RTC_H__
#define __APP_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif


#define PRJ_RTC_TYPE_USE_EXT_IC 1  // 板子上有 RTC_IC 
#define PRJ_RTC_TYPE_COMPATIBLE 2  // 带和不带 RTC_IC 的PCB 都要支持 

#define PRJ_RTC_TYPE   PRJ_RTC_TYPE_COMPATIBLE   // 0 PCB里没有 RTC_IC; 1 板子上有RTC_IC;  2 带和不带 RTC_IC 的PCB 都要支持 


#define SYS_TIME_SEC_MIN  1714100000L  // 系统时间戳的最小允许值 


/*
初始化rtc
*/
void app_rtc_init();


/*
	设置时间日期 
*/
void app_rtc_set_date();


/*
	设置闹钟
参数：
	uint32_t times 时间戳10位
*/
void app_rtc_set_alarm(long times);


/*
	获取时间
返回值
	uint32_t 时间戳10位
*/

long app_rtc_read_time();


/*
	清除闹钟状态 ,应用获取到闹钟状态后， 需要手动清除状态 ，否则一直会触发闹钟状态
	
*/
void app_rtc_clear_status();

/*
	获取闹钟状态
*/
int rtc_read_alm_status();

#ifdef __cplusplus
}
#endif

#endif //__APP_RTC_H__
