#ifndef _WEB_DISK_PLAYBACK_DISPLAY_H_
#define _WEB_DISK_PLAYBACK_DISPLAY_H_


//切换播放器
#define SWITCH_PLAYER_WB_MAINCONT   0x12   //百度网盘已下载的主界面
#define SWITCH_PLAYER_WB_TEXTBOOK   0x13   //百度网盘已下载的子界面
#define SWITCH_PLAYER_MYDOWN        0x14   //我的下载


#define PATH_BYTE_SIZE 300    //路径 buff 的字节大小


extern char* Baidu_displays_absolute_path_file_played;//要显示播放的文件绝对路径

extern int player_toggle_layout_flag_bit;//播放器切换版面标志位



extern int zb_ap_web_disk_playback_display_init(void); // 初始化播放显示 

extern int zb_ap_web_disk_playback_display_free(void); // 删除播放界面 

#endif

