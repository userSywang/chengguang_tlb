
#include "http_get_online_res.h"
#include <sstream>
#include <iostream>
#include <string>
#include <chrono>
#include "global_http_tool.h"


using json = nlohmann::json;

size_t write_callback_tts(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    ResponseData_TTS* resp = static_cast<ResponseData_TTS*>(userp);
    
    // 修正指针类型转换并改用范围插入
    const unsigned char* data_start = static_cast<unsigned char*>(contents);
    const unsigned char* data_end = data_start + realsize;
    resp->content.insert(resp->content.end(), data_start, data_end);
    
    return realsize;
}

//send http request template specialized
template<>
TtsResponse send_http_request<TtsRequest, TtsResponse>(
    const std::string& url,
    const std::string& content_type,
    const TtsRequest& request,
    const std::function<std::string(const TtsRequest&)>& to_json,
    const std::function<TtsResponse(const std::string&)>& parse_res,
    bool is_post_req) {
    static bool initialized = false;
    if (!initialized) {
        curl_global_init(CURL_GLOBAL_ALL);
        initialized = true;
    }
    CURL* curl = curl_easy_init();
    TtsResponse response;
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
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_data);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    std::cout << "post_data:" << post_data.c_str() << std::endl;

    CURLcode res = curl_easy_perform(curl);
    
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
        if (resp_data.content_type.find("audio/mp3") != std::string::npos) {
            std::cout<<"resp_data.content.size():"<<resp_data.content.size()<<std::endl;
            response.result.resize(resp_data.content.size());
            std::transform(resp_data.content.begin(), resp_data.content.end(),
                response.result.begin(),
                [](char c) { return static_cast<unsigned char>(c); });
            response.code = "0";
        } else if (resp_data.content_type.find("application/json") != std::string::npos) {
            //response = parse_res(resp_data.content); // 直接使用迭代器构造);
        }
    } catch (const std::exception& e) {
        response.code = "PARSE_ERROR";
        response.msg = e.what();
    }
    return response;
}


// 具体接口实现
GetDefaultTopicResponse send_get_default_topic_request(const GetDefaultTopicRequest& req) {
    return send_http_request<GetDefaultTopicRequest, GetDefaultTopicResponse>(
        AI_TEACH_API_GET_DEFAULT_TOPIC,"Content-Type: application/json",
        req,
        [](const GetDefaultTopicRequest& r) { return get_default_topic_request_to_json(r); },
        [](const std::string& json) { return parse_get_default_topic_response(json); },
        true
    );
}

GenerateTopicResponse send_generate_topic_request(const GenerateTopicRequest& req) {
    return send_http_request<GenerateTopicRequest, GenerateTopicResponse>(
        AI_TEACH_API_GENERATE_TOPIC,"Content-Type: application/json",
        req,
        [](const GenerateTopicRequest& r) { return generate_topic_request_to_json(r); },
        [](const std::string& json) { return parse_generate_topic_response(json); },
        true
    );
}

GenerateDialogResponse send_generate_dialog_request(const GenerateDialogRequest& req) {
    return send_http_request<GenerateDialogRequest, GenerateDialogResponse>(
        AI_TEACH_API_GENERATE_DIALOG,"Content-Type: application/json",
        req,
        [](const GenerateDialogRequest& r) { return generate_dialog_request_to_json(r); },
        [](const std::string& json) { return parse_generate_dialog_response(json); },
        true
    );
}

GenerateRecommendationResponse send_generate_recommendation_request(const GenerateRecommendationRequest& req) {
    return send_http_request<GenerateRecommendationRequest, GenerateRecommendationResponse>(
        AI_TEACH_API_GENERATE_RECOMMENDATION,"Content-Type: application/json",
        req,
        [](const GenerateRecommendationRequest& r) { return generate_recommendation_request_to_json(r); },
        [](const std::string& json) { return parse_generate_recommendation_response(json); },
        true
    );
}

EnZhTranslationResponse send_enzh_translation_request(const EnZhTranslationRequest& req){
    return send_http_request<EnZhTranslationRequest, EnZhTranslationResponse>(
        TRANSLATE_APP_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const EnZhTranslationRequest& r) { return enzh_translation_request_to_json(r); },
        [](const std::string& json) { return parse_enzh_translation_response(json); },
        true
    );
}

AsrResponse send_Asr_request(const AsrRequest& req){
    return send_http_request<AsrRequest, AsrResponse>(
        ASR_APP_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const AsrRequest& r) { return asr_request_to_json(r); },
        [](const std::string& json) { return parse_asr_response(json); },
        true
    );
}

TtsResponse send_Tts_request(const TtsRequest& req){
    return send_http_request<TtsRequest, TtsResponse>(
        TTS_APP_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const TtsRequest& r) { return tts_request_to_json(r); },
        [](const std::string& json) { return parse_tts_response(json); },
        true
    );
}

GetResResponse send_GetRes_request(const GetResRequest& req){
    return send_http_request<GetResRequest, GetResResponse>(
        GET_ONLINE_RES_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const GetResRequest& r) { return get_res_request_to_json(r); },
        [](const std::string& json) { return parse_get_res_response(json); },
        false
    );
}

GetResResponse send_GetAllMediaRes_request(const GetResRequest& req){
    return send_http_request<GetResRequest, GetResResponse>(
        GET_ONLINE_ALL_RES_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const GetResRequest& r) { return get_res_request_to_json(r); },
        [](const std::string& json) { return parse_get_res_response(json); },
        false
    );
}

DictionaryResponse send_Dictionary_request(const DictionnaryRequest& req){
    return send_http_request<DictionnaryRequest, DictionaryResponse>(
        DICTIONNARY_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const DictionnaryRequest& r) { return dictionary_request_to_json(r); },
        [](const std::string& json) { return parse_dictionary_response(json); },
        true
    );
}

