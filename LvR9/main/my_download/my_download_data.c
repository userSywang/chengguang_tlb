//#include <stdio.h>
#ifndef C_PC_SIM
#include <dirent.h>   // for opendir() 
#include <unistd.h>
#endif

#include <lvgl/lvgl.h>
#include <lvgl/src/misc/lv_types.h>
#include "../lvgl_main.h"
#include "my_download.h"
#include "../data_parse/img_res.h"
#include "../utils/misc_utils.h"
#include "../data_parse/analyze_listening.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include "../upd_data_parse/favorites.h"
//————
#define TAG "my_download_data.c"

char* previous_level_directory;//前一级目录 Previous level directory
char** second_level_directory;    //二级目录名称
char** second_son_absolute_path;  //二级目录绝对路径
int Number_secondary_file_dir = 0;//二级目录文件的数量 Number of secondary file directories

//----------------------------------------------
int My_download_file_directory_data_processing(char* path, int Dir_depth); //我的下载文件目录数据处理
int Print_file_directory(char* dirname);//遍历打印某一文件夹下的所有文件目录
int Print_directory_tree(char* path, int indent);//打印出指定目录的目录树
int delete_directory(const char* path);//删除指定目录的目录树
int Del_dir_tree_from_specified_directory(char* path);//删除指定文件和目录树


//----------------------------------------------
int MDLD_dld_data_RAM_alloc(void)
{
	if (!second_level_directory) {
		second_level_directory = (char **)heap_caps_malloc_dflt(sizeof(char *) * SECONDARY_DIR_MAX);
		memset(second_level_directory, 0, sizeof(char *) * SECONDARY_DIR_MAX);
	}

	if (!second_son_absolute_path) {
		second_son_absolute_path = (char **)heap_caps_malloc_dflt(sizeof(char *) * SECONDARY_DIR_MAX);
		memset(second_son_absolute_path, 0, sizeof(char *) * SECONDARY_DIR_MAX);
	}

	if (!previous_level_directory) {
		previous_level_directory = (char *)heap_caps_malloc_dflt(PATH_BYTE_SIZE);
	}
	return 0;
}

void MDLD_dld_data_RAM_free(void)
{
	MDLD_doFreeIfNotNull(heap_caps_free, previous_level_directory);

	for (int idx = 0; idx < SECONDARY_DIR_MAX; idx++)
	{
		MDLD_doFreeIfNotNull(heap_caps_free, second_son_absolute_path[idx]);
		MDLD_doFreeIfNotNull(heap_caps_free, second_level_directory[idx]);
	}
	MDLD_doFreeIfNotNull(heap_caps_free, second_son_absolute_path);
	MDLD_doFreeIfNotNull(heap_caps_free, second_level_directory);
	return;
}

//————————————————————————————————
// 我的下载文件目录数据处理
//————————————————————————————————
int My_download_file_directory_data_processing(char* path,int Dir_depth)
{
    LV_LOG_USER("已进入：我的下载文件目录数据处理\n");
    ESP_LOGI(TAG, "传入的路径：%s", path);
    ESP_LOGI(TAG, "传入的目录深度：%d", Dir_depth);

    LV_LOG_USER("开始目录处理\n");
    //1, 打开目录 
    char* sd_card_directory = zb_strdup(path);//SD_CARD_FILE_NAME;
    if (!sd_card_directory) {
        ESP_LOGE(TAG, "内存分配失败！");
        Number_secondary_file_dir = 0;
        heap_caps_free(sd_card_directory); // 释放分配的内存
        sd_card_directory = NULL;
        return -1;
    }

    lv_fs_dir_t stDir = { 0 };
    lv_fs_res_t dirRet = lv_fs_dir_open(&stDir, sd_card_directory);
    if (dirRet != 0)
    {
        ESP_LOGE(TAG, "打开目录失败！%d", dirRet);
        Number_secondary_file_dir = 0;
        heap_caps_free(sd_card_directory); // 释放分配的内存
        sd_card_directory = NULL;
        return -1;
    }

    //ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
    //2, 循环读取目录下的文件信息 --> 直到读取完毕 
    int i = 0;
    char d_name[MDLD_FILE_NAME_BUF_LEN];
    d_name[0] = '\0';
    while ((dirRet = lv_fs_dir_read(&stDir, d_name)) == 0)	//循环读取目录中的信息,如果返回值为NULL,结束循环 
    {
        char filepath[PATH_BYTE_SIZE];		//存放文件的完整路径 
        if (strcmp(".", d_name) == 0 || strcmp("..", d_name) == 0)
        {
            continue;
        }
        int nameSkipN = ('/' == d_name[0] ? 1 : 0);
        char * fileName = d_name + nameSkipN;
        //目录深度为1时跳过特定文件夹
        if (Dir_depth == 1) {

            if ((strcmp(fileName, "autorun.inf") == 0) || 
                (strcmp(fileName, "autorun. inf") == 0)||
                (strcmp(fileName, "Autorun.inf") == 0) ||
                (strcmp(fileName, "Autorun. inf") == 0)) 
            {
                ESP_LOGW(TAG, "autorun.inf文件找到了");
                continue;
            }

            if (strcmp(fileName, "sysData") == 0 ||
                strcmp(fileName, "_data") == 0 ||
                strcmp(fileName, "autorun.inf") == 0 ||
                strcmp(fileName, "System Volume Information") == 0)
            {
                ESP_LOGI(TAG, "目录深度为1时跳过特定文件夹");
                continue;
            }
        }
        snprintf(filepath,sizeof(filepath), "%s/%s", sd_card_directory, fileName);
        //ESP_LOGI(TAG, "跳过特定文件夹后filname[%d]:%s\n", i, filepath);
        if (strlen(filepath) >= PATH_BYTE_SIZE) {
            ESP_LOGE(TAG, "路径长度为:%d 超过最大值:%d",strlen(filepath), PATH_BYTE_SIZE);
            continue;
        }

        // 获取文件属性
        int st_mode = 0;
        int fsRet = zb_stat__st_mode(filepath, &st_mode);
        if (fsRet < 0) {
            ESP_LOGE(TAG, "获取文件属性stat失败 %s", filepath);
            continue;
        }
        // 如果是目录或者文件，并且后缀为.mp3或.wav
        if ( (S_ISDIR(st_mode)) || (S_ISREG(st_mode) &&
            ((const char*)strrchr(d_name, '.') &&
                (strcmp((const char*)strrchr(d_name, '.'), ".mp3") == 0 ||
                    strcmp((const char*)strrchr(d_name, '.'), ".wav") == 0))) ) {
            //ESP_LOGI(TAG, "过滤后完整路径filname[%d]:%s\n", i, filepath);

            if (i >= NUMBER_FILES_MAX) {
                ESP_LOGW(TAG, "警告！！！读取的文件数量为：%d 已超过：%d",i+1, NUMBER_FILES_MAX);
                break;
            }

            i++;
            if (NULL == second_son_absolute_path[i - 1]) {
                second_son_absolute_path[i - 1] = (char*)heap_caps_malloc_dflt(PATH_BYTE_SIZE);//二级目录绝对路径开辟空间
            }
            strlcpy(second_son_absolute_path[i - 1], filepath, PATH_BYTE_SIZE);
            if (i <= 10) {
                ESP_LOGI(TAG, "second_son_absolute_path[%d]:%s\n", i - 1, second_son_absolute_path[i - 1]);
            }

            if (NULL == second_level_directory[i - 1]) {
                second_level_directory[i - 1] = (char*)heap_caps_malloc_dflt(MDLD_FILE_NAME_BUF_LEN);//二级目录名称开辟空间
            }
            strlcpy(second_level_directory[i - 1], fileName, MDLD_FILE_NAME_BUF_LEN);
            if (i <= 10) {
                ESP_LOGI(TAG, "second_level_directory[%d]:%s\n", i - 1, second_level_directory[i - 1]);
            }
        }
        d_name[0] = '\0';
    }
    Number_secondary_file_dir = i;
    ESP_LOGI(TAG, "一共检索到Number_secondary_file_dir = %d个文件!\n", i);
    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
    //3, 关闭目录 
    lv_fs_dir_close(&stDir); 
    return i;
}

//遍历打印某一文件夹下的子文件目录
//返回值：文件目录的个数
int Print_file_directory(char* dirname)
{
    LV_LOG_USER("已进入：遍历打印文件目录\n");
    ESP_LOGI(TAG, "遍历的是%s文件夹中的内容\n", dirname);

    int fileMode = 0;
    zb_stat__st_mode(dirname, &fileMode);
    if ((S_IFDIR & fileMode) == 0)
        return -1;
#ifndef C_PC_SIM
    //1,打开目录文件
    DIR* dp = opendir(dirname);
    if (NULL == dp)
    {
        ESP_LOGE(TAG, "打开失败！");
        return -1;
    }
    //2, 循环读取目录下的文件信息 --> 直到读取完毕 
    int i = 0;
    struct dirent* fp;
    while ((fp = readdir(dp)))	//循环读取目录中的信息,如果返回值为NULL,结束循环 
    {
        char filepath[PATH_BYTE_SIZE];//存放文件的完整路径   char filepath[PATH_MAX];
        //忽略当前目录和上级目录
        if (strcmp(".", fp->d_name) == 0 || strcmp("..", fp->d_name) == 0) 
        {
            continue;
        }
        snprintf(filepath,sizeof(filepath), "%s/%s", dirname, fp->d_name);//构造新路径
        ESP_LOGI(TAG, "filname[%d]:%s\n", i++, filepath);
    }
    //Number_secondary_file_dir = i;
    ESP_LOGI(TAG, "一共检索到%d个文件!\n", i);
    //3, 关闭目录 
    closedir(dp);
    return i;
#else
    return 0;
#endif
}

//打印出指定目录的目录树
//path:路径
//indent:缩进
//返回值：文件目录的个数
static int Dsc = 0;//被指定文件夹的文件数量
static int dsc = 0;//当前文件夹中文件数量
static int Num_folders = 0;//被指定文件夹的文件夹数量

int Print_directory_tree(char* path,int indent){
    ESP_LOGI(TAG, "已进入：打印出指定目录%s的目录树,目录深度为%d-\n",path,indent);
#ifndef C_PC_SIM
	DIR *dir;
	struct dirent *entry; 
	char new_path[PATH_BYTE_SIZE]; //新的路径
    //1,打开目录文件
    dir = opendir(path);
    if (NULL == dir)
    {
        ESP_LOGE(TAG, "opendir failed");
        return -1;
    }
	for (int j = 0; j < indent; j++) {  
        printf("—");  //缩进
    }  
    printf("%s/\n", path);
    //2, 循环读取目录下的文件信息 --> 直到读取完毕 
    int i = 0;
    while ((entry = readdir(dir)))	//循环读取目录中的信息,如果返回值为NULL,结束循环 
    {
    	//目录：递归调用自己
    	if (entry->d_type == DT_DIR) { 
        	//忽略当前目录和上级目录
        	if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) 
        	{
           	 	continue;
        	}
       	 	snprintf(new_path,sizeof(new_path), "%s/%s", path, entry->d_name);//构造新路径
            Print_directory_tree(new_path, indent + 1); // 递归调用  
            ESP_LOGI(TAG, "当前文件夹下的文件数量为：%d\n", dsc);
			ESP_LOGI(TAG, "new_path[%d]:%s\n", i++, new_path);
			dsc = 0;//当前文件夹的文件数量
			Num_folders++;//被指定文件夹的文件夹数量
    	}
		//文件：直接打印文件名
		else {  
            for (int k = 0; k < indent; k++) {  
                printf("—"); //缩进 
            }  
            printf("%s\n", entry->d_name);// 打印文件 
            dsc++;
            Dsc++;
        }  
    }
    ESP_LOGI(TAG, "一共检索到%d个文件!\n", i);
    ESP_LOGI(TAG, "被指定文件夹的文件数量为：%d\n", Dsc);
	ESP_LOGI(TAG, "被指定文件夹的文件夹数量为：%d\n", Num_folders);
    //3, 关闭目录 
    closedir(dir);
    return i;
#else
    return 0;
#endif
}

/**
* 删除指定目录的目录树
* path:指定目录的路径
* 返回值：
*   成功：>=0
*   失败：<0
*/
int delete_directory(const char* path) {
    LV_LOG_USER("已进入：删除指定目录的目录树\n");
	return zb_del_dir(path);
}

/**
* 删除指定文件和目录树
* path:指定目录的路径
* 返回值：
*   成功：>=0
*   失败：<0
*/
int Del_dir_tree_from_specified_directory(char* path) {
    LV_LOG_USER("已进入：删除指定文件和目录树\n");

    int st_mode = 0;
    int fsRet = zb_stat__st_mode(path, &st_mode);
    // 获取文件状态
    if (fsRet < 0) {
        ESP_LOGE(TAG, "获取文件状态 失败！");
        return -1;
    }

    // 如果是目录，递归删除目录树
    if (S_ISDIR(st_mode)) {
        ESP_LOGI(TAG, "是目录，递归删除目录树");
        return zb_del_dir(path);
    }
    else {
        // 如果是文件，直接删除
        ESP_LOGI(TAG, "是文件，直接删除");
        return zb_remove(path);
    }
}



