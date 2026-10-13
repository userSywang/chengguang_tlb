#include "online_res_read.h"
#include "../online_tool/http_get_online_res.h" 
#include "../online_tool/global_http_tool.h"
#include <mutex>
#include <unordered_map>
#include <list>

extern "C" {
    #include "../include/img_a_tlb.h"
    #include "../utils/misc_utils.h"
    #include "../data_parse/img_res.h"
    #include "../data_support/yd_book_select.h"
    #include <unistd.h>
}


std::unique_ptr<GetResResponse> g_tempRes = nullptr;
std::unique_ptr<std::vector<OnlineMediaNode>> g_mediaNodeList = nullptr;
std::unique_ptr<std::vector<OnlineResNode>> g_resNodeList = nullptr;
std::vector<OnlineApNode> g_ApNodeList = {
    {"剑少主题班", "/YD_YYJX/JSZTB"},
    {"外教带你说英语", "/YD_YYJX/WJSYY"},
    {"英语名著", ""},
    {"有道大语文素养", "/YD_YWSY"},
    {"课外故事", "/KWGS"},
    {"古诗岁听", ""},
    {"分级阅读", "/FJYD"},
    {},
    {"儿歌", "" },
    {"情景对话", ""},
    {"国学熏听", ""},
    {},
    {"语文教材", ""},
    {"英语教材", ""},
};
std::string g_url = "";
void async_get_online_res_from_path(const std::string& path,std::function<void()> callback){
    if(g_mediaNodeList){
        g_mediaNodeList.reset();
        g_mediaNodeList = nullptr;
    }
    get_online_media_res_req(path,500,g_tempRes,[callback](){
        if(g_tempRes == nullptr || g_tempRes.get()->code != "0"){
            if(callback) callback(); return;
        }
        g_mediaNodeList = std::make_unique<std::vector<OnlineMediaNode>>();
        for(auto& node:g_tempRes.get()->data){
            if(node.mediaType != 1) continue;
            g_url = node.mediaUrl;
            char* local_path = (char*)malloc(500);
            bool is_download = check_res_down(RES_YD_ONLINE,node.path.c_str(),&local_path,0,false);

            
            std::string local_path_str = local_path;
            free(local_path);
            g_mediaNodeList.get()->emplace_back(OnlineMediaNode{node.name,node.path,node.mediaUrl,node.mixedSubtitleUrl,is_download,local_path_str});
        }
        if(callback) callback();
    });   
}

void async_get_online_dir_list(uint8_t now_sel,std::function<void()> callback){
    if(g_resNodeList){
        g_resNodeList.reset();
        g_resNodeList = nullptr;
    }
    std::string path = "";
    path = g_ApNodeList[now_sel].path;
    get_online_res_req(path,500,g_tempRes,[callback](){
        if(g_tempRes == nullptr || g_tempRes.get()->code != "0"){
            if(callback) callback(); return;
        } 
        g_resNodeList = std::make_unique<std::vector<OnlineResNode>>();
        for(auto& node : g_tempRes.get()->data){
            if(node.nodeType != 0) continue;
            node.coverUrl = node.coverUrl + "&imageView&thumbnail=140x162";
            std::cout<<"coverUrl:"<<node.coverUrl<<std::endl;
            g_resNodeList.get()->emplace_back(OnlineResNode{node.name,node.path,node.coverUrl, false,"", false, ""});
        }
        if(callback) callback();
    });
}

lv_img_dsc_t* get_offline_img_by_ap_index(uint8_t ap_index,uint8_t book_index){
    uint8_t apIndex = 0;
    switch (ap_index){
        case ONLINE_PLAY_AP_EGE:
            apIndex = 0;
            break;
        case ONLINE_PLAY_AP_QINGJING:
            apIndex = 1;
            break;
        case ONLINE_PLAY_AP_GUOXUE:
            apIndex = 2;
            break;
        case ONLINE_PLAY_AP_GUSHI:
            apIndex = 3;
            break;
        default:
            return nullptr;
    }
    if(book_index >= g_imgList[apIndex].size()) return nullptr;
    lv_img_dsc_t* pImgDec = zb_img_res_load_one(NULL, g_imgList[apIndex][book_index], IMG_FREE_TYPE_AUTO, NULL);
    return pImgDec;
}

lv_img_dsc_t* get_tch_img_by_path(std::string path){
    if(path.empty()) return nullptr;
    lv_img_dsc_t *img_Src = zb_load_image_from_file(path.c_str());
    return img_Src;
}

struct ImageCacheEntry {
    std::vector<uint8_t> data;
    lv_img_dsc_t dsc;
    ImageCacheEntry(std::vector<uint8_t>&& img_data) : data(std::move(img_data)) {
        init_dsc();
    }
    void init_dsc() {
        dsc.data = data.data();
        dsc.data_size = static_cast<uint32_t>(data.size());
        dsc.header.always_zero = 0;
        dsc.header.w = 0;
        dsc.header.h = 0;
        dsc.header.cf = LV_IMG_CF_RAW;
    }
    ImageCacheEntry(const ImageCacheEntry&) = delete;
    ImageCacheEntry& operator=(const ImageCacheEntry&) = delete;
    ImageCacheEntry(ImageCacheEntry&&) = default;
    ImageCacheEntry& operator=(ImageCacheEntry&&) = default;
};

lv_img_dsc_t* get_online_img_by_url(const std::string& url) {
    if(url.empty()) return nullptr;
    
    static const size_t MAX_CACHE_SIZE = 100; // 实际缓存大小应根据内存调整
    static std::mutex cache_mutex;
    static std::list<std::string> access_order;
    static std::unordered_map<std::string, 
        std::pair<ImageCacheEntry, std::list<std::string>::iterator>> img_cache;
    
    std::lock_guard<std::mutex> lock(cache_mutex);
    
    auto it = img_cache.find(url);
    if(it != img_cache.end()) {
        access_order.splice(access_order.begin(), access_order, it->second.second);
        return &it->second.first.dsc;
    }
    
    std::vector<uint8_t> img_data;
    if(!download_http_request_buf(url, img_data) || img_data.empty()) {
        ESP_LOGE("IMG", "下载失败: %s", url.c_str());
        return nullptr;
    }
    
    if(img_cache.size() >= MAX_CACHE_SIZE) {
        auto lru_url = access_order.back();
        access_order.pop_back();
        img_cache.erase(lru_url);
    }
    
    ImageCacheEntry entry(std::move(img_data));
    access_order.push_front(url);
    auto res = img_cache.emplace(url, std::make_pair(std::move(entry), access_order.begin()));
    
    return &res.first->second.first.dsc;
}

#define TAG "MY_MKDIR"
int my_mkdir(const char *path) {
    int mode = 0777;
    int iFuncRet = -1;
    char *p = strdup(path);
    char *sep = p + 1; 
    
    while ((sep = strchr(sep, '/'))) {
        *sep = '\0';
        // 增强存在性检查
        if (access(p, F_OK) != 0) { 
            iFuncRet = mkdir(p, mode);
            if (iFuncRet < 0 && errno != EEXIST) {
                ESP_LOGE(TAG, "mkdir err:%d at %s", errno, p);
                free(p);
                return -1;
            }
            else if (errno == EEXIST) {
                ESP_LOGI(TAG, "Directory already exists: %s", p);
            }
        }
        *sep = '/';
        sep++;
    }
    
    // 最终目录检查
    if (access(path, F_OK) == 0) {
        ESP_LOGI(TAG, "Target directory already exists: %s", path);
        free(p);
        return 0;
    }

    iFuncRet = mkdir(path, mode);
    if (iFuncRet < 0 && errno != EEXIST) {
        ESP_LOGE(TAG, "mkdir err:%d", errno);
    }
    free(p);
    return iFuncRet;
}



double SimpleLrcParser::parseTimestamp(const std::string& str) {
    double minutes = 0, seconds = 0;
    char colon = 0;
    
    std::istringstream iss(str);
    iss >> minutes >> colon >> seconds;
    
    if (iss.fail() || colon != ':' || minutes < 0 || seconds < 0) {
        return -1;
    }
    
    return minutes * 60 + seconds;
}

void SimpleLrcParser::parseLine(const std::string& line) {
    if (line.empty()) return;
    
    size_t pos = 0;
    while (pos < line.size() && std::isspace(line[pos])) pos++;
    if (pos >= line.size()) return;
    if (line[pos] != '[') return;
    
    size_t end_bracket = line.find(']', pos + 1);
    if (end_bracket == std::string::npos) return;
    double time = parseTimestamp(line.substr(pos + 1, end_bracket - pos - 1));
    if (time < 0) return;  // 无效时间
    std::string content = line.substr(end_bracket + 1);
    
    auto existing = std::find_if(entries.rbegin(), entries.rend(), 
        [time](const LrcEntry& e) { return e.time == time; });
    if (existing != entries.rend()) {
        if (existing->content_sec.empty()) {
            existing->content_sec = content;
        }
    } else {
        entries.push_back({time, content, ""});
    }
}
bool SimpleLrcParser::loadFromBuffer(std::vector<uint8_t>& buffer) {
    std::string content(buffer.begin(), buffer.end());
    return loadFromString(content);
}
bool SimpleLrcParser::loadFromString(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        parseLine(line);
    }
    
    std::sort(entries.begin(), entries.end(), 
        [](const LrcEntry& a, const LrcEntry& b) {
            return a.time < b.time;
        });
        
    return true;
}
bool SimpleLrcParser::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
        parseLine(line);
    }
    
    std::sort(entries.begin(), entries.end(), 
        [](const LrcEntry& a, const LrcEntry& b) {
            return a.time < b.time;
        });
        
    return true;
}


std::vector<std::vector<unsigned int>> g_imgList = {
{
    ONLINE_RES_IMG_ENG_SONG
},
{
    ONLINE_RES_IMG_JIAOTONG,
    ONLINE_RES_IMG_XIUXIAN,
    ONLINE_RES_IMG_CHANGJING,
    ONLINE_RES_IMG_QINGGAN,
    ONLINE_RES_IMG_RICHANG,
    ONLINE_RES_IMG_JIHUI
},
{
    ONLINE_RES_IMG_BAIJIAXIN,
    ONLINE_RES_IMG_DIZIGUI,
    ONLINE_RES_IMG_QIANZIWEN,
    ONLINE_RES_IMG_SANZIJING
},
{
    ONLINE_RES_IMG_ONE_FIR,
    ONLINE_RES_IMG_ONE_SEC,
    ONLINE_RES_IMG_TWO_FIR,
    ONLINE_RES_IMG_TWO_SEC,
    ONLINE_RES_IMG_THR_FIR,
    ONLINE_RES_IMG_THR_SEC,
    ONLINE_RES_IMG_FOUR_FIR,
    ONLINE_RES_IMG_FOUR_SEC,
    ONLINE_RES_IMG_FIVE_FIR,
    ONLINE_RES_IMG_FIVE_SEC,
    ONLINE_RES_IMG_SIX_FIR,
    ONLINE_RES_IMG_SIX_SEC,
    ONLINE_RES_IMG_KEWAI1,
    ONLINE_RES_IMG_KEWAI2,
    ONLINE_RES_IMG_KEWAI3,
    ONLINE_RES_IMG_KEWAI4,
    ONLINE_RES_IMG_KEWAI5,
    ONLINE_RES_IMG_KEWAI6,
    ONLINE_RES_IMG_KEWAI7,
    ONLINE_RES_IMG_KEWAI8,
    ONLINE_RES_IMG_KEWAI9
}
};


