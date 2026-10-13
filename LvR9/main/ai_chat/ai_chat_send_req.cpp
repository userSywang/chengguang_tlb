extern "C"{
    #include "../../trd/openssl/include/openssl/sha.h"
    #include "../src/wifi/wifi_connect.h"
    #include "../../trd/tz_audio_player/include/tz_pcm_audio.h"
    #include "../../trd/tz_audio_player/include/tz_audio_player.h"
}

#include "ai_chat.h"
#include "json_parse_struct.h"
#include "http_utils.h"
#include <chrono>
#include <random>
#include <sstream>
#include <functional>
#include <iomanip>
#include <unistd.h>   // POSIX 标准头文件
#include <sys/stat.h> // 文件状态检查
#include <cstdio>     // std::remove
#include <iostream>
#include <algorithm> // std::copy_n
#include <fstream> 
#include "../include/utf8.h"

#define WAV_AUDIO_PATH "/userdata/ai_chat.wav"
#define WAV_AUDIO_SINGAL_PATH "/userdata/ai_chat_mono.wav"

constexpr char BASE64_CHARS[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::unique_ptr<GetDefaultTopicResponse> g_defaultTopicRes = nullptr;
std::unique_ptr<GenerateDialogResponse> g_generateDialogRes = nullptr;
std::unique_ptr<GenerateTopicResponse> g_generateTopicRes = nullptr;
std::unique_ptr<GenerateRecommendationResponse> g_generateRecommendationRes = nullptr;
std::vector<HistoryItem> g_chatHistory;

std::unique_ptr<EnZhTranslationResponse> g_enzhTranslationRes = nullptr;
std::unique_ptr<AsrResponse> g_AsrRes = nullptr;
std::unique_ptr<TtsResponse> g_TtsRes = nullptr;

bool get_is_net_connected() {
	return sys_get_wifi_link_state();
}

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

void ai_chat_take_audio_start_5s(){
    tz_record_wav(WAV_AUDIO_PATH,10,1);
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

void play_tts_audio() {

    // if(!g_TtsRes.get()->result.empty()) {
    //     const std::string filepath = "/userdata/tts_output.mp3";
    //     std::ofstream ofs(filepath, std::ios::binary);
    //     if(ofs) {
    //         ofs.write(reinterpret_cast<const char*>(g_TtsRes.get()->result.data()), 
    //                  g_TtsRes.get()->result.size());
    //         std::cout << "Audio saved to: " << filepath << std::endl;
    //     } else {
    //         std::cerr << "Failed to open file for writing: " << filepath << std::endl;
    //     }
    //     ofs.close();

    //     // 播放音频文件
    //     tz_play_audio("/userdata/tts_output.mp3",0,-1);
    // }
    static TZ_AUD_SRC_STREAM_T stream;
    stream.audSrcLength = g_TtsRes.get()->result.size();
    stream.audSrc_read = [](void* buf, size_t size) -> int {
            static size_t pos = 0;
            size_t copy_size = std::min(size, g_TtsRes.get()->result.size() - pos);
            memcpy(buf, g_TtsRes.get()->result.data() + pos, copy_size);
            pos += copy_size;
            return copy_size;
        };
    stream.audSrc_seek = [](long offset, int whence) -> int { return 0; };
    stream.audSrc_close = []() {};
    
    tz_play_mem_audio_start(&stream);
}

//tools function
template<typename REQ>
std::string get_sign(const REQ& req,const std::string& q,const std::string& secret) {
    std::string input = q;
    size_t len = utf8::distance(q.begin(), q.end());

    if (len > 20) {
        auto front_end = q.begin();
        utf8::advance(front_end, 10, q.end());
        std::string first_part(q.begin(), front_end);

        auto back_start = q.begin();
        utf8::advance(back_start, len - 10, q.end());
        std::string last_part(back_start, q.end());
        input = first_part
              + std::to_string(len) 
              + last_part;
    }
    std::string signStr = req.appKey
                        + input
                        + req.salt
                        + req.curtime
                        + secret;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, signStr.c_str(), signStr.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }

    return ss.str();
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


template<typename REQ,typename RES>
bool send_request(const REQ& req,
                  const std::function<RES(const REQ&)>& send_req_fun,
                  std::unique_ptr<RES>& res) {
    try {
        res = std::make_unique<RES>(send_req_fun(req));
    } catch (const std::exception& e) {
        printf("Request failed: %s\n", e.what());
        return false;
    }
    return res->code == "0";
}

bool ai_chat_get_default_topic() {
	GetDefaultTopicRequest req;
	req.appKey = AI_TEACH_APP_KEY;
    req.q = "topics";
    req.signType = "v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.sign = get_sign<GetDefaultTopicRequest>(req,req.q,AI_TEACH_APP_SECRET);
    
	return send_request<GetDefaultTopicRequest,GetDefaultTopicResponse>
                    (req,send_get_default_topic_request,g_defaultTopicRes);
}
bool ai_chat_generate_topic(std::string& topic) {
	GenerateTopicRequest req;
    req.appKey = AI_TEACH_APP_KEY;
    req.topic = topic;
    req.signType = "v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.sign = get_sign<GenerateTopicRequest>(req,req.topic,AI_TEACH_APP_SECRET);
    return send_request<GenerateTopicRequest,GenerateTopicResponse>
                    (req,send_generate_topic_request,g_generateTopicRes);
}

bool ai_chat_generate_dialog(const std::string& taskId,
                            const Scene& scene,
                            const std::string& userLevel,
                            const std::vector<HistoryItem>& history) {
	GenerateDialogRequest req;
    req.appKey = AI_TEACH_APP_KEY;
    req.signType = "v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.taskId = taskId;
    req.scene = scene;
    req.userLevel = userLevel;
    req.history = history;
    req.sign = get_sign<GenerateDialogRequest>(req,req.taskId,AI_TEACH_APP_SECRET);
	return send_request<GenerateDialogRequest,GenerateDialogResponse>
                    (req,send_generate_dialog_request,g_generateDialogRes);
}
bool ai_chat_generate_recommendation(const std::string& taskId,
                                    const Scene& scene,
                                    const std::string& userLevel,
                                    const std::vector<HistoryItem>& history,
                                    const std::string& count,
                                    const std::vector<std::string>& indexArr){
    GenerateRecommendationRequest req;
    req.appKey = AI_TEACH_APP_KEY;
    req.signType = "v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.taskId = taskId;
    req.scene = scene;
    req.userLevel = userLevel;
    req.history = history;
    req.count = count;
    req.indexArr = indexArr;
    req.sign = get_sign<GenerateRecommendationRequest>(req,req.taskId,AI_TEACH_APP_SECRET);
	return send_request<GenerateRecommendationRequest,GenerateRecommendationResponse>
                    (req,send_generate_recommendation_request,g_generateRecommendationRes);
}

bool ai_chat_enzh_translation(const std::string& q){
    EnZhTranslationRequest req;
    req.appKey = TRANSLATE_APP_KEY;
    req.signType = u8"v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.q = q;
    req.sign = get_sign<EnZhTranslationRequest>(req,req.q,TRANSLATE_APP_SECRET);
    req.from = u8"en";
    req.to = u8"zh-CHS";
    return send_request<EnZhTranslationRequest,EnZhTranslationResponse>
                    (req,send_enzh_translation_request,g_enzhTranslationRes);
}

bool ai_chat_asr(){
    AsrRequest req;
    req.q = To_Base64(WAV_AUDIO_SINGAL_PATH);
    req.appKey = ASR_APP_KEY;
    req.signType = u8"v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.sign = get_sign<AsrRequest>(req,req.q,ASR_APP_SECRET);
    req.langType = u8"en";
    req.format = u8"wav";
    req.rate = u8"16000";
    req.channel = u8"1";
    req.type = u8"1";
    req.q = url_encode(req.q);
    return send_request<AsrRequest,AsrResponse>
                    (req,send_Asr_request,g_AsrRes);
}

bool ai_chat_tts(const std::string& q,const std::string& language){
    TtsRequest req;
    req.q = "你好你好你好你";
    std::cout<<"input q = "<< req.q<<std::endl;
    req.appKey = TTS_APP_KEY;
    req.signType = u8"v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.sign = get_sign<TtsRequest>(req,req.q,TTS_APP_SECRET);

    req.q = url_encode(req.q);
    req.speed = u8"1.0";
    req.volume = u8"1.0"; 
    req.format = u8"mp3";
    std::string encoded_voice = (language == "en") ? u8"youxiaomei" : u8"youxiaoxun";
    req.voiceName = encoded_voice;
    return send_request<TtsRequest,TtsResponse>
                    (req,send_Tts_request,g_TtsRes);
}