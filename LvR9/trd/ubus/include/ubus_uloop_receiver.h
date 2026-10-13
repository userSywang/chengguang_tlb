#ifndef __UBUS_ULOOP_RECEIVER_H
#define __UBUS_ULOOP_RECEIVER_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief 初始化ubus消息中心
 * 初始化时需传入invoke对象
 * @param object 使用对象
 * @return int
 * - 0 成功  会阻塞
 * - -1 失败
 */
int ubus_uloop_receiver_invoke_init(struct ubus_object *object);

/**
 * @brief 初始化ubus消息中心
 * 初始化时需传入event对象
 * @param event_handler 使用对象
 * @return int
 * - 0 成功  会阻塞
 * - -1 失败
 */
int ubus_uloop_receiver_event_init(struct ubus_event_handler *event_handler, const char *object_name);

/**
 * @brief 退出uloop_run阻塞
 *
 * @param msg void
 * @return void
 */
void ubus_uloop_receiver_exit(void);

#ifdef __cplusplus
}
#endif
#endif
