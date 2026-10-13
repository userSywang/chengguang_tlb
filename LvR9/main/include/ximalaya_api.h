#ifndef _XIMALAYA_H_
#define _XIMALAYA_H_

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
	喜马拉雅：

*/
typedef enum{

	/*
	*    4.1
	*/
	// 获取卡片信息
	XIMALAYA_GET_CARD_INFO  = 0  ,
	
	// 获取专题信息
	XIMALAYA_GET_SPECIAL_INFO ,

	// 获取专辑信息
	XIMALAYA_GET_ALBUM_INFO ,

	// 获取内容专辑分类
	XIMALAYA_GET_CONTEXT_ALBUM_TYPE ,
	
	// 获取分类专辑
	XIMALAYA_GET_TYPE_ALBUM ,

	//根据关键字搜索专辑
	XIMALAYA_GET_KEY_SEARCH_ALBUMS,

	//根据关键字搜索音频
	XIMALAYA_GET_KEY_SEARCH_AUDIO,
	
	//功能:语音搜索热门推荐专辑
	XIMALAYA_VOICE_SEARCH_RECOMMENDED_ALBUMS,

	// 获取首页导航列表
	XIMALAYA_GET_HOMEPAGE_NAVIGATION_LIST, 

	// 获取频道卡片信息
	XIMALAYA_GET_CHANNEL_CARD_INFO ,
	
	// 搜索热词
	XIMALAYA_GET_SEARCH_HOT_WORDS ,



	/*
	*    4.2
	*/
	// 获取二维码信息
	XIMALAYA_GET_QRCODE_LOGIN_INFO,

	// 获取二维码状态
	XIMALAYA_GET_QRCODE_STATUS,

	//刷新token
	XIMALAYA_REFRESH_TOKEN,

	//获取已购专辑
	XIMALAYA_GET_PURCHSED_ALBUM,

	// 查询用户 VIP 状态
	XIMALAYA_GET_USER_VIP_STATUS  ,

	// 用户退出登录
	XIMALAYA_SET_LOGOUT_STATUS ,

	//获取临时token
	XIMALAYA_GET_TEMP_TOKEN , 

	/*
	*    4.3
	*/
	// 上报播放历史列表
	XIMALAYA_REPORT_PLAYBACK_HISTORY ,
	
	// 播放历史列表
	XIMALAYA_GET_PLAYBACK_HISTORY ,


	/*
	*    4.4
	*/
	// 查询用户 订阅是否专辑
	XIMALAYA_GET_ALBUM_IS_SUBSCRIBE  ,

	//专辑订阅
	XIMALAYA_ALBUM_SUBSCRIPTIONS ,
	
	//取消专辑订阅
	XIMALAYA_CANCEL_ALBUM_SUBSCRIPTIONS ,
	
	// 获取专辑订阅列表
	XIMALAYA_GET_ALBUM_SUBSCRIPTIONS_LIST ,

	

	/*
	*    4.5
	*/
	//热搜榜单
	XIMALAYA_GET_HOT_SEARCH_LIST  , 

	//获取客服信息
	XIMALAYA_GET_CUSTOMER_INFO , 
	
	// 获取会员协议信息
	XIMALAYA_GET_MEMBERSHIP_INFO ,
	
	 // 获取隐私协议信息
	XIMALAYA_GET_PRIVACY_INFO ,

	
	
	//获取免费音频地址
	XIMALAYA_GET_FREE_AUDIO_INFO ,

	//获取付费音频信息
	XIMALAYA_GET_PAID_AUDIO_INFO ,

	// 测试api
	XIMALAYA_TEST_API    ,

	
	XIMALAYA_API_MAX    ,
}ximalaya_cmd_t;


typedef void (*ximalaya_request_data_callback)(ximalaya_cmd_t cmd , char *data , void *user_data);


typedef struct {

	char *app_key ;
	char *secret ;
	char *device_id ;
	char *sn ; 
	char *product_type ; 
	int task_prio ; 
	int debug_level ; 
	
	ximalaya_request_data_callback ximalaya_callback;  // function close stream, NULL mean do not need to close. 
	void *user_data;

} ximalaya_cfg_t ; 

/*
	功能:喜马拉雅初始化
*/
void ximalaya_task_init(ximalaya_cfg_t *cfg) ; 


/*
	功能:去喜马拉雅初始化
*/
void  ximalaya_task_deinit(void);




/****************************************************************************************

					4.2
	
*****************************************************************************************/

/*
	功能:获取二维码信息
参数
	qrcode_type = 1   只需传 qrcode_type 、 is_vip  参数 ，其他参数填NULL

	qrcode_type =( 2、3、4 )  如果已经登录 access_token、refresh_token、uid、cookie也需要传入


结果	
	通过回调函数返回，数据格式参考喜马拉雅文档
*/

//int ximalaya_get_qrcode_login_info(int qrcode_type);
int ximalaya_get_qrcode_login_info(int qrcode_type , 
											  const char *is_vip ,
											  const char *access_token ,
										      const char *refresh_token,
										      const char *uid ,
										      const char *cookie ,
										      const char *album_id , 
										      const char *source_id ,
										      const char *device_info );



/*
功能
	获取二维码状态 (在显示二维码状态，需要轮询此接口3-5秒请求一次，直到获取到结果或者退出二维码界面)

参数
	char * sid
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_qrcode_status_api(const char *sid);


/*
	功能:刷新令牌

参数
	char * refresh_token
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int  ximalaya_refresh_token_api(const char *refresh_token);


/*
	功能:获取已购专辑
	
参数
	char * sort  asc=正序，desc=倒序
	char * access_token
	char * user_id  喜马拉雅用户 id
	int page 当前页
	int count 每页大小
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int  ximalaya_get_purchased_albums_api(const char *access_token ,  const char *user_id , const char*sort , int page ,int count);

/*
	功能:查询用户 VIP 状态

参数
	char * access_token
	char * user_id

	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/

int ximalaya_get_user_vip_status_api(const char * access_token , const char * user_id);
/*
	功能: 用户退出登录
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int  ximalaya_set_logout_api(void);

/*
	功能: 获取临时令牌
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int  ximalaya_get_temp_token_api();




/****************************************************************************************

					4.1
	
*****************************************************************************************/
/*
	功能:获取卡片信息
参数
	int page 
	int count 
	int rows
	int columns
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int  ximalaya_get_card_info_api(int page ,int count ,int rows, int columns );

/*
	功能:获取专题信息
	
参数
	const char *id 专辑ID
	int page 当前页
	int count 每页大小
	int rows  图片宽度
	int columns   图片高度
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_special_info_api(const char *id , int page ,int count , int rows, int columns );

/*
	功能:获取专辑信息
	
参数
	sort 排序 asc=正序，desc=倒序
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_album_info_api(const char *album_id, int page ,int count , int rows, int columns ,const char * sort);

/*
	功能:获取分类专辑
参数
	int category_id   分类 id
	int page   页
	int count  页大小
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_type_album_api(int category_id , int page ,int count );


/*
	功能:根据关键字搜索音频

参数
	int page  页码,默认值为 1
	int count 每页展示记录数，默认为 20
	char * q  搜索关键字
	int sort_type 默认值 1， 排序类型(1=综合排序/2=更新时间排序/3-收藏量排序)
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int  ximalaya_get_key_search_audio_api(int page , int count , const char *q , int sort_type );


/*
	功能：根据关键字搜索专辑
参数
	const char * access_token  用户令牌
	int page  页
	int count 页大小
	const char * q 搜索关键字
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int  ximalaya_get_key_search_albums_api(const char *access_token , int page , int count , const char *q);

/*
	功能:语音搜索热门推荐专辑
参数
	int page   页码,默认值为 1
	int count   每页展示记录数，默认为 20
	int rows  专辑封面宽度
	int columns  专辑封面高度
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_voice_search_recommended_albums_api(int page ,int count , int rows ,int columns);

/*
	功能:获取首页导航列表
@param
	int rows 专辑封面宽度
	int columns 专辑封面高度
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_homePage_navigation_list_api(int rows , int columns);

/*
* 频道卡片信息
	const char *channel_id 频道ID号
参数
	int page 当前页
	int count 每页大小
	int rows  图片宽度
	int columns   图片高度
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档

*/
int ximalaya_get_channel_cards_info_api(const char *channel_id , int page , int count , int rows ,int columns);

/*
功能:获取搜索热词
	
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_search_hot_words_api();






/****************************************************************************************

					4.3
	
*****************************************************************************************/

/*
功能
	获取播放历史
参数
	- access_token : 访问令牌
	- history_array ：播放历史数组字符串
	[{
	"album_id":316453,
	"baby_id":1395050002,
	"break_second":34,
	"duration":34,
	"end_at":1645688418348,
	"start_at":1645688057047,
	"track_id":4655155000
	}]
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/

int ximalaya_report_playback_history_api( const char* access_token , const char *history_array );

/*
功能
	获取播放历史
参数
	- access_token : 访问令牌
	- baby_id ：宝宝 id (登录时候获取的)
	- count ：每页数量
	- page: 当前页
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/

int ximalaya_get_playback_history_api(const char* access_token , const char *baby_id ,int count , int page);


/****************************************************************************************

					4.4
	
*****************************************************************************************/
/*
功能
	查询用户 专辑是否订阅
参数
	- access_token: 访问令牌
	- album_id: 专辑 id
	- baby_id :宝宝 id (登录时候获取的)
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/

int ximalaya_get_albums_is_subscribe_api(const char *access_token , const char * album_id , const char *baby_id );

/*
功能
	专辑订阅
参数
	- access_token: 访问令牌
	- album_id: 专辑 id
	- baby_id :宝宝 id (登录时候获取的)
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/

int ximalaya_album_subscribe_api(const char *access_token , const char * album_id , const char *baby_id );


/*
功能
	获取专辑订阅列表
参数
	- access_token: 访问令牌
	- baby_id :宝宝 id (登录时候获取的)
	- page :
	- count :
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_album_subscribe_list_api(const char *access_token , const char *baby_id , int page , int count );

/*
功能
	取消专辑订阅
参数
	- access_token: 访问令牌
	- album_id: 专辑 id
	- baby_id :宝宝 id (登录时候获取的)
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_cancel_album_subscribe_api(const char *access_token , const char * album_id , const char *baby_id );

/****************************************************************************************

					4.5
	
*****************************************************************************************/
/*
功能
	热搜榜单

结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_hot_search_list_api(void);

/*

功能
	获取客服信息
结果
	通过回调函数返回，数据格式参考喜马拉雅文档

*/
int ximalaya_get_customer_info_api(void);

/*
功能
	获取会员协议信息
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/

int ximalaya_get_membership_info_api(void);

/*
功能
	获取会员协议信息
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_privacy_info_api(void);


/****************************************************************************************

					4.7
	
*****************************************************************************************/

/*
功能
	获取免费音频信息
参数
	- access_token: 访问令牌
	- track_id :声音 id
	- track_quality_level: 1 是获取高品质，0 是低品质
	- album_id: 专辑 id
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_free_audio_info_api(const char *access_token , uint64_t track_id , int track_quality_level , uint64_t album_id );


/*
功能
	获取付费音频信息
参数
	- access_token: 访问令牌
	- track_id :声音 id
	- track_quality_level: 1 是获取高品质，0 是低品质
	- album_id: 专辑 id
结果
	通过回调函数返回，数据格式参考喜马拉雅文档
*/
int ximalaya_get_paid_audio_info_api(const char *access_token , uint64_t track_id , int track_quality_level , uint64_t album_id );



#ifdef __cplusplus
}
#endif


#endif//_XIMALAYA_H_


