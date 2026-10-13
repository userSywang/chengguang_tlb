//========================================================[]
//Name: tch_mtrl_init.c
//Function: 
//========================================================[]
/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#include "../zb_base.h"
#include "../utils/misc_utils.h"
#include "../ex_widget/app_msg.h"
#include "./tch_mtrl.h"
#include "../include/img_a_tlb.h"
#include "../data_parse/img_res.h"
#include "book_select.h"




static const char* TAG = "TEXTBOOK";

//=========================================================[]
//=========================================================[]

static uint16_t g_subjectSel = 0;
static int8_t g_ItemSel = 0;
lv_obj_t  * g_tchMnuBgWin;
uint8_t g_tchCurWin = 0;

//用于选择的缓冲变量
static uint8_t g_seledGrade = 0;  // 0:小学  1:初中  2:高中  3:全部
static uint8_t g_seledPublisher = 0;  // 0:全部  1:人教新起点  2:人教PEP

static char g_selectedPublisher[][100] = { "人教PEP版","人教统编版"}; //教材全解沪教版  人教PEP版
//static char g_seledChnPublisher[100] = { "人教统编版" }; //人教五四版

//当确认修改后 会将缓冲变量值写入到这里 实际使用用这个
static uint16_t m_seledGrade[] = { 0,0 };  // 0:小学  1:初中  2:高中  3:全部
static uint16_t m_seledPublisher[] = { 0,0 };  // 0:全部  1:人教新起点  2:人教PEP

static const char* c_subject[] = { "英语", "语文"};// , "全部"};
static const char* c_grades[] = { "小学", "初中", "高中" };// , "全部"};
//static const char * publishers[] = {"全部", "人教新起点", "人教PEP"};
static lv_style_t style_btn;
static lv_style_t age_btn_selected_style;
lv_obj_t * cfg_win_grade_btn[4] = {NULL};
lv_obj_t * cfg_win_publisher_btn[MAX_VERSION_NUM] = {NULL};

static lv_obj_t* g_publish_sel_win = NULL;
static lv_obj_t* g_confirm_cfg = NULL;

static lv_obj_t* focus_obj;
static download_data_t g_down_data;

void _tch_mtrl_free();

//exp
//static const book_count = 4;
static const char * textName = "一年级上（小学\n人教新起点版...";
static const unsigned int Item_img[] = {
	TEXT_BOOK_ICON_IMG_TEXTBOOK1,
	TEXT_BOOK_ICON_IMG_TEXTBOOK2,
	TEXT_BOOK_ICON_IMG_TEXTBOOK3,
	TEXT_BOOK_ICON_IMG_TEXTBOOK4,
};

uint16_t return_g_subjectSel()
{
	return g_subjectSel;
}

void _add_publisher_list(int grade);

//输入从json中获取的教材名，输出用于显示的格式化名称
char* get_text_book_name(char* name){
	//todo 传入一个正常name 例如“一年级上（小学人教新起点版）英语”
	//返回进过格式化可以传入label用于显示的name，例如“一年级上（小学\n人教新起点版...”
	return name;
}

static void msgbox_timeout_1_cb(lv_timer_t* timer)
{
	// msg 弹出过后，背后的焦点窗口给变化了，设置回来
	//lv_group_focus_obj(lv_obj_get_child(play_main_win, 0));
	lv_group_focus_obj(focus_obj);
}
static void pop_msg_downing(lv_event_t* e) {
	focus_obj = lv_event_get_target(e);
	app_msgbox_timeout("", "     当前该教材无离线数据\n         后续实现联网下载!", 1500, msgbox_timeout_1_cb, NULL);
	return;
}

lv_obj_t* tchMtrl_createEscBtn(lv_obj_t * parent, lv_event_cb_t clickCb, int extra,int start_x, int start_y)
{
	const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
	lv_obj_t * context = lv_obj_create(parent);
	lv_obj_set_size(context, pImgDsc->header.w + TOP_BTN_BORDER * 2 + 10, pImgDsc->header.h + TOP_BTN_BORDER * 2 + 10);
	lv_obj_set_pos(context, 11, 2);
	lv_obj_set_style_border_width(context, 0, 0);

	lv_obj_t * img = lv_img_create(context);
	lv_img_set_src(img, pImgDsc);
	lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
	lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(img, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(img, 4, 0);
	lv_obj_add_flag(context, LV_OBJ_FLAG_CLICKABLE);

	lv_color_t colorBg = lv_color_hex(0xc0c0c0);
	lv_obj_set_style_bg_color(img, colorBg, 0);
	lv_obj_set_style_bg_opa(img, LV_OPA_0, 0);
	
	lv_color_t colorBorderFcs = lv_color_white();
	lv_obj_set_style_border_color(img, colorBorderFcs, 0);
	lv_obj_add_event_cb(context, clickCb, LV_EVENT_CLICKED, NULL);


	return img;
}

static void _tchMtrl_lst_scr_esc_click_cb(lv_event_t * evt)
{
	zb_requestSwitchAp(C_ZB_AP_MAINMENU);
	return;
}

static void _tchMtrl_grade_btn_click_cb(lv_event_t * e){
    if (e->code!= LV_EVENT_CLICKED) return;
    lv_obj_t* obj = lv_event_get_target(e);
    int userData = (int)lv_obj_get_event_user_data(obj, _tchMtrl_grade_btn_click_cb);
    printf("_tchMtrl_grade_btn_click_cb %d clcked\n",userData);
	g_seledGrade = userData; // m_seledGrade[g_subjectSel];
    for(int i = 0; i < 3; i++){
        if(i == userData){
            lv_obj_remove_style(cfg_win_grade_btn[i], &style_btn, 0);
            lv_obj_add_style(cfg_win_grade_btn[i], &age_btn_selected_style, 0);
        }else{
            lv_obj_remove_style(cfg_win_grade_btn[i], &age_btn_selected_style, 0);
            lv_obj_add_style(cfg_win_grade_btn[i], &style_btn, 0);
        }
    }
	lv_obj_clean(g_publish_sel_win);
	_add_publisher_list(userData);
	return;
}

static void _tchMtrl_publisher_btn_click_cb(lv_event_t * e){
    if (e->code!= LV_EVENT_CLICKED) return;
    lv_obj_t* obj = lv_event_get_target(e);
    int userData = (int)lv_obj_get_event_user_data(obj, _tchMtrl_publisher_btn_click_cb);
    printf("_tchMtrl_publisher_btn_click_cb %d clcked\n",userData);
    g_seledPublisher = userData;


    for(int i = 0; i < get_version_sum(); i++){
        if(i == userData){
            lv_obj_remove_style(cfg_win_publisher_btn[i], &style_btn, 0);
            lv_obj_add_style(cfg_win_publisher_btn[i], &age_btn_selected_style, 0);
        }else{
            lv_obj_remove_style(cfg_win_publisher_btn[i], &age_btn_selected_style, 0);
            lv_obj_add_style(cfg_win_publisher_btn[i], &style_btn, 0);
        }
    }
	return;
}

static void _tchMtrl_cfg_comform_click_cb(lv_event_t * e){
    if (e->code!= LV_EVENT_CLICKED) return;
    lv_obj_t* obj = lv_event_get_target(e);

	// 获取所点击版本列表中的 版本内容：
	lv_obj_t* btn = lv_obj_get_child(g_publish_sel_win, g_seledPublisher);
	lv_obj_t* label = lv_obj_get_child(btn, 0);
	const char* version = lv_label_get_text(label);
	LV_LOG_USER("Button clicked: %s", version);

	tch_mtrl_cfg_setting_del(FUN_CONFIRM, version);
	return;
}

static void download_finished() {
	lv_obj_set_style_img_recolor_opa(g_down_data.cover_obj, LV_OPA_TRANSP, 0);
	lv_obj_clean(g_down_data.cover_obj);
}

static void update_progress_timer() {
	static cc = 0;
	cc = cc + 1;
	printf("cc= %d, enter timer:total_bytes = %ld,cur=%ld \n",cc, g_down_data.total_bytes, g_down_data.downloaded_bytes);
	//if (g_down_data.is_downloading)
	//	printf("TRUE.......\n");
	//else
	//	printf("FALSE.......\n");
	//if (g_down_data.total_bytes > 0)
	//	printf("total_bytes > 0\n");
	//else
	//	printf("total_bytes == 0\n");
	if (g_down_data.is_downloading && g_down_data.total_bytes > 0) {
		// 计算进度百分比
		int progress = (int)((g_down_data.downloaded_bytes * 100) / g_down_data.total_bytes);
		lv_bar_set_value(g_down_data.progress_bar, progress, LV_ANIM_OFF);

		printf("progress: = %d\n", progress);
		// 可选：显示下载速度
		/*static uint32_t last_time = 0;
		static curl_off_t last_bytes = 0;
		uint32_t now = lv_tick_get();
		if (last_time > 0 && now - last_time > 1000) {
			double speed_kbps = (g_down_data.downloaded_bytes - last_bytes) / 1024.0;
			last_bytes = g_down_data.downloaded_bytes;
			last_time = now;
			printf("Speed: %.2f KB/s\n", speed_kbps);
		}*/

	}
}

bool create_thread_download(download_data_t *config);
// 新增回调函数
static void _item_click_handler(lv_event_t * e) {
    if (e->code == LV_EVENT_CLICKED) {
        const char* text = (const char*)lv_event_get_user_data(e);
        printf("教材被点击: %s\n", text);
		char buf[400];
		sprintf(buf, "/%s/_data/textbook/books/%s/book000/album.mnu", BOOK_PATH, text);
		printf("judge file exist: %s\n", buf);
		if (!zb_isFileExist(buf)) {
		//if (strlen(text) ==0) {
			/*
			lv_obj_t* obj = lv_event_get_target(e); // list: item
			lv_obj_t* cover_img = lv_obj_get_child(obj,0); //cover
			lv_obj_set_style_img_recolor_opa(cover_img, LV_OPA_60, 0);
			lv_obj_clean(cover_img);
			
			lv_obj_t* bar = lv_bar_create(cover_img);
			lv_obj_set_size(bar, 108, 12);
			lv_bar_set_range(bar, 0, 100);
			lv_bar_set_value(bar, 0, LV_ANIM_OFF);
			lv_obj_align_to(bar, cover_img, LV_ALIGN_CENTER, 0, 0);

			lv_obj_t* downing_label = lv_label_create(cover_img);
			lv_label_set_text(downing_label, "正在下载");
			lv_obj_align_to(downing_label, cover_img, LV_ALIGN_CENTER, 0, 25);
			lv_obj_set_style_text_color(downing_label, lv_color_white(), 0);


			// 创建并应用样式
			static lv_style_t style_bar;
			static lv_style_t style_indicator;
			// 初始化样式
			lv_style_init(&style_bar);
			lv_style_init(&style_indicator);

			// 设置进度条背景样式（圆角、边框等）
			lv_style_set_radius(&style_bar, 10);  // 10像素圆角
			lv_style_set_bg_color(&style_bar, lv_color_white());  // 深灰色背景
			lv_style_set_bg_opa(&style_bar, LV_OPA_COVER);
			lv_style_set_border_width(&style_bar, 0);  // 无边框

			// 设置指示器样式（白色、圆角）
			lv_style_set_radius(&style_indicator, 10);  // 与背景相同的圆角
			//lv_style_set_bg_color(&style_indicator, lv_color_hex(0x00FF00));  // 白色进度条
			lv_style_set_bg_color(&style_indicator, lv_color_hex(0x008cFF));  // 白色进度条
			lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);

			// 应用样式
			lv_obj_add_style(bar, &style_bar, LV_PART_MAIN);  // 应用到背景
			lv_obj_add_style(bar, &style_indicator, LV_PART_INDICATOR);  // 应用到进度指示器
			lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
			//pop_msg_downing(e);

			memset(&g_down_data, 0, sizeof(download_data_t));

			g_down_data.progress_bar = bar;
			g_down_data.cover_obj = cover_img;
			//g_down_data.fp = NULL;
			//g_down_data.total_bytes = 0;
			//g_down_data.downloaded_bytes = 0;

			if (true) {
				sprintf(g_down_data.url, "%s/%s/book.bin", NET_BOOK_PATH, text);
				sprintf(g_down_data.save_path, "/%s/%s", BOOK_PATH, text);
#ifndef C_PC_SIM
				mkdir_p(g_down_data.save_path, 0777);
#else
				zb_mkdir(g_down_data.save_path,0777);
#endif
				//sprintf(g_down_data.save_path, "/%s/%s/album.bin", BOOK_PATH, text);
			}
			else {
				sprintf(g_down_data.url, "%s/%s", NET_BOOK_PATH, "boll/wyxbz3q/wyxbz3q_4_1/book.bin");
				sprintf(g_down_data.save_path, "%s", "/userdata/_data/book.bin");
			}
			printf("---url: =  %s\n", g_down_data.url);
			printf("---save_path: =  %s\n", g_down_data.save_path);

			//g_down_data.timer = lv_timer_create(update_progress_timer, 1000, NULL);

#ifndef C_PC_SIM
			//printf("before test_curl_bookVersion\n");
			//test_curl_bookVersion();
			//printf("-------before test_curl_book_bin\n");
			//test_curl_book_bin();
			printf("-------before test_curl_book_bin_one\n");
			//pthread_mutex_init(g_down_data.mutex, NULL);
			create_thread_download(&g_down_data);
			//test_curl_book_bin_one(&g_down_data);
#endif
*/
			pop_msg_downing(e);
		}
		else {
			tch_mtrl_ap_win_deinit();
			get_booknode_by_bookbin(text);
			tch_mtrl_content_init(text);
		}
    }
}

void tch_mtrl_cfg_setting_esc() {
	g_seledGrade = m_seledGrade[g_subjectSel];
	tch_mtrl_cfg_setting_del(FUN_CANCEL, "");
}

// 取消和确认调用这个函数
void tch_mtrl_cfg_setting_del(FUN_DIRECTION fun, const char *version){
	if (fun == FUN_CONFIRM) {

		if(m_seledGrade[g_subjectSel] != g_seledGrade || strcmp(g_selectedPublisher[g_subjectSel], version) != 0)
		{
			//strcpy(g_selectedPublisher[g_subjectSel], version);
			memcpy(g_selectedPublisher[g_subjectSel], version, strlen(version));
			g_selectedPublisher[g_subjectSel][strlen(version)] = '\0';
			fun = FUN_FORWARD;
		}
		else {
			fun = FUN_BACKWARD;
		}
		m_seledGrade[g_subjectSel] = g_seledGrade;
		m_seledPublisher[g_subjectSel] = g_seledPublisher;
		printf("_tchMtrl_cfg_comform_click_cb clcked ,grade = %d , publisher = %d\n", m_seledGrade[g_subjectSel], m_seledPublisher[g_subjectSel]);
	}
	else
		fun = FUN_BACKWARD;  // 按取消返回键也是当成返回处理

	tch_mtrl_ap_win_deinit();
	tch_mtrl_win_init(fun);
}

/*
CURL_EXTERN CURL *curl_easy_init(void);
CURL_EXTERN CURLcode curl_easy_setopt(CURL *curl, CURLoption option, ...);
CURL_EXTERN CURLcode curl_easy_perform(CURL *curl);
CURL_EXTERN void curl_easy_cleanup(CURL *curl);
*/
// 用于接收响应数据的回调函数
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	printf("%.*s", (int)realsize, (char*)contents);
	return realsize;
}
#ifndef C_PC_SIM
//获取某一个版本 年级的教材列表
int test_curl_bookList()
{
	/*
	传入参数： 
	{"appid":"88f4f2dbdb39241eecd8aa03353141f4","user_id":"TLB04697EC8912F0","pathName":"/api/sdk/audio/query_aduio_booklist","subject":"英语","grade":"六年级","book_version":"重大版","token":"d6b71139dc14aa4e455e3375d05b44c8"}
	http="http://thirdparty.bolledu.net/audio/api2"
	返回值：
	{"msg":"SUCCESS","code":0,"data":[{"book_version":"重大版","grade":"六年级","book_full_name":"六年级上英语（重大版）","icon":"","publisher":"","term":"","book_id":"tape6a_020002_202409022312"},{"book_version":"重大版","grade":"六年级","book_full_name":"六年级下英语（重大版）","icon":"","publisher":"","term":"","book_id":"tape6b_020002_202502230158"}]}
	*/
}
// 回调函数：保存响应内容
static size_t write_cb(void* data, size_t size, size_t nmemb, void* userp) {
	static int t = 0;
	size_t total = size * nmemb;
	t = t + total;
	//printf("百度响应内容（前200字节）: %.*s\n", (int)(total > 200 ? 200 : total), (char*)data);
	printf("total=[%d]; [%d]: %.*s\n",(int)t, (int)(total), (char*)data);
	/*
	{"msg":"SUCCESS","code":0,"data":{"authorize_version":"20240912200119","grade_book_version":{"语文":[{"grade":"初一","book_versions":["人教五四版","人教五四版","人教五四版","人教五四版","人教统编版","人教统编版"," 人教统编版","人教统编版"]},{"grade":"初三","book_versions":["人教五四版","人教五四版","人教统编版","人教统编版"]},{"grade":"初二","book_versions":["人教五四版","人教五四版","人教统编版","人教统编版"]},{"grade":"一年级","book_versions":["人教五四版","人教五四版","人教五四版","人教五四版","人教统编版","人教统编版","人教统编版","人教统编版"]},{"grade":"二年级","book_versions":["人教五四版","人教五四版","人教统编版","人教统编版"]},{"grade":"三年级","book_versions":["人教 五四版","人教五四版","人教统编版","人教统编版"]},{"grade":"四年级","book_versions":["人教五四版","人教五四版","人教统 编版","人教统编版"]},{"grade":"五年级","book_versions":["人教五四版","人教五四版","人教统编版","人教统编版"]},{"grade":"六年级","book_versions":["人教五四版","人教五四版","人教五四版","人教五四版","人教统编版","人教统编版"]}]}}}
	*/
	return total;
}

int test_curl_book_bin()
{
	CURL* curl;
	CURLcode res;
	//获取科目下所有的教材版本
	printf("======================curl 0");
	// 初始化 curl
	curl = curl_easy_init();
	if (curl) {
		// 设置请求 URL
		printf("======================curl 1\n");
		// 设置目标URL
		curl_easy_setopt(curl, CURLOPT_URL, "http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books/boll/wyxbz3q/wyxbz3q_4_1/book.bin");
		// 设置响应回调
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
		// 禁用证书验证
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		// 模拟浏览器User-Agent（避免被拦截）
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/116.0.0.0");

		// 执行请求
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "访问百度失败: %s\n", curl_easy_strerror(res));
			return 1;
		}
		curl_easy_cleanup(curl);

	}

	return 0;
}

// 写入数据的回调函数
size_t write_data_fp(void* ptr, size_t size, size_t nmemb, void* userdata) {
	static int t = 0;

	size_t total = size * nmemb;
	t = t + size * nmemb;
	//printf("write_data_fp: total=[%d]; [%d]: %.*s\n", (int)t, (int)(total), (char*)ptr);
	printf("write_data_fp: total=[%d]; [%d]\n", (int)t, (int)(total));
	download_data_t* data = (download_data_t*)userdata;
	if (data->fp == NULL)
		printf("file is null\n");
	size_t written = fwrite(ptr, size, nmemb, data->fp);
	return written;
}
/*
size_t write_data_split_fp(void* ptr, size_t size, size_t nmemb, void* userdata) {
	static int t = 0;

	size_t data_read_num = size * nmemb;
	t = t + size * nmemb;
	//printf("write_data_fp: total=[%d]; [%d]: %.*s\n", (int)t, (int)(total), (char*)ptr);
	printf("write_data_fp: total=[%d]; [%d]\n", (int)t, (int)(data_read_num));
	download_data_t* data = (download_data_t*)userdata;
	if (data->fp == NULL)
		printf("file is null\n");
	//size_t written = fwrite(ptr, size, nmemb, data->fp);

	if (data_read_num > 0)
	{
		int bytes = 0;
		remainLenInCache += data_read_num;
		ESP_LOGI(TAG, "data_buf %p", data_buf);
		int ret = write_split_book(&dsbt, (const unsigned char*)ptr, remainLenInCache, data_read_num);
		if (ret >= 0) {
		}
	}
	return data_read_num;
}*/


// 进度回调函数    dltotal（总大小）和 dlnow（已下载大小）。
static int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	//download_data_t* data = (download_data_t*)clientp;
	//printf("pcb: dltotal: %lld, dlnow: %lld\n", (long long)dltotal, (long long)dlnow);
	g_down_data.total_bytes = (uint32_t)dltotal;
	g_down_data.downloaded_bytes = (uint32_t)dlnow;
	printf("pcb: dltotal: %ld, dlnow: %ld\n", g_down_data.total_bytes, g_down_data.downloaded_bytes);
#ifndef C_PC_SIM
	usleep(2 * 1000);
#else
	Sleep(2 * 1000);
#endif
	lv_async_call(update_progress_timer, NULL);
	//update_progress_timer();

	return 0;
}


int test_curl_book_bin_one(void *arg)
{
	CURL* curl;
	CURLcode res;
	FILE* fp;
	download_data_t* down_cfg = (download_data_t*)arg;

	//const char* filename = "/userdata/_data/book.bin";
	//下载教材数据
	printf("======================curl 0\n");
	// 初始化 curl
	curl = curl_easy_init();
	if (curl) {

		printf("test_curl_book_bin_one: save_path(%p), %s\n", down_cfg->save_path, down_cfg->save_path);
		fp = fopen(down_cfg->save_path, "wb");
		if (!fp) {
			printf("打开文件失败：%s\n", down_cfg->save_path);
			return 1;
		}
		down_cfg->fp = fp;
		// 设置请求 URL
		printf("======================curl 1\n");
		// 设置目标URL
		curl_easy_setopt(curl, CURLOPT_URL, down_cfg->url);
		printf("======================curl 2\n%s\n", down_cfg->url);
		// 设置响应回调
		if (false) {
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_fp);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, down_cfg);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);      // 启用进度条
			// 禁用证书验证
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, down_cfg);
		}
		else {
			printf("======================curl 3\n");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, /*write_cb*/write_data_fp);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, down_cfg);
			
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);      // 启用进度条
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, down_cfg);
			
			//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // 启用详细日志
			//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // 禁用重定向
			//curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0L); // 禁用分块解码
			// 禁用证书验证
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		// 模拟浏览器User-Agent（避免被拦截）
		printf("======================curl 4\n");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/116.0.0.0");

		printf("======================curl 5\n");
		// 执行请求
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "访问百度失败: %s\n", curl_easy_strerror(res));
			return 1;
		}
		curl_easy_cleanup(curl);
		fclose(down_cfg->fp);
		down_cfg->is_downloading = false;
		lv_async_call(download_finished, NULL);
	}

	return 0;
}

// 启动下载任务
bool create_thread_download(download_data_t *config) {
	//pthread_mutex_lock(&config.mutex);
	printf("create_thread_download.......11\n");

	if (config->is_downloading) {
		//pthread_mutex_unlock(&config.mutex);
		return false;
	}

	config->is_downloading = true;
	//pthread_mutex_unlock(&config.mutex);

	// 创建下载线程
	// pthread_create(&thread_device_update, NULL, recv_device_update_hander, event_data);
	//if (pthread_create(&config->thread, NULL, download_thread, &g_state) != 0) {
	if (pthread_create(&config->thread, NULL, test_curl_book_bin_one, config) != 0) {
		printf("create_thread_download.......error\n");
		//pthread_mutex_lock(&config.mutex);
		config->is_downloading = false;
		//pthread_mutex_unlock(&config.mutex);
		return false;
	}

	printf("create_thread_download.......OK\n");
	// 分离线程（不关心返回值）
	pthread_detach(config->thread);

	return true;
}

int test_curl_bookVersion()
{
	CURL* curl;
	CURLcode res;
	//获取科目下所有的教材版本
	//const char* json_data = "{\"name\":\"John Doe\",\"age\":30,\"city\":\"New York\"}";
	const char* json_data = "{\"appid\":\"88f4f2dbdb39241eecd8aa03353141f4\", \"user_id\" : \"TLB04697EC8912F0\", \"pathName\" : \"/api/sdk/audio/query_book_version\", \"subject\" : \"语文\", \"token\" : \"d6b71139dc14aa4e455e3375d05b44c8\"}";
	printf("======================curl 0");
	// 初始化 curl
	curl = curl_easy_init();
	if (curl) {
		// 设置请求 URL
		printf("======================curl 1");
		curl_easy_setopt(curl, CURLOPT_URL, "http://thirdparty.bolledu.net/audio/api2");

		printf("======================curl 2");
		// 设置为 POST 请求
		curl_easy_setopt(curl, CURLOPT_POST, 1L);

		printf("======================curl 3");
		// 设置 POST 数据
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

		// 设置 POST 数据大小
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(json_data));

		// 设置响应数据回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		printf("======================curl 6");
		// 执行请求
		res = curl_easy_perform(curl);

		printf("======================curl 7");
		// 检查错误
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		}

		printf("======================curl 10");
		// 清理
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		/*
		// 设置目标URL
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
		// 设置响应回调
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
		// 禁用证书验证         
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		// 模拟浏览器User-Agent（避免被拦截）
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/116.0.0.0");

		// 执行请求
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "访问百度失败: %s\n", curl_easy_strerror(res));
			return 1;
		}
		curl_easy_cleanup(curl);
		*/

	}
	
	return 0;
}
#endif

// 根据 grade == -1 表示走默认项，否则有用户选择；
// 获取数据，添加版本列表；
void _add_publisher_list(int grade)
{
	int version_num;
	//char tmp_grade
	//if (grade != -1)
		// 暂时还不能更新进去，只有确认后才保存
		//m_seledGrade[g_subjectSel] = grade;
	//version_num = read_versions(c_subject[g_subjectSel], c_grades[m_seledGrade[g_subjectSel]]);
	version_num = read_versions(c_subject[g_subjectSel], c_grades[g_seledGrade]);

	if (grade == -1) {
		for (int i = 0; i < version_num; i++) {
			if (strcmp(g_selectedPublisher[g_subjectSel], get_version_by_id(i)) == 0)
			{
				m_seledPublisher[g_subjectSel] = i;
				break;
			}
		}
	}


	if (version_num > 0) {
		for (int i = 0; i < version_num; i++) {
			cfg_win_publisher_btn[i] = lv_btn_create(g_publish_sel_win);
			if (i != m_seledPublisher[g_subjectSel]) 
				lv_obj_add_style(cfg_win_publisher_btn[i], &style_btn, 0);
			else 
				lv_obj_add_style(cfg_win_publisher_btn[i], &age_btn_selected_style, 0);
			lv_obj_set_size(cfg_win_publisher_btn[i], 300, 45);
			lv_obj_t* label = lv_label_create(cfg_win_publisher_btn[i]);
#ifndef C_PC_SIM
			lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
#endif
			lv_label_set_text(label, get_version_by_id(i));
			lv_obj_center(label);
			lv_obj_add_event_cb(cfg_win_publisher_btn[i], _tchMtrl_publisher_btn_click_cb, LV_EVENT_CLICKED, i);
		}
		lv_obj_clear_flag(g_confirm_cfg, LV_OBJ_FLAG_HIDDEN);
	}
	else {
		lv_obj_add_flag(g_confirm_cfg, LV_OBJ_FLAG_HIDDEN);
	}
}

void tch_mtrl_cfg_setting_init(void) {

    printf("tch_mtrl_cfg_setting_init\n");
	g_tchCurWin = TCH_CFG_SETTING;
	lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t main_colorBg = lv_color_hex(TCH_MTRL_AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, main_colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, LV_PCT(100), TCH_HEADER_HEIGHT); // 60
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, main_colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	tchMtrl_createEscBtn(top_title_win, tch_mtrl_cfg_setting_esc, 0, 20 - TOP_BTN_BORDER, 14 - TOP_BTN_BORDER);// 16, 15);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
#endif
    lv_label_set_text(lblTitle, "选择学段");
	lv_obj_center(lblTitle);
	//lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, TCH_MTRL_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	lv_obj_t* title1 = lv_label_create(mainBg);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(title1, 18, FT_FONT_STYLE_NORMAL);
#endif
	lv_label_set_text(title1, "选择学习阶段");
	lv_obj_align_to(title1,top_title_win, LV_ALIGN_BOTTOM_LEFT, 8, 20);
	lv_obj_set_style_text_color(title1, lv_color_hex(0x808080), 0);

	lv_obj_t* grade_sel_win = lv_obj_create(mainBg);
	lv_obj_set_size(grade_sel_win, LV_PCT(100), 120);
	lv_obj_align_to(grade_sel_win, title1, LV_ALIGN_TOP_LEFT, -8, 25);
	lv_obj_set_style_border_width(grade_sel_win, 0, 0);
	lv_obj_set_style_pad_all(grade_sel_win, 0, 0);
	lv_obj_set_style_pad_row(grade_sel_win, 0, 0);
	lv_obj_set_style_pad_column(grade_sel_win, 0, 0);
	lv_obj_set_scroll_dir(grade_sel_win, LV_DIR_NONE);
	static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grade_sel_win, col_dsc, row_dsc);

	//static bool isFirst = true;
	if (style_btn.prop_cnt == 0) {
	//isFirst = false;
		lv_style_init(&style_btn);
		lv_style_set_bg_color(&style_btn, lv_color_hex(0xF5F5F5)); // 未选中时浅灰色背景
		lv_style_set_bg_opa(&style_btn, LV_OPA_100);
		lv_style_set_text_color(&style_btn, lv_color_hex(0x000000)); // 黑色字体 #333333
		lv_style_set_radius(&style_btn, 10);                         // 16px圆角
		lv_style_set_pad_hor(&style_btn, 20);                        // 水平内边距
		lv_style_set_pad_ver(&style_btn, 6);
		lv_style_set_border_width(&style_btn, 0); // 无边框
		lv_style_set_shadow_width(&style_btn, 0); // 无阴影

		lv_style_init(&age_btn_selected_style);
		lv_style_set_bg_color(&age_btn_selected_style, lv_color_hex(0xE6F0FF));     // 浅蓝色背景 #E6F0FF
		lv_style_set_text_color(&age_btn_selected_style, lv_color_hex(0x1A5FB4));   // 深蓝字体 #1A5FB4
		lv_style_set_border_width(&age_btn_selected_style, 0); 
		lv_style_set_radius(&age_btn_selected_style, 10);                         // 16px圆角
		lv_style_set_pad_hor(&age_btn_selected_style, 20);                        // 水平内边距
		lv_style_set_pad_ver(&age_btn_selected_style, 6);
		lv_style_set_shadow_width(&age_btn_selected_style, 0); // 无阴影
	}
	
	int len = sizeof(c_grades) / sizeof(c_grades[0]);
	for(int i = 0; i < len; i++){
		cfg_win_grade_btn[i] = lv_btn_create(grade_sel_win);
		if(i!=m_seledGrade[g_subjectSel]) lv_obj_add_style(cfg_win_grade_btn[i], &style_btn, 0);
		else lv_obj_add_style(cfg_win_grade_btn[i], &age_btn_selected_style, 0);
		lv_obj_set_size(cfg_win_grade_btn[i], 140, 45);
		lv_obj_t * label = lv_label_create(cfg_win_grade_btn[i]);
#ifndef C_PC_SIM
		lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
#endif
		lv_label_set_text(label, c_grades[i]);
    	lv_obj_center(label);
		lv_obj_set_grid_cell(cfg_win_grade_btn[i],
            LV_GRID_ALIGN_CENTER, i%2, 1,
            LV_GRID_ALIGN_CENTER, i/2, 1
        );
        lv_obj_add_event_cb(cfg_win_grade_btn[i], _tchMtrl_grade_btn_click_cb, LV_EVENT_CLICKED, i);
	}

	lv_obj_t* title2 = lv_label_create(mainBg);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(title2, 18, FT_FONT_STYLE_NORMAL);
#endif
	lv_label_set_text(title2, "选择版本");
	lv_obj_align_to(title2,grade_sel_win, LV_ALIGN_BOTTOM_LEFT, 8, 25);
	lv_obj_set_style_text_color(title2, lv_color_hex(0x808080), 0);

	lv_obj_t* publish_sel_win = lv_obj_create(mainBg);
	lv_obj_set_size(publish_sel_win, LV_PCT(100), 168);
	lv_obj_align_to(publish_sel_win, title2, LV_ALIGN_TOP_LEFT, -8, 25);
	lv_obj_set_style_border_width(publish_sel_win, 0, 0);
	lv_obj_set_style_pad_all(publish_sel_win, 0, 0);
	lv_obj_set_style_pad_row(publish_sel_win, 10, 0);
	lv_obj_set_style_pad_column(publish_sel_win, 0, 0);
	lv_obj_set_style_pad_top(publish_sel_win, 10, 0);
	lv_obj_set_style_pad_bottom(publish_sel_win, 10, 0);
	lv_obj_set_scroll_dir(publish_sel_win, LV_DIR_VER);

	lv_obj_set_flex_flow(publish_sel_win, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(publish_sel_win, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_border_opa(publish_sel_win, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(publish_sel_win, LV_OPA_0, 0);
	g_publish_sel_win = publish_sel_win;

	lv_obj_t *conform_btn = lv_btn_create(mainBg);
	lv_obj_set_size(conform_btn, 300, 45);
	lv_obj_align_to(conform_btn, publish_sel_win, LV_ALIGN_BOTTOM_MID, 0, 80);
	lv_obj_set_style_radius(conform_btn, 25, 0);
	lv_obj_t * label = lv_label_create(conform_btn);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
#endif
	lv_label_set_text(label, "确定");
    lv_obj_center(label);
	lv_obj_set_style_bg_opa(conform_btn, LV_OPA_100, LV_EVENT_PRESSED);
	lv_obj_set_style_bg_opa(conform_btn, LV_OPA_30, 0);
	lv_obj_set_style_shadow_width(conform_btn, 0, 0);
    lv_obj_add_event_cb(conform_btn, _tchMtrl_cfg_comform_click_cb, LV_EVENT_CLICKED,NULL);
	g_confirm_cfg = conform_btn;

	// 添加版本列表内容
	_add_publisher_list(-1);

	sys_setCurMainWinAndCb(mainBg, _tch_mtrl_scr_cb);
	g_tchMnuBgWin = mainBg;
}

static void switch_btn_clickCb(lv_event_t* evt) { 
    tch_mtrl_ap_win_deinit();
    tch_mtrl_cfg_setting_init();
    return; 
}

#if 0
lv_img_dsc_t* load_image_from_file(char* pcPath) {
	lv_img_dsc_t* stImgDsc = NULL;
	FILE* fp = NULL;
	do {
		fp = zb_fopen(pcPath, "rb");
		if (!fp) {
			ESP_LOGE(TAG, "Fail to open file %s", pcPath);
			return stImgDsc;
		}
		
		stImgDsc = heap_caps_malloc_dflt(sizeof(lv_img_dsc_t));
		zb_fread(stImgDsc, 1, 8, fp);
		LV_ASSERT(stImgDsc->data_size >= 1 && stImgDsc->data_size < 1024 * 400);
		void* imgBuf = heap_caps_malloc_dflt(stImgDsc->data_size);
		zb_fread(imgBuf, 1, stImgDsc->data_size, fp);
		stImgDsc->data = imgBuf;
	} while (0);

	if (fp) {
		zb_fclose(fp);   fp = NULL;
	}
	return stImgDsc;
}
#endif

int read_current_books()
{
	//static uint16_t g_subjectSel = 0;
	//static uint8_t g_seledGrade = 0;  // 0:小学  1:初中  2:高中  3:全部
	//static uint8_t g_seledPublisher = 0;  // 0:全部  1:人教新起点  2:人教PEP
	int bookSum = 0;
	//if (g_subjectSel == TCH_MTRL_TEXT_CN)
	//	bookSum = read_books("语文", c_grades[g_seledGrade], g_seledChnPublisher);
	//else if (g_subjectSel == TCH_MTRL_TEXT_ENG)
	//	bookSum = read_books("英语", c_grades[g_seledGrade], g_seledEngPublisher);
	_tch_mtrl_free();
	bookSum = read_books(c_subject[g_subjectSel], c_grades[m_seledGrade[g_subjectSel]], g_selectedPublisher[g_subjectSel]);
	return bookSum;
}

// 当前简单处理，后续优化
char* _chg_ln_book_name(char* name, char* dest) {
	char len = 18;
	strncpy(dest, name, len);
	strncpy(dest+len, "\n", 1);
	if (strlen(name + len) > len) {
		strncpy(dest + len + 1, name + len, len);
		*(dest + len+len+1) = '\0';
	}
	else
		strncpy(dest + len + 1, name + len, strlen(name + 18));
	return dest;
}


void tch_mtrl_win_init(FUN_DIRECTION direction) {
	if (direction == FUN_FORWARD)
		read_current_books();
    g_tchCurWin = TCH_TCH_MTRL;
    lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t main_colorBg = lv_color_hex(TCH_MTRL_AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, main_colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
	lv_obj_set_size(top_title_win, LV_PCT(100), TCH_HEADER_HEIGHT);// 60);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, main_colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	tchMtrl_createEscBtn(top_title_win, _tchMtrl_lst_scr_esc_click_cb, 0, 20- TOP_BTN_BORDER, 14- TOP_BTN_BORDER);// 16, 15);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
#endif
	if(g_subjectSel == TCH_MTRL_TEXT_CN)
        lv_label_set_text(lblTitle, "语文教材");
    else if(g_subjectSel == TCH_MTRL_TEXT_ENG)
        lv_label_set_text(lblTitle, "英语教材");
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0, 0);
	//lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, TCH_MTRL_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);


	lv_obj_t* switch_btn_con = lv_obj_create(top_title_win);
	lv_img_dsc_t* switch_btn_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_SWITCH, IMG_FREE_TYPE_AUTO, NULL);
	lv_obj_set_size(switch_btn_con, switch_btn_img->header.w + TOP_BTN_BORDER * 2 + 14, switch_btn_img->header.h + TOP_BTN_BORDER * 2 + 10);
	lv_obj_align(switch_btn_con, LV_ALIGN_RIGHT_MID, -16, 2);
	lv_obj_set_style_border_width(switch_btn_con, 0, 0);
	//lv_obj_set_style_bg_color(switch_btn_con, lv_color_black(), 0);
    lv_obj_t * switch_btn = lv_img_create(switch_btn_con);
	lv_img_set_src(switch_btn, switch_btn_img);
	lv_obj_set_size(switch_btn, switch_btn_img->header.w + TOP_BTN_BORDER * 2, switch_btn_img->header.h + TOP_BTN_BORDER * 2);
	//lv_obj_align(switch_btn, LV_ALIGN_TOP_RIGHT, -16, 15);
	//lv_obj_align(switch_btn, LV_ALIGN_TOP_RIGHT, (TOP_BTN_BORDER-20), 14 - TOP_BTN_BORDER);
	lv_obj_align(switch_btn, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(switch_btn, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(switch_btn, 4, 0);
	lv_obj_add_flag(switch_btn, LV_OBJ_FLAG_CLICKABLE);

	lv_color_t colorBg = lv_color_hex(0xc0c0c0);
	lv_obj_set_style_bg_color(switch_btn, colorBg, 0);
	lv_obj_set_style_bg_opa(switch_btn, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(switch_btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_bg_opa(switch_btn, LV_OPA_10, LV_STATE_PRESSED);

	//lv_obj_set_style_border_color(switch_btn, lv_color_make(0x200, 0x0, 0x00), 0);
	//lv_obj_set_style_border_width(switch_btn, 1, 0);
	
	lv_color_t colorBorderFcs = lv_color_white();
	lv_obj_set_style_border_color(switch_btn, colorBorderFcs, 0);
	lv_obj_add_event_cb(switch_btn, switch_btn_clickCb, LV_EVENT_CLICKED, NULL);

	//ADD TEACH MATRIAL LIST
    lv_obj_t * lst_win = lv_obj_create(mainBg);
    lv_obj_set_size(lst_win, LV_PCT(100), SCREEN_HEIGHT - TCH_HEADER_HEIGHT);
    lv_obj_align_to(lst_win, top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_border_width(lst_win, 0,0);
	lv_obj_set_scrollbar_mode(lst_win, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

	lv_obj_set_flex_flow(lst_win, LV_FLEX_FLOW_ROW_WRAP);			// 允许自动换行
	lv_obj_set_style_pad_top(lst_win, 0, 0);
	lv_obj_set_style_pad_left(lst_win, 15, 0);
	lv_obj_set_style_pad_right(lst_win, 15, 0);
	lv_obj_set_style_pad_row(lst_win, 10, 0);						// 行间距固定 20px
	lv_obj_set_style_pad_column(lst_win, 10, 0);

	//for (int i = 0; i < bookSum; i++)
	bk_node* pNode = g_bkHead;
	int bnum = 0;
	while(pNode)
	{
		lv_obj_t *item = lv_obj_create(lst_win);
		lv_obj_set_size(item, (SCREEN_WIDTH - 20) / 2 - 10, 225);			// 宽度自适应容器，高度固定160px
		lv_obj_set_style_radius(item, BOOK_LIST_RADIUS_SIZE, 0);		// 圆角
		lv_obj_set_style_border_width(item, 0, 0);
		lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_set_scroll_dir(item, LV_DIR_NONE);
		lv_obj_set_style_bg_color(item, lv_color_hex(0xd3d3d3), LV_STATE_PRESSED);
		//lv_obj_set_user_data(item, pNode->data.book_url);
		//lv_obj_set_style_border_width(item, 2, LV_STATE_PRESSED);

		lv_obj_t *img = lv_img_create(item);
		lv_obj_set_style_radius(img, BOOK_LIST_RADIUS_SIZE, LV_PART_MAIN);						// 圆角
		lv_obj_set_style_clip_corner(img, BOOK_LIST_RADIUS_SIZE, LV_PART_MAIN);
		//lv_img_dsc_t img_Src;
		//img_Src = zb_img_res_load_one(NULL, Item_img[i], IMG_FREE_TYPE_AUTO, NULL);
		
		char buf[400] = { 0 };
		sprintf(buf, "/%s/_data/textbook/%s.bin",BOOK_PATH, pNode->data.cover_url);
		printf("buf: %s",buf);
		lv_img_dsc_t *img_Src = zb_load_image_from_file(buf);
		lv_img_set_src(img,img_Src);
		pNode->data.pimg_dsc = img_Src;
		//lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 10);
		lv_obj_set_style_img_opa(img, LV_OPA_70, LV_STATE_PRESSED);

		char name_buf[500] = { 0 };
		lv_coord_t max_width = 140;  // 按钮宽度
		lv_coord_t max_height = 80;   // 两行高度（45按钮高度 + 35间距）
		lv_point_t text_size;
		char* bookName = pNode->data.name;// _chg_ln_book_name(pNode->data.name, name_buf);//get_text_book_name(textName);
		
		lv_obj_t *label = lv_label_create(item);
#ifndef C_PC_SIM
		lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
#endif
		lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);  // 文本水平居中
		lv_obj_set_size(label, 140, 45);
		size_t max_chars = 15;
       size_t char_count = 0;
        const char *p = bookName;
        while(*p && char_count < max_chars) {
            if((*p & 0xC0) != 0x80) char_count++;
            p++;
        }

        if(char_count >= max_chars) {
            size_t pos = 0;
            size_t count = 0;
            const char *ptr = bookName;
            
            while(*ptr && count < max_chars-3) {
                if((*ptr & 0xC0) != 0x80) count++;
                pos++;
                ptr = bookName + pos;
            }
            
            strncpy(name_buf, bookName, pos);
            strcat(name_buf, "...");
			bookName = name_buf;
        }
		lv_label_set_text(label, bookName);
		printf("name_buf = %s\n bookName = %s\n", name_buf,bookName);
		lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); //LV_LABEL_LONG_WRAP

		lv_obj_align_to(label,img, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

		sprintf(buf, "/%s/_data/textbook/books/%s/book000/album.mnu", BOOK_PATH, pNode->data.book_url);
		printf("judge file exist: %s\n", buf);
		if (!zb_isFileExist(buf))
		{
			printf("judge file not exist: \n");
			lv_obj_t* down_img = lv_img_create(img);
			lv_img_dsc_t* img_dsc = zb_img_res_load_one(NULL, PLAYER_IMG_BOOK_RDYPLAY/*PLAYER_IMG_DOWNLOAD*/, IMG_FREE_TYPE_AUTO, NULL);
			lv_img_set_src(down_img, img_dsc);
			//lv_obj_set_style_opa(down_img, LV_OPA_60, 0);
			//lv_obj_set_style_img_recolor_opa(img, LV_OPA_60, 0);
			lv_obj_center(down_img);
			lv_obj_align_to(down_img, img, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
			lv_obj_set_size(down_img, img_dsc->header.w, img_dsc->header.h);
			//lv_obj_add_event_cb(item, _item_click_handler, LV_EVENT_CLICKED, "");
		}
		//else {
			lv_obj_add_event_cb(item, _item_click_handler, LV_EVENT_CLICKED, pNode->data.book_url);
		//}
		pNode = pNode->next;
		bnum++;
	}

	printf("tch_mtrl_win_init actual = %d\n", bnum);
	sys_setCurMainWinAndCb(mainBg, _tch_mtrl_scr_cb);
	g_tchMnuBgWin = mainBg;
}

int tch_mtrl_ap_win_deinit(void) { //界面销毁
	if (g_tchMnuBgWin) {
		lv_obj_del(g_tchMnuBgWin);  g_tchMnuBgWin = NULL;
	}
	return 0;
}

static void _tch_mtrl_curScr_del(void) {
	if (g_tchCurWin == TCH_TCH_MTRL) {
		tch_mtrl_ap_win_deinit();
		return 0;
	}
	switch (g_ItemSel + 1) 
	{
		default:
	
		break;
	}
}

extern void tch_mtrl_back_content(void);

int tch_mtrl_ap_handler(lv_event_code_t msg, int param) {
    LV_LOG("%s,[%d]\r\n ", __FUNCTION__,__LINE__);
	switch ((int)msg) 
	{
	case C_EVENT_FREE_AP_RES:
		LV_LOG("%s,[%d] , g_ItemSel: %d \r\n ", __FUNCTION__,__LINE__, g_ItemSel);
		_tch_mtrl_curScr_del();
		break;

	case C_EVENT_EX_CMD: {
		if (param == PLY_CMD_PLY_LST_RET) {
			switch (g_tchCurWin)
			{
				//case TCH_TCH_MTRL:
				//	break;
				//case TCH_CFG_SETTING:
				//	break;
			case TCH_CONTENT:
				LV_LOG("%s,[%d] , g_ItemSel: %d \r\n ", __FUNCTION__, __LINE__, g_ItemSel);
				tch_mtrl_back_content();
				break;
			default:
				break;
			}
			// ESP_LOGI(TAG, "EVENT_EX_CMD A, par:%d", param);
			// switch (param) {
			// 	case CMD_PAR_ENTER_SUB: {
			// 		if (g_setMnuBgWin) {
			// 			lv_obj_del(g_setMnuBgWin);   g_setMnuBgWin = NULL;
			// 		}
			// 		_setng_choice_proc();
			// 		break;
			// 	}
			// 	case CMD_PAR_RET_STNG_MENU: {
			// 		_stng_curScr_del();
			// 		setng_menu_scr_init(0);
			// 	}
			// 	case NET_CONN_CMD_CODE: {
			// 		if (NETWORK_SETTING == g_setngSel + 1) {
			// 			ESP_LOGI(TAG, "req exit NetCnn");
			// 			//zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			// 		}
			// 		break;
			// 	}
			// 	case MY_DEV_SETTING: {  // todo
			// 		seting_hanlder_sub_my_device(param);
			// 		break;
			// 	}
			// 	default:
			// 		break;
			// }
		}
		break;
	}
	case C_EVENT_DOES_BLOCK_ALARM:
		//return (UPDATE_SETTING == g_setngSel);

	case C_EVENT_DOES_BLOCK_SYS_POWER_KEY:
		// if (2 == param) { // 2 means longPress evt (power key)
		// 	if (UPDATE_SETTING == g_setngSel) {
		// 		return 1; // 数据更新时 要屏蔽关机键 
		// 	}
		// }
		return 0;

	default:
		break;
	}
	return 0;
}

int tch_mtrl_eng_ap_init(void){
    g_subjectSel = TCH_MTRL_TEXT_ENG;
    tch_mtrl_win_init(FUN_FORWARD);
	return 0;
}
int tch_mtrl_eng_ap_handler(lv_event_code_t msg, int param){
    tch_mtrl_ap_handler(msg, param);
	return 0;
}
int tch_mtrl_cn_ap_init(void){
    g_subjectSel = TCH_MTRL_TEXT_CN;
    tch_mtrl_win_init(FUN_FORWARD);
	return 0;
}
int tch_mtrl_cn_ap_handler(lv_event_code_t msg, int param){
    tch_mtrl_ap_handler(msg, param);
	return 0;
}

void _tch_mtrl_free() {
	bk_node* pNode = g_bkHead;
	
	if (pNode) {
		while (pNode != NULL) {
			bk_node* pNodeNx = pNode->next;
			//zb_img_res_free_one();

			lv_img_dsc_t* img_Src = pNode->data.pimg_dsc;
			heap_caps_free(img_Src->data);
			img_Src->data = NULL;
			heap_caps_free(img_Src);

			heap_caps_free(pNode);
			pNode = pNodeNx;
		}
		g_bkHead = NULL;
		//book_node_free();
	}
}

void _tch_mtrl_scr_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);
	int keyVal = 0;
	lv_event_code_t evtCode = evt->code;

	if (LV_EVENT_KEY_STAGE == evt->code) {
		keyVal = lv_event_get_key(evt);
		if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			evtCode = LV_EVENT_KEY;
		}
	}

	switch (evtCode) {
	case LV_EVENT_KEY_STAGE:
		if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)
			break;
		keyVal &= 0xff;
		switch (keyVal) {
		case LV_KEY_ESC:
			switch (g_tchCurWin)
			{
			case TCH_TCH_MTRL:
				_tch_mtrl_free();
				zb_requestSwitchAp(C_ZB_AP_MAINMENU);
				break;
			case TCH_CFG_SETTING:
				tch_mtrl_cfg_setting_del(FUN_CANCEL, "");
				break;
			case TCH_CONTENT:
				tch_mtrl_content_del();
				break;
			case TCH_DOWNLOAD:
				tch_mtrl_download_del();
				break;
			default:
				break;
			}
			break;
		//case LV_KEY_HOME:
		//	lv_indev_keypad_wait_release();
		//	break;
		case LV_KEY_ENTER:

			break;
		default:
			break;
		}
		break;
	case LV_EVENT_DELETE:
		break;
	default:
		break;
	}
}