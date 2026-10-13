#ifndef __FACTORY_UART_DRIVER_H__
#define __FACTORY_UART_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif
/*
	SN串口写数据
参数
	- buf ： 串口发送数据
	- size： 发送数据长度
返回值
	- 大于0 发送数据长度
	- 小于0 读数据失败
*/
int uart_data_write( const char *buf, size_t size);
/*
	SN串口读数据
参数
	- buf ：读到的串口数据
	- length： 读串口数据长度
	- ticks_to_wait： 读数据超时时间
返回值
	- 大于0 读到的数据长度
	- 小于0 读数据失败
*/
int uart_data_read(void *buf, uint32_t length, TickType_t ticks_to_wait );
/*
	SN串口初始化
	
参数
	- baud_rate：波特率(调试串口波特率不能太快 ， 一般9600 )
	- enable_debug : 
	enable_debug = true : esp32s3 uart 只使用rx  , tx 可以正常输出log
	enable_debug = false : esp32s3 uart 能接收发送 , 没有输出log
*/
void factory_uart_driver_init(int baud_rate , bool enable_debug);

/*
	SN串口去初始化
*/
void  factory_uart_driver_deinit();


#ifdef __cplusplus
}
#endif

#endif//__FACTORY_UART_DRIVER_H__
