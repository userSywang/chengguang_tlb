#ifndef __ONLINE_RES_READ_H__
#define __ONLINE_RES_READ_H__

#include <iostream>
#include <functional>
#include <fstream>  // 新增文件流操作头文件
#include <string>
#include <vector>
#include <memory>
extern "C" {
    #include <lvgl/lvgl.h>
}

enum OnlinePlayApSel{
    ONLINE_PLAY_AP_JIANQIAO,
    ONLINE_PLAY_AP_WAIJIAO,
    ONLINE_PLAY_AP_YINGYUMINZHU,
    ONLINE_PLAY_AP_DAYUWEN,
    ONLINE_PLAY_AP_KEWAIGUSHI,
    ONLINE_PLAY_AP_GUSHI,
    ONLINE_PLAY_AP_FENJI,
    IS_ONLINE,
    ONLINE_PLAY_AP_EGE,
    ONLINE_PLAY_AP_QINGJING,
    ONLINE_PLAY_AP_GUOXUE,
    IS_TEACH_MTL,
    ONLINE_PLAY_AP_YUWEN,
    ONLINE_PLAY_AP_YINYU,
    ONLINE_PLAY_AP_SUN,
};


void async_get_online_res_from_path(const std::string& path,std::function<void()> callback);
void async_get_online_dir_list(uint8_t now_sel,std::function<void()> callback);
lv_img_dsc_t* get_offline_img_by_ap_index(uint8_t ap_index,uint8_t book_index);
lv_img_dsc_t* get_tch_img_by_path(std::string path);
lv_img_dsc_t* get_online_img_by_url(const std::string& url);

int my_mkdir(const char *path);

struct OnlineApNode{
    std::string name;       //模块名称
    std::string path;       //对应的在线path
};

struct OnlineResNode{
    std::string name;       //目录、图书名称
    std::string path;       //对应的在线path
    std::string coverImg;   //当为教材时为在本地的路径 为内置资源时为空 为有道资源时为url
    bool isChenGuang;
    std::string mediaUrl;   //为晨光资源是该字段有效
    bool isDownlaoded;      //为晨光资源是该字段有效
    std::string local_path; //为晨光资源是该字段有效 指示将资源下载到哪个目录 或在哪个目录打开资源
};

struct OnlineMediaNode{
    std::string name;       //音频名称
    std::string path;       //对应的在线path
    std::string mediaUrl;   //当为有道资源时该字段有效 可用于判断属于那种资源
    std::string SubtitleUrl;
    bool isDownlaoded;
    std::string local_path; //为有道资源是该字段有效 指示将资源下载到哪个目录 或在哪个目录打开资源
};


extern std::unique_ptr<std::vector<OnlineMediaNode>> g_mediaNodeList;    //二层目录 音频列表
extern std::unique_ptr<std::vector<OnlineResNode>> g_resNodeList;          //三层目录 音频列表
extern std::vector<OnlineApNode> g_ApNodeList;
extern std::vector<std::vector<unsigned int>> g_imgList;

extern std::string g_url;

struct LrcEntry {
    double time = 0.0;
    std::string content_fir;
    std::string content_sec;
};

struct SimpleLrcParser {
    std::vector<LrcEntry> entries;
    
    bool loadFromFile(const std::string& filepath);
    bool loadFromString(const std::string& content);
    bool loadFromBuffer(std::vector<uint8_t>& buffer);
    //const std::string* getContentAt(double time) const;
    
private:
    void parseLine(const std::string& line);
    double parseTimestamp(const std::string& str);
};
#endif