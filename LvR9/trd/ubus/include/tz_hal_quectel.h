#ifndef __TZ_HAL_QUECTEL_H__
#define __TZ_HAL_QUECTEL_H__

#ifdef __cplusplus
extern "C"
{
#endif

	static const char* const method_call = "call";
	static const char* const method_hangup = "hangup";
	static const char* const method_vts = "vts";
	static const char* const method_answer = "answer";
	static const char* const method_get_simstate = "get_simstate";
	static const char* const method_get_opername = "get_opername";
	static const char* const method_get_signalstrength = "get_signalstrength";
	static const char* const method_get_networktype = "get_networktype";
	static const char* const method_get_deviceid = "get_deviceid";
	static const char* const method_enter_simpin = "enter_simpin";
	static const char* const method_enter_simpuk = "enter_simpuk";
	static const char* const method_set_volume = "set_volume";
	static const char* const method_get_volume = "get_volume";
	static const char* const method_set_airplane = "set_airplane";
	static const char* const method_get_airplane = "get_airplane";
	static const char* const method_set_dataflow = "set_dataflow";
	static const char* const method_get_dataflow = "get_dataflow";
	static const char* const method_get_netuse = "get_netuse";

	static const char* const event_in_call = "in_call";
	static const char* const event_out_call = "out_call";
	static const char* const event_in_hangup = "in_hangup";
	static const char* const event_in_answer = "in_answer";
	static const char* const event_sim_info = "sim_info";
	static const char* const event_sim_inserted = "sim_inserted";

	static const char* const ubus_method = "method";
	static const char* const ubus_value = "value";
	static const char* const ubus_event = "event";
	static const char* const ubus_phone_number = "phone_number";
	static const char* const ubus_vts_number = "vts_number";
	static const char* const ubus_state = "state";
	static const char* const ubus_sim_state = "sim_state";
	static const char* const ubus_oper_name = "oper_name";
	static const char* const ubus_signal_strength = "signal_strength";
	static const char* const ubus_signal_rssi = "signal_rssi";
	static const char* const ubus_signal_sinr = "signal_sinr";
	static const char* const ubus_network_type = "network_type";
	static const char* const ubus_device_id = "device_id";
	static const char* const ubus_sim_pin = "sim_pin";
	static const char* const ubus_sim_puk = "sim_puk";
	static const char* const ubus_volume_level = "volume_level";
	static const char* const ubus_airplane_state = "airplane_state";
	static const char* const ubus_inserted_state = "inserted_state";
	static const char* const ubus_dataflow_state = "dataflow_state";
	static const char* const ubus_net_use = "net_use";
	static const char* const ubus_call_state = "call_state";

	/**
	 * @brief usb模式
	 */
	typedef enum
	{
		USB_HOST = 0,        	// host模式  4G功能
		USB_PERIPHERAL,							// device模式  adb/mtp功能
	} UsbMode;

	/*
		主动上报的信息
		1、sim卡相关信息(运营商、网络类型、信号强度)
		2、来电信息(来电电话号码)
		3、对方挂断信息
		4、sim卡插拔状态(插入、拔出或未知)
	*/

	/**
	 * @brief 音量等级0~5
	 */

	 /**
	  * @brief 当前使用的网络
	  */
	typedef enum
	{
		NETWORK_UNUSE = 0,					// 没有使用网络
		NETWORK_USE_4G,							// 使用2/3/4G网络
		NETWORK_USE_WIFI,        		// 使用WIFI网络
	} NetUse;


	/**
	 * @brief 数据流量开关状态
	 */
	typedef enum
	{
		DATAFLOW_OFF = 0,        	// 关
		DATAFLOW_ON,							// 开
	} DataFlowState;

	/**
	 * @brief SIM 卡的插拔状态
	 */
	typedef enum
	{
		SIM_OUT = 0,        			// 拔出
		SIM_IN,										// 插入
		SIM_UNKNOWN,							// 未知状态。该状态出现在SIM卡初始化之前
	} InsertedState;

	/**
	 * @brief 飞行模式状态
	 */
	typedef enum
	{
		AIRPLANE_OFF = 0,        	// 关
		AIRPLANE_ON,							// 开
	} AirplaneState;

	/**
	 * @brief 网络类型
	 */
	typedef enum
	{
		NETWORK_TYPE_UNKNOWN = 1,			// 未知网络
		NETWORK_TYPE_2G,							// 2G网络
		NETWORK_TYPE_3G,        			// 3G网络
		NETWORK_TYPE_4G,							// 4G网络
	} NetworkType;

	/**
	 * @brief 信号强度
	 */
	typedef enum
	{
		LEVEL_0 = 0,					// 0格
		LEVEL_1,							// 1格
		LEVEL_2,        			// 2格
		LEVEL_3,							// 3格
		LEVEL_4,							// 4格
	} SignalLevel;

	/**
	 * @brief 营运商
	 */
	typedef enum
	{
		CHINA_UNKNOWN = 0,					// 未知运营商
		CHINA_MOBILE,								// 中国移动
		CHINA_UNICOM,        				// 中国联通
		CHINA_CT,										// 中国电信
	} OperName;

	/**
	 * @brief SIM卡状态
	 */
	typedef enum
	{
		SIM_STATE_ABSENT = 0,					// 未插卡
		SIM_STATE_UNKNOWN,				    // 未知状态
		SIM_STATE_READY,              // 准备就绪
		SIM_STATE_PIN_REQUIRED,		    // 需要SIM卡PIN码解锁
		SIM_STATE_PUK_REQUIRED,		    // 需要SIM卡PUK码解锁
	};

	enum
	{
		QUECTEL_CALL_CONNECT = 1,										        // 呼叫连接
		QUECTEL_CALL_END,												        // 呼叫结束
	};

	typedef struct tz_quectel_handler
	{
		int (*call)(const char* phone_number);
		int (*answer)(void);
		int (*hangup)(void);
		int (*get_sim_state)(int* state);
		int (*get_oper_name)(int* oper);
		int (*get_signal_strength)(int* signalLevel, int* rssi, int* sinr);
		int (*get_network_type)(int* netType);
		int (*get_device_id)(char* imei);
		int (*set_volume)(const int level);
		int (*get_volume)(int* vol);
		int (*get_net_use)(int* netuse);
	}quectel_handler_t;

	/**
	 * @brief 获取usb模式切换
	 *
	 * @return 成功返回1，失败返回0
	 */
	int tz_get_otg_mode(UsbMode* mode);

	/**
	 * @brief 设置usb模式切换
	 *
	 * @return 成功返回1，失败返回0
	 */
	int tz_set_otg_mode(const UsbMode mode);


	/**
	 * @brief 申请quectel控制句柄，调用quectel控制接口前需要先调用此接口
	 *
	 * @return quectel_handler_t* 成功返回句柄地址，失败返回NULL
	 */
	quectel_handler_t* tz_hal_quectel_handler_new(void);

	/**
	 * @brief 释放quectel控制句柄
	 *
	 * @param module quectel控制句柄变量地址
	 */
	void tz_hal_quectel_handler_destory(quectel_handler_t* module);

#ifdef __cplusplus
}
#endif

#endif // __TZ_HAL_QUECTEL_H__
