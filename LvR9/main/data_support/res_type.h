#ifndef RES_TYPE_H
#define RES_TYPE_H

typedef enum {
	RES_NONE,
	RES_CG_BUILDIN,		// 内置
	RES_CG_ONLINE,		// 在线
	RES_YD_ONLINE,		// 有道在线资源
	RES_YD_BUILDIN,		// 有道内置资源
	RES_BAIDU_CLOUD,	// 百度网盘
} e_res_store;

#endif