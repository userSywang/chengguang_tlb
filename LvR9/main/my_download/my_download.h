#ifndef _MY_DOWNLOAD_H_
#define _MY_DOWNLOAD_H_

#include <stdio.h>
#include "../utils/misc_utils.h"
#include "../data_parse/analyze_listening.h"
//#include "../upd_data_parse/favorites.h"
#include <string.h>
#include <sys/stat.h>

#ifndef C_PC_SIM  //播放音乐文件
#include "../include/audio_stream.h"
#include "../include/audio_media_def.h"
//#include "../include/audio_player.h"
#endif

//#include "../baidu_netdisk/web_disk.h"

//——————————
#ifndef C_PC_SIM  //目录
#include <dirent.h>   // for opendir()  , 仅可用于小机模式

#else  //#ifndef _S_ISTYPE
#define _S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
#define S_ISREG(mode) _S_ISTYPE((mode), _S_IFREG)
#define S_ISDIR(mode) _S_ISTYPE((mode), _S_IFDIR)
#endif



#define DIRECTORY_LEVEL_MAX 10      //最大目录的层数 Directory level
#define NUMBER_FILES_MAX  100       //文件的最大数量 
#define SECONDARY_DIR_MAX   100     //二级目录最大值 Secondary dir maximum

#define MDLD_FILE_NAME_BUF_LEN  256

#ifndef PATH_BYTE_SIZE
#define PATH_BYTE_SIZE 300  //路径字节大小
#endif

#define SD_CARD_FILE_NAME   "/userdata"  //SD卡文件名称SD card file name


extern char* previous_level_directory;//前一级目录 Previous level directory

extern char** second_level_directory;   //二级目录 
extern char** second_son_absolute_path; //二级目录绝对路径
extern int16_t Save_focus_val_all_lev_dir[DIRECTORY_LEVEL_MAX+1];//保存各级目录的焦点值

extern int Number_secondary_file_dir;//二级目录文件的数量 Number of secondary file directories
extern int Current_SD_card_directory_depth;//当前SD卡目录深度


extern int My_download_file_directory_data_processing(char* path, int Dir_depth);// 我的下载文件目录数据处理
extern int Print_file_directory(char* dirname);//遍历打印某一文件夹下的所有文件目录
extern int Print_directory_tree(char* path, int indent);//打印出指定目录的目录树
extern int delete_directory(const char* path);//删除指定目录的目录树
extern int Del_dir_tree_from_specified_directory(char* path);//删除指定文件和目录树
extern int xor_encryption_main();//文件内容加密与解密——》异或加密

extern bool enter_flag_bit_button_callback_function;//由按钮回调函数进入标志位


#ifndef C_PC_SIM
extern int search_dir(char* dirname);//检索一个目录下的文件信息 --> 返回检索的文件数量
#endif


//——————————
#define MDLD_doFreeIfNotNull(freeFunc, ptr)  if (ptr) { \
		freeFunc(ptr);  ptr = NULL; }

typedef int (*FN_MYD_SCREEN_INIT)(void);
typedef int (*FN_MYD_SCREEN_FREE)(void);

struct MYD_SCR_INF {
    FN_MYD_SCREEN_INIT m_fn_init;
    FN_MYD_SCREEN_FREE m_fn_free;
};

extern int8_t  g_MYD_scrNo;

void MYD_switchScr(uint8_t scrNoNew, int parm);
//————
//————————————————————
#define C_ZB_AP_MYD_SON_A       1  //我的下载的选择界面
#define C_ZB_AP_MYD_SON_B       2  //我的下载的删除界面
#define C_ZB_AP_MYD_SON_C       3  //我的下载的显示界面
//————————
//各界面的
extern int my_download_select_scr_init(void);
extern int my_download_select_scr_free(void);
extern int my_download_edit_scr_init(void);
extern int my_download_edit_scr_free(void);
extern int zb_example_ap_my_download_display_init(void);
extern int zb_example_ap_my_download_display_free(void);
//——————————


#endif

