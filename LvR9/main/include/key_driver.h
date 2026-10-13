#ifndef __KEY_DRIVER_H__
#define __KEY_DRIVER_H__

enum{
    KEY_EVENT_DOWN = 0 ,
    KEY_EVENT_UP,
    KEY_EVENT_LONG,
};


enum{
	
	KEY_UP		 = 1 ,	
	KEY_DOWN 	   = 2 ,
	KEY_RIGHT	   = 3 ,	
	KEY_LEFT 	   = 4 ,
	KEY_RETURN		   = 5 ,	
	KEY_OK	   = 6 ,	
	KEY_AB		   = 7 ,	
	KEY_VOLUME_DEC  = 8 , 
	KEY_VOLUME_INC  = 9 , 
	KEY_RECORD      = 10 , 
	KEY_HOME 	   = 11 ,	
	KEY_END		   = 12 ,	
	KEY_BACKSPACE   = 13 ,
	KEY_DEL		   = 14 ,
	KEY_NEXT 	   = 15 ,
	KEY_PREV 	   = 16 ,	
	KEY_POWER 	   = 17 ,	
	
	KEY_MAX  ,
};

enum {
	UNKNOWN_EVENT		  = 0,
	ALARM_TRIGGER_EVENT   = 1,
	SDCARD_INSERT_EVENT   = 2,
	SDCARD_REMOVE_EVENT   = 3,
};

typedef struct key_scan_para_t {
	int scan_time;				//按键扫描频率, 单位ms
	int filter_cnt;			//当filter_cnt累加到base_cnt值时, 消抖有效
	int long_time;  			//按键判定长按数量
	int long_repeat_time;  	//定长按重复时间
}key_scan_para;


typedef struct key_event_para_t {
    int event; // 按键事件
    int value; // 按键值
	void * usre_data;
}key_event_para;


typedef  void  (*key_event_callback)(key_event_para key );

typedef  void  (*alarm_event_callback)(int status , void *user_data );

/*
功能： 按键初始化  
参数：	key_scan_para
*/
void key_driver_init(key_scan_para *para);

/*
功能： 去按键初始化  
*/
void key_driver_deinit(void);

/*
	设置按键回调事件
	参数 key_event_callback 
*/
void set_key_event_callback(key_event_callback callback);


int  set_alarm_sdcard_event_callback(alarm_event_callback callback , void *user_data);


int get_cur_pressed_key_value(void);


/*********************************************

	获取rtc GPIO 闹钟状态 ， 需要先初始化按键
	返回:
		1：存在闹钟
		0：无闹钟
		< 0 : 未初始化

**********************************************/
int get_rtc_alarm_status(void);

/*********************************************

	获取 SD 插入状态
	返回:
		1：SD已插入 
		0：SD已拔出
		< 0 : 未初始化
	
**********************************************/
int get_sdcard_insert_status(void);




#endif  // __KEY_DRIVER_H__
