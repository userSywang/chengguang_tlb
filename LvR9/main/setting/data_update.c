//========================================================[]
//Name: data_update.c
//Function: update program and data.
//Author: Xie Zhenzong 
//Date: 2024-06-12
//========================================================[]
/********************** 
     INCLUDES
 *********************/
#include <string.h>
#include <math.h>

#include <lvgl/lvgl.h>
 
#include "../utils/global_parm.h"
#include "../utils/misc_utils.h"
#include "app_update.h"
#include "../ex_widget/app_msg.h"


#ifndef C_PC_SIM

#endif
//#include "../src/http/http_down_sd_file.h"

#include "../data_parse/img_res.h"


/*********************
 *      DEFINES
 *********************/
#define DISK_FILE_PATH_LEN 512

#define DAT_INF_HEAD_LN_BUF_LEN 240


const char* SDISK_ROOT="/userdata/";
const char* DATA_FILE_ROOT_URL = "http://video2.bolledu.net/BOLLTVVideo/upload/data/tlb_xsz/download/";

#define DATA_INFO_FILE_URL		"http://video2.bolledu.net/BOLLTVVideo/upload/data/tlb_xsz/download/_datainf"

const char * RES_IMG_PATH="/userdata/sysData/img/img_a_tlb.bin";


typedef void (* dl_pc_func_t) ( void * );  //下载进度显示处理

typedef struct _dl_process_param {
		uint32_t file_size; 	//大小
		volatile uint32_t cur_len; 	//当前下载长度
} dl_pc_param;


typedef struct download_cfg {
	char *download_url ; // 
	char *download_file_path ;//
	
	dl_pc_func_t f_process_cb;
//	dl_pc_param * param;
	bool bUseTmpPath;
} download_cfg_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/


typedef struct _data_file_info_t {
	int32_t f_CRC32;		//文件的CRC32
	int32_t len_remainder;	//长度除4096的余数
	char * file_path;  		//路径
}data_file_info_t;


extern data_update_file_cfg_t * data_cfg;
extern data_update_progress_state dp_st; 

/**********************
 *  STATIC VARIABLES
 **********************/

static download_cfg_t hpd;  //test
static data_file_info_t f_info;

static volatile bool data_update_task_run_flag = false;
static volatile bool data_update_finish_flag = false; // 是否完成(这里完成包括失败)
static volatile int8_t data_upd_stage = 0;  // 1 init, 2 wait confirm to batch chkCrc; 3 means doing loop check file's crc, DAT_UPD_STAT_INIT ...

static volatile int8_t data_update_ret = 0;     // 1- updateDataDone, 0- fail
static volatile bool data_update_task_enable = false;
static volatile int  size_state = 0;

static int16_t g_dataVerLocal = 0;
static int16_t g_dataVerRmt = 0;

/**********************
 *      MACROS
 **********************/
#define		FILE_WR_ERR_STR		"文件写入失败！"
#define		FILE_DW_ERR_STR		"文件下载失败！"

static const char *TAG = "DATA_INFO";

#define STR(s) #s
#define DATE_TIME __DATE__ " " __TIME__
//====================================================



//====================================================[]
//   extra  FUNCTIONS 
//====================================================[]

//--------------------------------------
bool get_esp32s3_wifi_connected_state(void);
uint32_t  esp_rom_crc32_le(uint32_t crc, uint8_t const * buf, uint32_t len);
int http_clond_download(download_cfg_t*config);
int http_stop_download_task(void);
int http_download_length_state(void);

void data_dl_new_file_progress_refresh(int cur_file, int file_num, char* fpath );
void zb_img_res_close(zb_img_file_res_t * img_res);


//====================================================[]
//   fuctions declare
//====================================================[]


//====================================================[]
//   STATIC  FUNCTIONS
//====================================================[]
//进度条回调
static dl_pc_func_t data_dl_file_progress_cb(void * param)
{
	dl_pc_param * dl_f = (dl_pc_param *)param;
	dp_st.cur_file_size = dl_f->file_size;
	dp_st.cur_file_dl_len = dl_f->cur_len;
	return 0;
}
//------------------------------------------------------------------
// 下载新文件时更新进度变量
static int data_dl_new_file_progress_set(int cur_file, int file_num, char* fpath )
{
	dp_st.cur_file_no = cur_file;
	dp_st.total_num = file_num;
	dp_st.cur_fpath = fpath;
//	dp_st.cur_file_size = 100;  //下载时才能获得
	dp_st.cur_file_dl_len = 0;
	return 0;
}

//-------------------------------------------------------------
char * urlEncode(const char* string) 
{
    const char* hexChars = "0123456789ABCDEF";
    int length = strlen(string);
    char* encodedString = heap_caps_malloc_dflt(3 * length + 1); // Allocate memory for the encoded string
    int index = 0;

    for (size_t i = 0; i < length; i++) 
	{
        char currentChar = string[i];

        if (isalnum(currentChar) || currentChar == '-' || currentChar == '_' || currentChar == '.'|| currentChar == '/' || currentChar == ':')
		{
            encodedString[index++] = currentChar; // Append alphanumeric and safe characters as is
        } 
		else if (currentChar == ' ') 
		{
            encodedString[index++] = '+'; // Convert space to '+'
        } 
		else 
		{
            encodedString[index++] = '%'; // Convert other characters to percent-encoding
            encodedString[index++] = hexChars[(currentChar >> 4) & 0xF]; // Append first hexadecimal digit
            encodedString[index++] = hexChars[currentChar & 0xF]; // Append second hexadecimal digit
        }
    }

    encodedString[index] = '\0'; // Add null terminator
    return encodedString;
}

//-------------------------------------------------------------
static void tlb_print_hex(unsigned char * in, unsigned int length, char title[])
{
	int i	= 0;
	int index = 0;

	// LISA_LOGI(TAG, "[index]:%d\n", g_index);
	for (int i = 0; i < (int) (length + 1) / 16 + 1; i++)
	{
		if (i == 0)
		{
			if (title == NULL)
				printf("[--------] \n");
			else 
				printf("[---%s---] \n", title);
		}
		index = (length - i * 16) < 16 ? (length - i * 16) : 16;

		for (int j = 0; j < index; j++)
		{
			printf("0x%02x ", in[i * 16 + j]);
		}

			printf("\n");
	}
	return;
}

//==============================================================================[]
//return: 0 -- wrong, 1 -- right;
int check_local_file_info(char* pathname, data_file_info_t* d_info)
{
	ESP_LOGI(TAG, "%s [%d], [%s]\n", __FUNCTION__,__LINE__, pathname);
	int32_t size = 0;
	int32_t count = 0;
	int32_t remainder = 0;
	int32_t f_crc = 0;
	char* data_buf = NULL;
	int refresh_data = 0;
	int ret = 0;
	FILE * fp = zb_fopen(pathname, "rb");
	if(fp == NULL)
	{
		LV_LOG("%s,[%d] file open*** error! *** \r\n ", __FUNCTION__,__LINE__);

		return(ret) ;
	}
//	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
#if 1
	size = zb_getFileLength(pathname);
#else
	fseek(fp, 0 ,SEEK_END);
	
	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
	size = ftell(fp);
#endif
//	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
	remainder = size & 0x0FFF;
	
	LV_LOG("%s,[%d] file size: %lx, remainder %lx \r\n ", __FUNCTION__,__LINE__, size, remainder);
	
	if(remainder != d_info->len_remainder)
	{
		LV_LOG("%s,[%d] *** file remainder error! *** \r\n ", __FUNCTION__,__LINE__);
		goto check_exit;
	}
	
	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
#if 0  // ndef C_PC_SIM
	data_buf = heap_caps_malloc(DATAINFO_CRC_BUF, (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
#else
	data_buf = heap_caps_malloc_dflt(DATAINFO_CRC_BUF);
#endif
	if(data_buf == NULL)
	{
		LV_LOG("%s,[%d] heap_caps_malloc_dflt *** error! *** \r\n ", __FUNCTION__,__LINE__);
		goto	check_exit;
	}

//	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
	
//xzz	fseek(fp, 0 ,SEEK_SET);
	
	dp_st.cur_file_size = size;
	dp_st.cur_file_dl_len = 0;
	count = DATAINFO_CRC_BUF;
//	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
	while(size)
	{
		if(size >= DATAINFO_CRC_BUF)
		{
			size -= DATAINFO_CRC_BUF;
		}
		else
		{
			count = size;
			size = 0;
		}
		size_state = size;
		zb_fread(data_buf, count, 1, fp);
		f_crc = esp_rom_crc32_le(f_crc, (uint8_t const *)data_buf, count);
		refresh_data += count;
		dp_st.cur_file_dl_len = refresh_data;
		if((refresh_data % (1024*1024)) == 0)  // 1M 打印一次
		{
			ESP_LOGI(TAG, "%s [%d],size:%ld, count:%ld cur: crc:[%lx]\n", __FUNCTION__,__LINE__, size,count, f_crc);
		}
	}
	
	if(f_crc == d_info->f_CRC32)
	{
		ESP_LOGI(TAG, "%s [%d],crc:[%lx] right. \n", __FUNCTION__,__LINE__, f_crc);
		ret = 1;
	}
	else
	{
		ESP_LOGI(TAG, "%s [%d],crc:[%lx] wrong.\n", __FUNCTION__,__LINE__, f_crc);
		ret = 0;
	}
check_exit:	

	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
	
	zb_fclose(fp);
	
	if(data_buf != NULL)
	{
		heap_caps_free(data_buf);
		data_buf = NULL;
	}
	return(ret) ;
}


static int16_t getInt16ParamFromDataInfHeadLn(const char * pcLine, const char * pcKey)
{
	if (!pcLine)
		return 0;
	if (!pcKey || !pcKey[0])
		return 0;
	int16_t retVal = 0;
	char * pcSaved = NULL;
	char * pcOnce = strtok_r(pcLine, ";", &pcSaved);
	for (int16_t itemNo = 0; itemNo < 25; itemNo++) {
		if (!pcOnce)
			break;
		char * pcMid = strchr(pcOnce, '=');
		if (NULL == pcMid) {
			// skip the invalid parm item.
		}
		else {
			char acKey[20];
			int len = pcMid - pcOnce;
			acKey[0] = '\0';
			if (len > sizeof(acKey) - 1) {
				len = sizeof(acKey) - 1;
			}
			strncat(acKey, pcOnce, len);
			if (strcmp(acKey, pcKey) == 0) {
				int intVal = 0;
				sscanf(pcMid + 1, "%d", &intVal);
				retVal = (int16_t)intVal;
				break;
			}
		}
		pcOnce = strtok_r(NULL, ";", &pcSaved);
	}
	return retVal;
}

//==============================================
//Open data info file
//==============================================
static int16_t read_datainfo_version(void)
{
	FILE * fp = NULL;
	int16_t dataVer = 0;

	do {
		fp = zb_fopen(DATA_INFO_FILE_PATH, "rb");
		if (NULL == fp) {
			ESP_LOGW(TAG, "file open*** error! %s", DATA_INFO_FILE_PATH);
			break;
		}
		char line_buf[DAT_INF_HEAD_LN_BUF_LEN];
		line_buf[0] = '\0';
		fgets(line_buf, sizeof(line_buf), fp); // 读取首行 
		dataVer = getInt16ParamFromDataInfHeadLn(line_buf + 2, "dataVer");
	} while (0);

	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}
	return dataVer;
}

//======================================================

/**
 * @input  : file [DATA_INFO_FILE_MID_PATH]
 * @output : g_dataVerRmt 
 * @return : num of file in list, > 0 means success;
 */
static int get_datainfo_cfg_file_info(void)
{
	char temp_buf[9] = {0};
	char * line_buf = NULL;
	int str_len = 0;
	int	file_num = 0;
	int iFuncRet = -1;
	FILE * fp = NULL;

	do {
		line_buf = heap_caps_malloc_dflt(DAT_INF_HEAD_LN_BUF_LEN);
		if (NULL == line_buf) {
			LV_LOG("%s,[%d] malloc *** error! *** \r\n ", __FUNCTION__,__LINE__);
			iFuncRet = -3;  break;
		}
		char * p_start = line_buf;
		char * p_end = line_buf;
		fp = zb_fopen(DATA_INFO_FILE_MID_PATH, "rb");
		if (NULL == fp) {
			LV_LOG("%s,[%d] *** data info file not opened! *** \r\n ", __FUNCTION__,__LINE__);
			iFuncRet = -5;  break;
		}
		line_buf[0] = '\0';
		int ret = fgets(line_buf, DAT_INF_HEAD_LN_BUF_LEN, fp); // 读取首行 
		if(ret == NULL) 
		{
			LV_LOG("%s,[%d] *** data info file error! *** \r\n ", __FUNCTION__,__LINE__);
			iFuncRet = -7;  break;
		}
		memcpy(temp_buf, line_buf, 8);
		if(((p_end - line_buf) >12) || (strcmp("::total=",temp_buf ) != 0)) //找不到结束符
		{
			LV_LOG("%s,[%d] datainfo file *** error! *** \r\n ", __FUNCTION__,__LINE__);
			iFuncRet = -9;  break;
		}

		file_num = getInt16ParamFromDataInfHeadLn(line_buf + 2, "total");
		fseek(fp, 0, 0);
		line_buf[0] = '\0';
		fgets(line_buf, DAT_INF_HEAD_LN_BUF_LEN, fp); //再读首行 
		int16_t dataVer = getInt16ParamFromDataInfHeadLn(line_buf, "dataVer");
		g_dataVerRmt = dataVer;
		
		iFuncRet = file_num;
		ESP_LOGI(TAG, "flNum:%d", file_num);
	}while(0);

	if (line_buf != NULL) {
		heap_caps_free(line_buf);  line_buf = NULL;
	}
	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}
	return iFuncRet;
}


/*
 * @return : > 0 means version match;  others means need updateDat.
 */
int cmpDataVersionLocalAndRmt(void) {
	int ret = get_datainfo_cfg_file_info();
	if (ret < 0) {
		ESP_LOGI(TAG, "getRmt dataVer faild!");
		return -1;
	}
	g_dataVerLocal = read_datainfo_version();
	ESP_LOGI(TAG, "cmp datVer %hd, rmt %hd", g_dataVerLocal, g_dataVerRmt);
	if ((g_dataVerLocal > 0) && (g_dataVerLocal == g_dataVerRmt)) {
		return g_dataVerLocal;
	}
	return -3;
}

//-------------------------------------------
//获取单个文件的信息

/*
 * @return < 0 means fail.
 */
int  get_file_info(data_file_info_t* finfo_out)
{
	data_file_info_t* finfo = finfo_out;
	char * line_buf = NULL;
	int str_len = 0;
	char int_str[9]={0};
	int iFuncRet = 0;

	do {
		if (data_cfg->fp == NULL) {
			LV_LOG("%s,[%d] *** data info file file not opened! *** \r\n ", __FUNCTION__,__LINE__);
			break;;
		}
		line_buf = heap_caps_malloc_dflt(DATAINFO_LINE_MAX_BYTE);
		if (line_buf == NULL) {
			LV_LOG("%s,[%d] *** malloc error! *** \r\n ", __FUNCTION__,__LINE__);
			break;
		}
		char * p_start = line_buf;
		char * p_end = line_buf;
		line_buf[0] = '\0';
		int ret = fgets(line_buf, DATAINFO_LINE_MAX_BYTE, data_cfg->fp); // 获取一行。
		if (ret == NULL) 
		{
			LV_LOG("%s,[%d] *** get data info error! *** \r\n ", __FUNCTION__,__LINE__);
			break;
		}
		p_end = strchr(line_buf,';');
		str_len = p_end - p_start;
		if(str_len > 8)
		{
			LV_LOG("%s,[%d] *** CRC len error! *** \r\n ", __FUNCTION__,__LINE__);
		}	
		memcpy(int_str, p_start, str_len);

		sscanf(int_str,"%lx", &finfo->f_CRC32 );
		
		p_start += 9;	
		memcpy(int_str, p_start, 3);
		int_str[3] = 0;	
		sscanf(int_str,"%lx", &finfo->len_remainder);

		p_start += 4;
		p_end = strchr(p_start, '\n');
		str_len = p_end - p_start;
		
		if((str_len >=1)&&(*(p_end-1) == '\r'))
		{
			str_len -= 1;
		}	
		memset(finfo->file_path, 0, FILE_URL_MAX_LEN);
		
		memcpy(finfo->file_path , p_start, str_len);
		iFuncRet = 1;
	} while (0);

	if (line_buf != NULL) {
		heap_caps_free(line_buf);  line_buf = NULL;
	}
	return iFuncRet;
}

//-------------------------------------------------------------------
//分析配置文件，逐个计算本地各文件CRC,如果与配置文件不符，则下载文件。
//-------------------------------------------------------------------

static int _data_update_download_exe(void)
{
	int ret = 0;
	int retry = 0;
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__,__LINE__);

	ret = get_datainfo_cfg_file_info();
	if (ret < 0) {
		LV_LOG("%s,[%d] *** faild! *** \r\n ", __FUNCTION__,__LINE__);
		return 0;
	}
	data_cfg->num = ret;
	
	f_info.file_path = heap_caps_malloc_dflt(DISK_FILE_PATH_LEN);
	char * file_url = heap_caps_malloc_dflt(FILE_URL_MAX_LEN);
	char * pathname = heap_caps_malloc_dflt(DISK_FILE_PATH_LEN);
	
	if((f_info.file_path == NULL) || (file_url == NULL) || (pathname == NULL))
	{
		LV_LOG("%s,[%d] *** malloc error! *** \r\n ", __FUNCTION__,__LINE__);
		return 0;
	}

	data_cfg->fp = zb_fopen(DATA_INFO_FILE_MID_PATH, "rb");
	if (NULL == data_cfg->fp) {
		ESP_LOGE(TAG, "fail to open %s", DATA_INFO_FILE_MID_PATH);
		data_cfg->num = 0;
	}
	char acLine[DAT_INF_HEAD_LN_BUF_LEN];
	fgets(acLine, sizeof(acLine), data_cfg->fp); // 跳过首行 
	ESP_LOGI(TAG, "waitConfirm");
	int i = 0;
#if 0
	data_upd_stage = DAT_UPD_STAT_WAIT_CONFIRM;
	while (DAT_UPD_STAT_WAIT_CONFIRM == data_upd_stage) {
		vTaskDelay(pdMS_TO_TICKS(250));
		if (!data_update_task_enable) {
			break;
		}
	}
#endif
	data_upd_stage = DAT_UPD_STAT_BATCH_CHECK;
	ESP_LOGI(TAG, "bfrLpRd file_num: %ld", data_cfg->num);
	
	for(i = 0; ((i < data_cfg->num) && (data_update_task_enable == true)); i++)
	{
		dp_st.cur_state = 0;
		ret = get_file_info(&f_info);
		if(ret == 0)	
		{
			LV_LOG("%s,[%d] *** get_file_info: %d error *** \r\n ", __FUNCTION__,__LINE__, i);
			break;
		}
		
		LV_LOG("##第[%d]个##,f_info: %lx, %lx :%s\r\n ",(i+1), f_info.f_CRC32,f_info.len_remainder,f_info.file_path);
		if(strstr(".gitignore", f_info.file_path))
		{
			continue;  //忽略此文件
		}
		// 准备处理文件
		data_dl_new_file_progress_set(i+1, data_cfg->num, f_info.file_path);
		
		memset(pathname, 0, DISK_FILE_PATH_LEN);
		snprintf(pathname, DISK_FILE_PATH_LEN, "%s%s", SDISK_ROOT, f_info.file_path);
		
		dp_st.cur_state = 1;  //checksum state.
		LV_LOG("%s,[%d] pathname:%s \r\n ", __FUNCTION__,__LINE__, pathname);
		ret = check_local_file_info(pathname, &f_info);
		LV_LOG("%s,[%d] ret: %d \r\n ", __FUNCTION__,__LINE__, ret);
		if((ret == 0) && (data_update_task_enable == true)) //本地文件不正确，需要下载
		{
			LV_LOG("%s,[%d] local_file_info error, update ... \r\n ", __FUNCTION__,__LINE__);
			dp_st.cur_state = 2;

			download_cfg_t cfg;
			memset(file_url, 0, FILE_URL_MAX_LEN);

			char* url_enc = urlEncode(f_info.file_path);
			
			snprintf(file_url, FILE_URL_MAX_LEN, "%s%s", DATA_FILE_ROOT_URL, url_enc);
			if (url_enc != NULL) {
				heap_caps_free(url_enc);  url_enc = NULL;
			}
			cfg.download_url = file_url;			
			cfg.download_file_path = pathname;
			LV_LOG("%s,[%d] file_path:%s \r\n ", __FUNCTION__,__LINE__, f_info.file_path);
			LV_LOG("%s,[%d] pathname:%s \r\n ", __FUNCTION__,__LINE__, pathname);
			LV_LOG("%s,[%d] file_url:%s \r\n ", __FUNCTION__,__LINE__, file_url);
			
			cfg.f_process_cb = data_dl_file_progress_cb;
#ifndef C_PC_SIM
			retry = 0;
			while((retry < 3) && (ret == 0)) //网络异常退出会重试两次
			{
				ret = http_clond_download(&cfg);
				retry ++;
			}
			if(ret == 0) 
			{
				LV_LOG("%s,[%d] http_clond_download error.\r\n ", __FUNCTION__,__LINE__);
				break;
			}
#endif			
		}
	}
	dp_st.cur_state = 0;

	if ((data_cfg->num > 0) && (i == data_cfg->num)) 
	{
		data_update_ret = 1;
	}
	else
	{
		data_update_ret = 0;
	}
	ESP_LOGI(TAG, "datUpdLp end, data_update_ret: %d", (int)data_update_ret);
	
	if(data_cfg->fp != NULL) {
		zb_fclose(data_cfg->fp);  data_cfg->fp = NULL;
	}
	if(pathname != NULL) {
		heap_caps_free(pathname);  pathname = NULL;
	}
	if(file_url != NULL) {
		heap_caps_free(file_url);  file_url = NULL;
	}

	if(f_info.file_path != NULL) {
		heap_caps_free(f_info.file_path);  f_info.file_path = NULL;
	}
	if (data_update_ret) {
		zb_remove(DATA_INFO_FILE_PATH);
		zb_rename(DATA_INFO_FILE_MID_PATH, DATA_INFO_FILE_PATH);
	}
	return(1);
}

//------------------------------------------[]
//重命名下载的配置文件
// return : 1 -- 成功, 0--失败；
int donwlaod_datainfo_file_rename(void)
{
	int ret = 0;
	ret = zb_remove(DATA_INFO_FILE_MID_PATH);
//	if(ret != 0) //文件一开始可能不存在
//	{
//		return(0);
//	}
#ifndef C_PC_SIM
	ret = rename(DATA_INFO_FILE_TEMP_PATH, DATA_INFO_FILE_MID_PATH);
#endif	

	if(ret != 0) 
	{
		return(0);
	}
	return(1);
}
//=======================================================
//下载数据配置文件
//=======================================================
int download_datainfo_file(void)
{
	if (datUpd_doesSkipDatVer()) {
		g_dataVerLocal = 0;
		ESP_LOGI(TAG, "skipDatVer");
	}
	else {
		g_dataVerLocal = read_datainfo_version();
	}
	ESP_LOGI(TAG, "DatVerLcl %hd", g_dataVerLocal);
	//------------------
#ifndef C_PC_SIM
	hpd.download_url = DATA_INFO_FILE_URL;
	hpd.download_file_path = DATA_INFO_FILE_TEMP_PATH;
	
	//todo, http_download_sd_file(&hpd);
#endif	
	
//	timer = lv_timer_create(download_timer_handler, 1000, &count);	
	return(1);

}
//-------------------------------------------------
//下载配置文件状态
bool get_datainfo_dl_finised_state(void)
{
#ifndef C_PC_SIM
	return http_download_finised_state();
#else
	return(1);
#endif
}

bool get_datainfo_dl_ret(void)
{
#ifndef C_PC_SIM
	return	http_download_ret();
#else
		return(1);
#endif
}
//-----------------------------------------------
//下载所有文件状态
bool get_data_update_dl_finised_state(void)
{
#ifndef C_PC_SIM
	return(data_update_finish_flag);
#else
	return(1);
#endif
}

int8_t get_data_update_dl_ret(void)
{
#ifndef C_PC_SIM
	return(data_update_ret);
#else
	return(1);
#endif
}

int8_t get_data_update_stage(void) {
	return data_upd_stage;
}

void data_update_set_confirm(bool doesContinue) {
#if 0
	if (DAT_UPD_STAT_WAIT_CONFIRM == data_upd_stage) {
		if (doesContinue) {
			data_upd_stage = DAT_UPD_STAT_BATCH_CHECK;
			ESP_LOGI(TAG, "datUpd has cnfrm");
		}
		else {
			data_update_task_enable = false;
			ESP_LOGI(TAG, "datUpd has be cancel");
		}
	}
#endif
}

//--------------------------------------------
//停止下载
void stop_data_update_dl(void)
{
	data_update_task_enable = false;
#ifndef C_PC_SIM
	//todo, http_stop_download_task();
#endif
}

//--------------------------------------------------
//返回当前下载数据长度，可多次读取判断是否网络异常
int data_update_dl_length_state(void)
{
#ifndef C_PC_SIM
	//return(http_download_length_state() + size_state);
	return 1;
#else
	return(1);
#endif
}

//===========================================================

static void data_update_download_task(void *para)
{
#if 0 // #ifndef C_PC_SIM
	data_update_finish_flag = false;
	data_upd_stage = DAT_UPD_STAT_INIT ;
	data_update_ret = 0;
	data_update_task_enable = true;
	data_update_task_run_flag = true;
	LV_LOG("%s,[%d] start.\r\n ", __FUNCTION__,__LINE__);
	
	zb_img_res_close(-1);	// 关掉图片资源文件
	
	_data_update_download_exe();
	
	zb_img_file_res_t * pImgRes = zb_img_res_open(RES_IMG_PATH); 
	
	LV_LOG("%s,[%d] finished.\r\n ", __FUNCTION__,__LINE__);
	data_update_finish_flag = true;
	data_upd_stage = 0;
	data_update_task_enable = false;
	data_update_task_run_flag = false;
	vTaskDelete(NULL);
#endif
	
}
//----------------------------------------------------------------------[]
int data_update_download_pro(void) 
{
    if (data_update_task_run_flag) 
	{
	    ESP_LOGW(TAG, "----- data update task running ----\n");
        return 0;
    }
#ifndef C_PC_SIM

    //todo xTaskCreatePinnedToCore(data_update_download_task, "d_task", 4*1024, (void *)0, 5 , NULL, 0);
#endif
    return 0;
}


