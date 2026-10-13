#ifndef GLOBAL_HTTP_TOOL_H
#define GLOBAL_HTTP_TOOL_H

#include <functional>
#include "../include/nlohmann/json.hpp"
#include "ThreadPool.h"
#include <iostream>
#include "../include/utf8.h"
extern "C"{
    #include "../include/curl/curl.h"
    #include <lvgl/lvgl.h>
}
using json = nlohmann::json;

struct ResponseData {
    std::string content;
    std::string content_type;
    long http_code = 0;
};
struct ResponseData_TTS {
    std::vector<unsigned char> content;
    std::string content_type;
    long http_code = 0;
};

struct ProgressData {
    std::mutex mtx;
    double last_time = 0;
    curl_off_t last_dl = 0;
};
std::string To_Base64(const std::string& FileName);
std::string url_encode(const std::string &value);

std::string get_random_salt();
std::string get_current_time();
std::string get_current_time_ms();
std::string get_device_id();

std::string get_online_res_req_sign();
bool get_is_net_connected();

void online_res_play_audio_by_url(const std::string& url);

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata);

static std::mutex res_mutex;
template<typename REQ, typename RES>
void send_request_async(const REQ& req,
                      const std::function<RES(const REQ&)>& send_req_fun,
                      std::unique_ptr<RES>& res,
                      std::function<void()> callback) {
    using ContextType = std::tuple<REQ, std::function<RES(const REQ&)>, 
                                std::unique_ptr<RES>*, std::function<void()>>;

    auto ctx = new ContextType(req, send_req_fun, &res, callback);
    g_thread_pool.enqueue([ctx]() {
        auto& [req, fun, res_ptr, cb] = *ctx;
        try {
            auto temp_res = fun(req);
            if(temp_res.code != "0"){
                std::cout << "send_request_async failed, code:" << temp_res.code << ", msg:" << temp_res.msg << std::endl;
            }

            auto shared_ctx = new auto(make_tuple(temp_res, ctx));
            lv_async_call([](void* param) {
                auto ctx_tuple = static_cast<std::tuple<RES, ContextType*>*>(param);
                auto& [tmp_res, ctx] = *ctx_tuple;
    
                if(ctx) {
                    std::lock_guard<std::mutex> lock(res_mutex);
                    if(std::get<2>(*ctx)) {
                        *std::get<2>(*ctx) = std::make_unique<RES>(std::move(tmp_res));
                    }
                    if(std::get<3>(*ctx)) {
                        std::get<3>(*ctx)(); 
                    }
                }
                delete ctx_tuple; // Ensure proper cleanup
            }, shared_ctx);
        } catch (...) {
            delete ctx;
        }
    });
}

static CURLSH* curl_share = nullptr;
void static_curl_init();
template<typename Req, typename Res>
Res send_http_request(const std::string& url,
                      const std::string& content_type,
                      const Req& request,
                      const std::function<std::string(const Req&)>& to_json,
                      const std::function<Res(const std::string&)>& parse_res,
                      bool is_post_req) {
    static_curl_init();
    CURL* curl = curl_easy_init();
    Res response;
    ResponseData resp_data;

    if (!curl) {
        response.code = "CURL_ERROR";
        response.msg = "Failed to initialize CURL";
        return response;
    }

    std::string post_data;
    try {
        post_data = to_json(request);
    } catch (const std::exception& e) {
        response.code = "SERIALIZE_ERROR";
        response.msg = e.what();
        curl_easy_cleanup(curl);
        return response;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, content_type.c_str());
    
    curl_easy_setopt(curl,CURLOPT_SHARE,curl_share);
    curl_easy_setopt(curl,CURLOPT_DNS_CACHE_TIMEOUT,60);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    if(is_post_req){
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
        std::cout << "post_data:" << post_data.c_str() << std::endl;
    }else{
        std::string urltem = url + "?" + post_data;
        std::cout<<"urltem:"<<urltem<<std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, urltem.c_str()); 
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }
   
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    auto start = std::chrono::high_resolution_clock::now();
    CURLcode res = curl_easy_perform(curl);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("send_http_requesr spend: %lld ms\n", duration.count());

    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp_data.http_code);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        response.code = "NETWORK_ERROR";
        response.msg = curl_easy_strerror(res);
        return response;
    }

    if (resp_data.http_code != 200) {
        std::stringstream ss;
        ss << "HTTP Error: " << resp_data.http_code;
        response.code = "HTTP_ERROR";
        response.msg = ss.str();
        return response;
    }

    try {
        response = parse_res(resp_data.content);
    } catch (const std::exception& e) {
        // 增加响应内容日志输出
        std::cerr << "API Response: " << resp_data.content << std::endl; 
        response.code = "PARSE_ERROR";
        response.msg = e.what();
    }
    return response;
}

void async_download_http_request(const std::string& url, 
    const std::string& file_path,
    std::function<void(int, curl_off_t)> progress_callback,
    std::function<void(bool)> completion_callback);
bool download_http_request(const std::string& url,const std::string& file_path,
    std::function<void(int progress, curl_off_t speed)> progress_callback);

void async_download_http_request_buf(const std::string& url,
    std::vector<uint8_t>& buffer,
    std::function<void(bool)> completion_callback);
bool download_http_request_buf(const std::string& url,
    std::vector<uint8_t>& buffer);
    
#endif