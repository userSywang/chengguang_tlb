#ifndef _ZB_SLEEP_MODE_H_
#define _ZB_SLEEP_MODE_H_


#include <stdint.h>

/** 进入睡眠模式 
 * Note: 系统函数, 应用不可调用. 
 */
extern void zb_enter_light_sleep_mode(int timeSecForAutoPwfOff);


extern int16_t ltslp_get_sleep_status();
extern int16_t ltslp_get_wakeup_cause();


#define LIGHTSLP_WAKEUP_CAUSE_AUTO_PWR_OFF  30  // 自动关机唤醒源类型 (注意,该值必须跟 esp_sleep_wakeup_cause_t 的其他值错开)


#endif

