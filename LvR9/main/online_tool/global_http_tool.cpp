#include <sstream>

#include <string>
#include <chrono>
#include <random>
#include <functional>
#include <iomanip>
#include <unistd.h>   // POSIX 标准头文件
#include <sys/stat.h> // 文件状态检查
#include <cstdio>     // std::remove
#include <algorithm>  // std::copy_n
#include <fstream> 
#include <mutex>
#include <iostream>
#include "global_http_tool.h"
#include "json_get_online_res.h"
extern "C"{
    #include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
    #include "../../trd/openssl/include/openssl/evp.h"
    #include "../../trd/openssl/include/openssl/rand.h"
    #include "../../trd/openssl/include/openssl/bio.h"
    #include "../../trd/openssl/include/openssl/buffer.h"
    #include "../src/wifi/wifi_connect.h"
}


ThreadPool g_thread_pool(2);
ThreadPool g_down_thread_pool(2);

constexpr char BASE64_CHARS[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string To_Base64(const std::string& FileName) {
    std::ifstream file(FileName, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件: " + FileName);
    }

    // 验证WAV头
    char riff_header[12];
    file.read(riff_header, 12);
    if (memcmp(riff_header, "RIFF", 4) != 0 || 
        memcmp(riff_header + 8, "WAVE", 4) != 0) {
        throw std::runtime_error("无效的WAV文件头");
    }

    // 定位数据块
    file.seekg(0, std::ios::end);
    const size_t total_size = file.tellg();
    const size_t header_size = 44; // 标准PCM头
    file.seekg(header_size, std::ios::beg);
    
    // 读取音频数据
    const size_t size = total_size - header_size;
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    std::string encoded;
    encoded.reserve(((size + 2) / 3) * 4);

    size_t input_pos = 0;
    while (input_pos < static_cast<size_t>(size)) {
        uint8_t bytes[3] = {0};
        size_t bytes_to_read = std::min(static_cast<size_t>(3), 
                                      static_cast<size_t>(size) - input_pos);
        std::copy_n(buffer.data() + input_pos, bytes_to_read, bytes);
        input_pos += bytes_to_read;

        encoded += BASE64_CHARS[ (bytes[0] & 0xFC) >> 2 ];
        encoded += BASE64_CHARS[ ((bytes[0] & 0x03) << 4) | ((bytes[1] & 0xF0) >> 4) ];
        encoded += (bytes_to_read > 1) ? 
                   BASE64_CHARS[ ((bytes[1] & 0x0F) << 2) | ((bytes[2] & 0xC0) >> 6) ] : '=';
        encoded += (bytes_to_read > 2) ? 
                   BASE64_CHARS[ bytes[2] & 0x3F ] : '=';
    }

    return encoded;
}  
std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << '+';
        } else {
            escaped << '%' << std::setw(2) << static_cast<int>(c);
        }
            
    }
    return escaped.str();
}
bool get_is_net_connected() {
	return sys_get_wifi_link_state();
}
std::string get_random_salt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 32; i++){
        if (i == 8 || i == 12 || i == 16 || i == 20)
            ss << "-";
        ss << (i == 12 ? 4 : (i == 16 ? dis2(gen) : dis(gen)));
    }
    return ss.str();
}
std::string get_current_time() {
	auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    return std::to_string(seconds.count());
}
std::string get_current_time_ms(){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return std::to_string(seconds.count());
}
std::string get_device_id(){
    static std::string device_id = "";
    static bool isFirst = true;
    if(isFirst){
        isFirst = false;
        tz_system_info_t sys_info;
        if(tz_get_system_info(&sys_info)){
            std::cout<<"tz_get_system_info failed"<<std::endl;
        }else{
            device_id = sys_info.sn_str;
        }
    }
    return device_id;
}
void static_curl_init(){
    static bool initialized = false;
    if (!initialized) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl_share = curl_share_init();
        curl_share_setopt(curl_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        initialized = true;
    }
}
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    ResponseData* resp = static_cast<ResponseData*>(userp);
    const char* data_start = static_cast<char*>(contents);
    const char* data_end = data_start + realsize;
    
    resp->content.insert(resp->content.end(), data_start, data_end);
    
    static size_t total_received = 0;
    
    return realsize;
}
size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    ResponseData* resp = static_cast<ResponseData*>(userdata);
    std::string header(buffer, size * nitems);
    if (header.find("Content-Type:") != std::string::npos) {
        resp->content_type = header.substr(header.find(':') + 2);
        resp->content_type.erase(resp->content_type.find_last_not_of("\r\n") + 1);
    }
    return size * nitems;
}
int curl_callback (void *clientp,double dltotal, double dlnow, double ultotal,double ulnow){
    constexpr int UPDATE_INTERVAL_MS = 500;
    static std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (now - last_update < std::chrono::milliseconds(UPDATE_INTERVAL_MS)) return 0;
    last_update = now;
    auto* data = static_cast<std::tuple<ProgressData*, std::function<void(int, curl_off_t)>*>*>(clientp);
    if (dltotal > 0) {
        std::lock_guard<std::mutex> lock(std::get<0>(*data)->mtx);
        int progress = static_cast<int>(dlnow * 100 / dltotal);
        curl_off_t speed = 0;

        auto current_time = time(nullptr);
        if (current_time - std::get<0>(*data)->last_time > 0) {
            speed = static_cast<curl_off_t>((dlnow - std::get<0>(*data)->last_dl) / 
            (current_time - std::get<0>(*data)->last_time));
        }

        auto cb_ctx = new auto(std::make_tuple(*std::get<1>(*data), progress, speed));
        lv_async_call([](void *param) {
            auto ctx = static_cast<std::tuple<std::function<void(int, curl_off_t)>, int, curl_off_t>*>(param);
            std::get<0>(*ctx)(std::get<1>(*ctx), std::get<2>(*ctx));
            delete ctx;
        }, cb_ctx);

        std::get<0>(*data)->last_time = current_time;
        std::get<0>(*data)->last_dl = dlnow;
    }
    return 0;
}
void async_download_http_request(const std::string& url, 
                               const std::string& file_path,
                               std::function<void(int, curl_off_t)> progress_callback,
                               std::function<void(bool)> completion_callback) {
        g_down_thread_pool.enqueue([=](){
        try {
            auto ctx = std::make_shared<std::tuple<std::string, std::string, 
                std::function<void(int, curl_off_t)>, std::function<void(bool)>>>(
                url, file_path, progress_callback, completion_callback);
            
            bool res = download_http_request(std::get<0>(*ctx), 
                                std::get<1>(*ctx),
                                [ctx](int progress, curl_off_t speed) {
                                    auto async_ctx = std::make_shared<std::tuple<int,curl_off_t,decltype(ctx)>>(progress, speed, ctx);
                                    lv_async_call([](void* param) {
                                        auto p = static_cast<decltype(async_ctx)*>(param);
                                        std::get<2>(*std::get<2>(**p))(std::get<0>(**p), std::get<1>(**p)); // 修正参数传递方式
                                        delete p;
                                    }, new auto(async_ctx));
            });
            if(res == true){
                lv_async_call([](void* param) {
                    auto p = static_cast<decltype(ctx)*>(param);
                    std::get<3>(**p)(true); // 下载成功
                    delete p;
                }, new auto(ctx));
            }else{
                lv_async_call([](void* param) {
                    auto p = static_cast<decltype(ctx)*>(param);
                    std::get<3>(**p)(false); // 下载成功
                    delete p;
                }, new auto(ctx));
            }
            
        } catch (...) {
            if(completion_callback) {
                lv_async_call([](void* param) {
                    auto cb = static_cast<std::function<void(bool)>*>(param);
                    (*cb)(false); // 下载失败
                    delete cb;
                }, new auto(completion_callback));
            }
        }
    });
}
bool download_http_request(const std::string& url,const std::string& file_path,
    std::function<void(int progress, curl_off_t speed)> progress_callback) {
    static_curl_init();
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL initialization failed" << std::endl;
        return false;
    }

    FILE* fp = fopen(file_path.c_str(), "wb");
    if (!fp) {
        std::cerr << "Failed to create file: " << file_path << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    ResponseData resp_data;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp_data);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    ProgressData progress_data;

    auto callback_wrapper = std::make_shared<std::tuple<ProgressData*, std::function<void(int, curl_off_t)>*>>(
        &progress_data, &progress_callback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, curl_callback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, callback_wrapper.get());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);  // 新增此行启用进度回调
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code != 200) {
        std::cerr << "[DEBUG] curl_code:" << res 
              << " http_code:" << http_code 
              << " error:" << curl_easy_strerror(res) << std::endl;
        remove(file_path.c_str());
        return false;
    }else{
        struct stat st;
        if (stat(file_path.c_str(), &st) == 0 && st.st_size == 0) {
            std::cerr << "Empty file downloaded" << std::endl;
            remove(file_path.c_str());
            return false;
        }
        std::cout<<"download success1"<<std::endl;
        return true;
    }
}

static std::mutex g_buffer_mutex;
void async_download_http_request_buf(const std::string& url,
                                std::vector<uint8_t>& buffer,
                                std::function<void(bool)> completion_callback) {
    auto buffer_ptr = std::make_shared<std::vector<uint8_t>*>(&buffer);
    
    g_down_thread_pool.enqueue([=](){
        try {
            auto ctx = std::make_shared<std::tuple<std::string, std::shared_ptr<std::vector<uint8_t>*>, 
                std::function<void(bool)>>>(url, buffer_ptr, completion_callback);
            
            std::lock_guard<std::mutex> lock(g_buffer_mutex);
            bool res = download_http_request_buf(std::get<0>(*ctx), 
                                **std::get<1>(*ctx));
            if(res){
                lv_async_call([](void* param) {
                    auto p = static_cast<decltype(ctx)*>(param);
                    std::get<2>(**p)(true);
                    delete p;
                }, new auto(ctx));
            }else{
                lv_async_call([](void* param) {
                    auto p = static_cast<decltype(ctx)*>(param);
                    std::get<2>(**p)(false);
                    delete p;
                }, new auto(ctx));
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Download error: " << e.what() << std::endl;
            if(completion_callback) {
                lv_async_call([](void* param) {
                    auto cb = static_cast<std::function<void(bool)>*>(param);
                    (*cb)(false);
                    delete cb;
                }, new auto(completion_callback));
            }
        }
    });
}

static int debug_callback(CURL* handle, curl_infotype type,
                         char* data, size_t size, void* userptr) {
    if(type == CURLINFO_TEXT) {
        std::cerr << "[CURL DEBUG] " << std::string(data, size);
    }
    return 0;
}
static int write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto& buf = *static_cast<std::vector<char>*>(userdata);
    const uint64_t total = size * nmemb;
    try {
        // 增加内存增长限制
        uint64_t current_size = buf.size() + total;
        const uint64_t max_size = 100 * 1024;
        // if (buf.size() + total > 1000 * 1024) { // 10MB最大限制
        //     std::cerr << "内存超限: " << buf.size() + total << " bytes" << std::endl;
        //     return (size_t)0;
        // }
        buf.insert(buf.end(), ptr, ptr + total);
    } catch (const std::bad_alloc& e) {
        std::cerr << "内存分配失败: " << e.what() << std::endl;
        return (size_t)0;
    }
    return total;
};
bool download_http_request_buf(const std::string& url,
                         std::vector<uint8_t>& buffer) {
    static_curl_init();
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL initialization failed" << std::endl;
        return false;
    }

    buffer.clear();
    buffer.reserve(100 * 1024);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);


    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if ( res!=CURLE_OK || (http_code < 200 || http_code >= 300) ) {
        std::cerr << "[ERROR] Failed to download: " << url 
              << " Curl code: " << res
              << " Code: " << http_code 
              << " Error: " << curl_easy_strerror(res) << std::endl;
        if(!buffer.empty()) {
            std::cerr << "[DEBUG] First 128 bytes: ";
            std::copy_n(buffer.begin(), std::min(128, (int)buffer.size()), 
                           std::ostream_iterator<char>(std::cerr, ""));
            std::cerr << std::endl;
        }
        buffer.clear();
        return false;
    }
    return !buffer.empty();
}