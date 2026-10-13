#ifndef JSON_STRUCT
#define JSON_STRUCT

#include <string>
#include <vector>


/*
本文件用于定义http传输过程中的结构体 用于json解析
*/

struct Topic {
    std::string emoji;
    std::string name;
    std::string enName;
};

struct TopicCategory {
    std::string tag;
    std::vector<Topic> topics;
};

struct Scene {
    std::string mainTask;
    std::string emoji;
    std::vector<std::string> subTasks;
    std::string botGender;
    std::string botRole;
    std::string userRole;
    std::string content;
};

struct HistoryItem {
    std::string speaker;
    std::string content;
};

struct ResultItem {
    std::vector<std::string> result;
    int score;
    std::vector<std::string> achievement;
    int index;
};

// 获取默认场景列表接口
struct GetDefaultTopicRequest {
    std::string appKey;
    std::string salt;
    std::string curtime;
    std::string signType;
    std::string sign;
    std::string q;
};

struct GetDefaultTopicResponse {
    std::string code;
    std::string msg;
    struct Data {
        std::vector<TopicCategory> topicList;
    } data;
};

// 场景生成接口
struct GenerateTopicRequest {
    std::string appKey;
    std::string salt;
    std::string curtime;
    std::string signType;
    std::string sign;
    std::string topic;
};

struct GenerateTopicResponse {
    std::string code;
    std::string msg;
    struct Data {
        std::string taskId;
        Scene scene;
    } data;
};

// 生成对话接口
struct GenerateDialogRequest {
    std::string appKey;
    std::string salt;
    std::string curtime;
    std::string signType;
    std::string sign;
    std::string taskId;
    Scene scene;
    std::string userLevel;
    std::vector<HistoryItem> history;
};

struct GenerateDialogResponse {
    std::string code;
    std::string msg;
    struct Data {
        int endCode;
        std::string taskId;
        std::vector<ResultItem> resultArr;
    } data;
};

// 生成推荐表达接口
struct GenerateRecommendationRequest {
    std::string appKey;
    std::string salt;
    std::string curtime;
    std::string signType;
    std::string sign;
    std::string taskId;
    Scene scene;
    std::string userLevel;
    std::vector<HistoryItem> history;
    std::vector<std::string> indexArr;
    std::string count;
};

struct GenerateRecommendationResponse {
    std::string code;
    std::string msg;
    struct Data {
        int endCode;
        std::string taskId;
        std::vector<ResultItem> resultArr;
    } data;
};

struct EnZhTranslationRequest {
    std::string q;         
    std::string appKey;    
    std::string salt;      
    std::string sign;      
    std::string signType;
    std::string curtime;   
    std::string from;
    std::string to;
};

struct EnZhTranslationResponse {
    std::string code;
    std::string msg;
    std::string query;
    std::vector<std::string> translation;
    std::string l;
    std::string dict;
    std::string webdict;
    
    std::string tSpeakUrl;
    std::string speakUrl;
};

struct AsrRequest {
    std::string q;          // Base64编码的音频
    std::string langType;   // 源语言类型
    std::string appKey;     // 应用ID
    std::string salt;       // UUID
    std::string curtime;    // 时间戳（秒）
    std::string sign;       // 签名
    std::string signType;   // 签名版本（v3）
    std::string format;      // 音频格式（wav）
    std::string rate;       // 采样率（16000）
    std::string channel;    // 声道数（1）
    std::string type;       // 上传类型（1）
};

struct AsrResponse {
    std::string code;               // 错误码
    std::vector<std::string> result;     // 识别结果
    std::string msg;                     // 错误信息（可选）
};

struct TtsRequest {
    std::string q;          // 待合成文本（UTF-8）
    std::string appKey;     // 应用ID
    std::string salt;       // UUID
    std::string sign;       // 签名
    std::string signType;   // 签名类型（v3）
    std::string curtime;    // 时间戳（秒）
    std::string format;     // 音频格式（mp3）
    std::string speed;      // 语速（0.5-2.0）
    std::string volume;     // 音量（0.50-5.00）
    std::string voiceName;  // 发言人名称
};

struct TtsResponse {
    std::string code;    // 错误码
    std::string msg;          // 错误信息
    std::vector<uint8_t> result; // 二进制音频数据
};
#endif