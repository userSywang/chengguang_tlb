
#ifndef MLOG_H
#define MLOG_H

/*
* mlog.h - define mlog manager of project
*
* Copyright (C) 2023
* Author: ly
*/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>

#define NONE "\e[0m"
#define RED "\e[1;31m"
#define GREEN "\e[0;32m"
#define YELLOW "\e[1;33m"
#define BLUE "\e[0;34m"

#ifdef __cplusplus
extern "C"
{
#endif

#if 1

	// 打印当前的时间信息  
	char* print_sys_time();

	/* 获取当前文件名 */
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)


#define LOGV(format, ...) \
   do {                                                                                                               \
        printf(GREEN "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__, "\n",       \
               ##__VA_ARGS__, "\n");                                                                                          \
        syslog(LOG_INFO, GREEN "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__,   \
               "\n", ##__VA_ARGS__, "\n");                                                                                   \
            fflush(stdout);                                                                                            \
    } while(0)                                                                                                      \

#define LOGD(format, ...)                                                                                              \
    do {                                                                                                               \
        printf(GREEN "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__, "\n",       \
               ##__VA_ARGS__, "\n");                                                                                          \
        syslog(LOG_INFO, GREEN "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__,   \
               "\n", ##__VA_ARGS__, "\n");                                                                                   \
            fflush(stdout);                                                                                            \
    } while(0)                                                                                                         \


#define LOGI(format, ...)                                                                                          \
     do {                                                                                                               \
        printf(YELLOW "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__, "\n",       \
               ##__VA_ARGS__, "\n");                                                                                          \
        syslog(LOG_INFO, GREEN "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__,   \
               "\n", ##__VA_ARGS__, "\n");                                                                                   \
            fflush(stdout);                                                                                            \
    } while(0)                                                                                                       \


#define LOGW(format, ...) \
   do {                                                                                                               \
        printf(BLUE "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__, "\n",       \
               ##__VA_ARGS__, "\n");                                                                                          \
        syslog(LOG_INFO, GREEN "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__,   \
               "\n", ##__VA_ARGS__, "\n");                                                                                   \
            fflush(stdout);                                                                                            \
    } while(0)                                                                                                       \

#define LOGE(format, ...) \
   do {                                                                                                               \
        printf("RED %s %s::%s[%d]:   " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__,               \
               ##__VA_ARGS__, "\n");        \
        syslog(LOG_INFO, RED "%s %s::%s[%d]:%s " NONE format, print_sys_time(), __FILENAME__, __func__, __LINE__,   \
               "\n", ##__VA_ARGS__, "\n");                                                                                   \
        fflush(stdout);                                                                                                \
    } while(0)                                                                                                      \

#else
	/* 宏：如果当前日志级别大于或等于指定级别，则输出日志 */
#define MLOG_LEVEL_NONE 0  // 无日志输出
#define MLOG_LEVEL_ERROR 1 // 错误日志
#define MLOG_LEVEL_WARN 2  // 警告日志
#define MLOG_LEVEL_INFO 3  // 信息日志
#define MLOG_LEVEL_DEBUG 4 // 调试日志

#define NONE "\e[0m"
#define RED "\e[1;31m"
#define GREEN "\e[0;32m"
#define YELLOW "\e[1;33m"
#define BLUE "\e[0;34m"

/* 获取当前文件名 */
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define MLOG(level, level_str, fmt, ...)                                                                               \
    do {                                                                                                               \
        if(level <= MLOG_LEVEL) {                                                                                      \
            printf(level_str " [%s:%d %s] " NONE fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__);                \
        }                                                                                                              \
        fflush(stdout);                                                                                                \
    } while(0)

#define MLOG_ERROR(fmt, ...) MLOG(MLOG_LEVEL_ERROR, RED "[ERR]", fmt, ##__VA_ARGS__)
#define MLOG_WARN(fmt, ...) MLOG(MLOG_LEVEL_WARN, YELLOW "[WARN]", fmt, ##__VA_ARGS__)
#define MLOG_INFO(fmt, ...) MLOG(MLOG_LEVEL_INFO, BLUE "[INFO]", fmt, ##__VA_ARGS__)
#define MLOG_DEBUG(fmt, ...) MLOG(MLOG_LEVEL_DEBUG, GREEN "[DBG]", fmt, ##__VA_ARGS__)

#if 1
/* 确定各个模块的启动时间 */
#define CSR_TIME 0xc01
#ifdef __ASSEMBLY__
#define __ASM_STR(x) x
#else
#define __ASM_STR(x) #x
#endif

/* 25000000 / 1000000 */
#define CPU_CLOCK_DIV 25
#define csr_read(csr)                                                                                                  \
    ({                                                                                                                 \
        register unsigned long __v;                                                                                    \
        __asm__ __volatile__("csrr %0, " __ASM_STR(csr) : "=r"(__v) : : "memory");                                     \
        __v / CPU_CLOCK_DIV / 1000;                                                                                    \
    })

#define LOGD(fmt, ...)                                                                                          \
    do {                                                                                                               \
        printf(RED "[%s:%d %s] %lu ms " NONE fmt, __FILENAME__, __LINE__, __func__, csr_read(CSR_TIME),                \
               ##__VA_ARGS__);                                                                                         \
    } while(0)
#else
#define MLOG_TIMING(fmt, ...)
#endif

#endif // 0

#ifdef __cplusplus
}
#endif

#endif  //  MLOG_H
