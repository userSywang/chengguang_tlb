#include "http_utils.h"
#include "json_utils.h"
#include <sstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    ResponseData* resp = static_cast<ResponseData*>(userp);
    resp->content.append(static_cast<char*>(contents), realsize);
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

template<typename Req, typename Res>
Res send_http_request(const std::string& url,
                      const std::string& content_type,
                      const Req& request,
                      const std::function<std::string(const Req&)>& to_json,
                      const std::function<Res(const std::string&)>& parse_res) {
    static bool initialized = false;
    if (!initialized) {
        curl_global_init(CURL_GLOBAL_ALL);
        initialized = true;
    }
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
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
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
        response = parse_res(resp_data.content);
    } catch (const std::exception& e) {
        response.code = "PARSE_ERROR";
        response.msg = e.what();
    }
    return response;
}

template<>
TtsResponse send_http_request<TtsRequest, TtsResponse>(
    const std::string& url,
    const std::string& content_type,
    const TtsRequest& request,
    const std::function<std::string(const TtsRequest&)>& to_json,
    const std::function<TtsResponse(const std::string&)>& parse_res) 
{
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
            response.result.assign(resp_data.content.begin(), resp_data.content.end());
            response.code = "0";
        } else if (resp_data.content_type.find("application/json") != std::string::npos) {
            response = parse_res(resp_data.content);
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
        [](const std::string& json) { return parse_get_default_topic_response(json); }
    );
}

GenerateTopicResponse send_generate_topic_request(const GenerateTopicRequest& req) {
    return send_http_request<GenerateTopicRequest, GenerateTopicResponse>(
        AI_TEACH_API_GENERATE_TOPIC,"Content-Type: application/json",
        req,
        [](const GenerateTopicRequest& r) { return generate_topic_request_to_json(r); },
        [](const std::string& json) { return parse_generate_topic_response(json); }
    );
}

GenerateDialogResponse send_generate_dialog_request(const GenerateDialogRequest& req) {
    return send_http_request<GenerateDialogRequest, GenerateDialogResponse>(
        AI_TEACH_API_GENERATE_DIALOG,"Content-Type: application/json",
        req,
        [](const GenerateDialogRequest& r) { return generate_dialog_request_to_json(r); },
        [](const std::string& json) { return parse_generate_dialog_response(json); }
    );
}

GenerateRecommendationResponse send_generate_recommendation_request(const GenerateRecommendationRequest& req) {
    return send_http_request<GenerateRecommendationRequest, GenerateRecommendationResponse>(
        AI_TEACH_API_GENERATE_RECOMMENDATION,"Content-Type: application/json",
        req,
        [](const GenerateRecommendationRequest& r) { return generate_recommendation_request_to_json(r); },
        [](const std::string& json) { return parse_generate_recommendation_response(json); }
    );
}

EnZhTranslationResponse send_enzh_translation_request(const EnZhTranslationRequest& req){
    return send_http_request<EnZhTranslationRequest, EnZhTranslationResponse>(
        TRANSLATE_APP_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const EnZhTranslationRequest& r) { return enzh_translation_request_to_json(r); },
        [](const std::string& json) { return parse_enzh_translation_response(json); }
    );
}

AsrResponse send_Asr_request(const AsrRequest& req){
    return send_http_request<AsrRequest, AsrResponse>(
        ASR_APP_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const AsrRequest& r) { return asr_request_to_json(r); },
        [](const std::string& json) { return parse_asr_response(json); }
    );
}

TtsResponse send_Tts_request(const TtsRequest& req){
    return send_http_request<TtsRequest, TtsResponse>(
        TTS_APP_URL,"Content-Type: application/x-www-form-urlencoded",
        req,
        [](const TtsRequest& r) { return tts_request_to_json(r); },
        [](const std::string& json) { return parse_tts_response(json); }
    );
}