// Note: 本源文件只用于小机模式工程 (PC仿真基本用不到该文件) 

#if 0

#include "app_main.h"
#include <string.h>
#include "lvgl_main.h"

#include "./ex_widget/com_vol.h"
#include "./utils/global_parm.h"


esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause(void);
extern void alarm_sdcard_event_reg(void * queue);
extern void zb_sys_bg_task(void *para);

//----------------------------------------------

static const char *TAG = "app_main";


//----------------------------------------------

static lv_disp_t  *lcdDisp = NULL;

static TaskHandle_t a_task_Handle = NULL;


//----------------------------------------------

#if 0
static QueueHandle_t lvgl_msg_queue_create(){
	return xQueueCreate(10, sizeof(lvgl_queue_command_t));
}
#endif

const char * app_get_hardware_version(void) {
   // return "BY_CGTLB_V10_20240315";  // for old pcb 
   //return "BY_CGTLB_V11_20240424";  // for 0424 pcb 
   return "BY_CGTLB_V11_20240617";  //
}


#endif

