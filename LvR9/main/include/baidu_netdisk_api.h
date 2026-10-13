/*************************************************************************
	百度网盘api接口 

	百度网盘参数说明参考  https://pan.baidu.com/union/doc/Bl0eta7z8

**************************************************************************/
#ifndef _BAIDU_NETDISK_API_H_
#define _BAIDU_NETDISK_API_H_	



#ifdef __cplusplus
extern "C" {
#endif

/**
* 定义了一个枚举类型baidu_cmd_t，该类型包含了一系列与百度网盘操作相关的命令标识。
* 每个命令标识都对应一个整数值，从0开始。
*/
typedef enum{
	BAIDU_GET_LOGIN_QRCODE_INFO = 0 ,//用于获取登录百度网盘所需的二维码信息
	BAIDU_GET_QRCODE_LOGIN_STATUS ,  //查询二维码登录的状态
	BAIDU_GET_USER_INFO ,            //用于获取用户的基本信息
	BAIDU_GET_FILE_LIST  ,           //用于获取指定目录或根目录下的文件列表
	BAIDU_GET_FILE_TYPE_LIST  ,      //用于获取文件类型的列表，比如图片、文档、视频等分类
	BAIDU_GET_DIR_RECURSION_LIST,    //用于递归地获取目录及其子目录下的文件列表,会遍历整个目录树，并返回所有文件的列表。
	BAIDU_GET_FILES_INFO ,           //用于获取一个或多个文件的详细信息，比如文件大小、修改时间、文件哈希等
	BAIDU_DELETE_FILES,              //删除指定的一个或多个文件	
	BAIDU_REFRESH_TOKEN,			 //更新访问令牌access_token

}baidu_cmd_t;

/**
* 函数指针类型baidu_request_data_callback
* baidu_cmd_t cmd：baidu_cmd_t类型的命令标识
* char *data：字符指针（可能是响应数据）
* void *user_data：void指针（可能是用户自定义的数据）
* 返回值：无（返回类型为void）
*/
typedef void (*baidu_request_data_callback)(baidu_cmd_t cmd , char *data , void *user_data);


typedef struct {

	char *app_key ; // Baidu network disk AppKey , Required parameters 百度网盘AppKey，必填参数。
	char *secret ;  // Baidu network disk secret , Required parameters 百度网盘密钥，必填参数。
	char *device_id ;//device_id, Required parameters 设备ID，必填参数。
	int task_prio ; //Task priority 任务优先级。
	int task_core ;//Task task_core 任务核心（此字段的具体含义可能需要根据具体的实现或文档来确定）。
	int debug_level; //1 error  , 2 warn , 3 info  , 4 debug 调试级别，1表示错误，2表示警告，3表示信息，4表示调试。
	baidu_request_data_callback baidu_callback;  // Baidu network disk status callback  百度网盘状态回调函数，类型为baidu_request_data_callback。
	void *user_data; //用户自定义数据指针。

} baidu_cfg_t ; 




/********************************************************************************************************************
功能: 百度网盘初始化

@param
	-cfg :baidu_cfg_t


*********************************************************************************************************************/
void baidu_netdisk_task_init(baidu_cfg_t *cfg) ; 





/********************************************************************************************************************
功能: 百度网盘去初始化

*********************************************************************************************************************/
void  baidu_netdisk_task_deinit(void);


/********************************************************************************************************************
功能:获取二维码URL

返回参数
{
	"device_code": "53ce3f60934d1ed108a63c83c6e6d813",
	"user_code": "bw9ju5ay",
	"verification_url": "https://openapi.baidu.com/device",
	"qrcode_url": "https://openapi.baidu.com/device/qrcode/9cb9801da820d292148478256f7aa270/bw9ju5ay",
	"expires_in": 300,
	"interval": 5
}
拼接的二维码url:     https://openapi.baidu.com/device?display=mobile&code=bw9ju5ay


*********************************************************************************************************************/
int get_baidu_netdisk_login_qrcode_api();



/********************************************************************************************************************
	功能: 获取百度网盘二维码登录状态

	提示:显示二维码时候，轮询扫描状态6-10轮询一次 (轮询时间必须大于5秒 ，否则查询不到结果 ， device_code 有效时间为300秒) ，登录成功和退出二维码界面，结束轮询
	
	access_token 和 refresh_token 需要保存，第二次进入判断是否存在，存在已经登录 ， access_token 30天过期，过期后需要使用refresh_token更新
@param 
	- device_code : 

{
	"expires_in": 2592000,
	"refresh_token": "127.791edd323e889b2d312c226acb70f95c.Ya7astQZxDQZ5BllLOw20wTD6Q53PioOBrPD6cx.SSWhvw",
	"access_token": "126.f565aa8a3fb0e805986cc774e84e5707.Ya4R88va2zXogYWkJ-qwSNBIr4epScyQKQCP8pO.H_CEHg",
	"session_secret": "",
	"session_key": "",
	"scope": "basic netdisk"
}


{
	"expires_in": 2592000,
	"refresh_token": "127.d1bfebe85afe64c5ad40cbf55f3f76aa.Y7dKQYsTZRmAi2F1OgqpqaCN6Q5E6L6hX7JOpuO.KvokUQ",
	"access_token": "126.5627478a2eca3d6abe28b5828b915495.YmDPn44f3fPshWKLqF23v9qxgHRq0Dk5vjI9K4n.mVudTA",
	"session_secret": "",
	"session_key": "",
	"scope": "basic netdisk"
}

*********************************************************************************************************************/

int get_baidu_netdisk_login_status_api(const char * device_code);



/********************************************************************************************************************
	功能:获取百度网盘个人信息
	
@param 
	- access_token : 访问令牌
	

*********************************************************************************************************************/
int get_baidu_netdisk_user_info_api(const char * access_token);



/********************************************************************************************************************
	功能:获取百度网盘文件列表
@param 
	- access_token : 访问令牌
	- dir：需要list的目录，以/开头的绝对路径
	- order：排序字段：默认为name；time表示先按文件类型排序，后按修改时间排序；name表示先按文件类型排序，后按文件名称排序；(注意，此处排序是按字符串排序的，如果用户有剧集排序需求，需要自行开发)size表示先按文件类型排序，后按文件大小排序。
	- start：起始位置，从0开始
	- limit:查询数目，建议最大不超过50
	- folder :是否只返回文件夹，0 返回所有，1 只返回文件夹，且属性只返回path字段
	- showempty:是否返回dir_empty属性，0 不返回，1 返回
*******************************************************************************************************************/
int get_baidu_netdisk_file_list_api(const char * access_token , const char *dir , const char * order ,
									int start , int limit ,int folder ,int showempty );

/******************************************************************************************************************
	功能:获取百度网盘分类文件列表(比如指定文件类型)
@param
	-access_token : 访问令牌
	-category:文件类型，1 视频、2 音频、3 图片、4 文档、5 应用、6 其他、7 种子多个category使用英文逗号分隔，示例：3,4
	-parent_path:目录名
	-recursion:是否需要递归，0 不递归、1 递归 （注意recursion=1时不支持show_dir=1）
	-ext:需要的文件格式，多个格式以英文逗号分隔，示例: txt,epub，默认为category下所有格式
	-order:排序字段：time按修改时间排序，name按文件名称排序(注意，此处排序是按字符串排序的，如果用户有剧集排序需求，需要自行开发)，size按文件大小排序，默认为time
	-show_dir :是否展示文件夹，0:否(默认) 1:是
	-desc:0为升序，1为降序，默认为1
	-start:查询起点，默认为0
	-limit:查询数目，最大不要大于50，建议20
*******************************************************************************************************************/
int get_baidu_netdisk_file_type_list_api(const char * access_token , const char *category ,const char *parent_path ,
const char *ext , const char *order ,const char *desc,int show_dir ,int recursion ,int start ,int limit);



/******************************************************************************************************************

	功能:获取百度网盘目录递归
	
@param
	- access_token : 访问令牌
	-path:目录名称绝对路径，必须/开头
	-order:排序字段：time按修改时间排序，name按文件名称排序(注意，此处排序是按字符串排序的，如果用户有剧集排序需求，需要自行开发)，size按文件大小排序，默认为time
	-desc:0为升序，1为降序，默认为1
	-recursion:是否递归，0为否，1为是
	-start:查询起点，默认为0
	-limit:查询数目，最大50 ，建议20
*********************************************************************************************************************/
int get_baidu_netdisk_dir_recursion_list_api(const char * access_token , const char *path , const char * order ,const char *desc , 
									int recursion , int start , int limit );

/******************************************************************************************************************

	功能:获取百度网盘查询文件信息

	- access_token : 访问令牌
	-fsids: fs_id 文件id 例如:"414244021542671" 、"414244021542671,633507813519281"
	-dlink:是否需要下载地址，0为否，1为是，获取到dlink后，参考下载文档进行下载操作
	-thumb:是否需要缩略图地址，0为否，1为是，
	-extra:图片是否需要拍摄时间、原图分辨率等其他信息，0 否、1 是，默认0
	-needmedia:视频是否需要展示时长信息，needmedia=1时，返回 duration 信息时间单位为秒 （s），转换为向上取整。0 否、1 是，默认0
	-detail:视频是否需要展示长，宽等信息。0 否、1 是，默认0
*********************************************************************************************************************/
int get_baidu_netdisk_file_info_api(const char * access_token , const char *fsids , int dlink ,  int thumb , int extra,
									int needmedia , int detail  );

/******************************************************************************************************************

	功能:获取百度网盘删除文件或者路径

	- access_token : 访问令牌
	- array： 删除文件、目录路径数组
	单个：const char * array="[\"/music/test\"]"
	多个：const char * array="[\"/music/test\",\"/music/test1\"]"
*********************************************************************************************************************/
int baidu_netdisk_delete_file_api(const char * access_token ,	const char *array);



/******************************************************************************************************************

	功能:更新access_token


	注意:Access Token 有效期30天，过期后支持刷新，刷新后的 Access Token 有效期仍为 30 天，刷新Access Token请按需刷新，不需要不停的刷新。

	刷新请求，如果API返回失败，旧的refresh_token会失效，此时需要重新发起授权请求，获取新的 Access Token、refresh_token，而不是使用旧的 refresh_token 循环再发起刷新请求。

	refresh_token 只支持使用一次，refresh_token 使用后失效，下次刷新 Access Token 时需要使用上一次刷新请求响应中的 refresh_token。

	获取到的授权码 code 有效期 10 分钟，且仅一次有效。

	- refresh_token : 
	
*********************************************************************************************************************/
int baidu_netdisk_refresh_token_api(const char * refresh_token);


#ifdef __cplusplus
}
#endif

#endif//_BAIDU_NETDISK_API_H_
