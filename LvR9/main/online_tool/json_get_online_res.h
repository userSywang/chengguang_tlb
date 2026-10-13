#ifndef JSON_GET_ONLINE_RES_H
#define JSON_GET_ONLINE_RES_H

#include <string>
#include <vector>
#include "../include/nlohmann/json.hpp"
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

struct ResNode {
    std::string path;      // 节点路径
    int mediaType;         // 媒体类型 0/1/2
    int nodeType;          // 节点类型 0/1
    std::string format;    // MIME类型
    int position;          // 索引位置
    std::string name;      // 显示名称
    std::string mediaUrl;  // 有效期1小时的媒体地址
    std::string coverUrl;
    std::string zhSubtitleUrl;
    std::string enSubtitleUrl;
    std::string mixedSubtitleUrl;
};

struct GetResRequest {
    std::string parentPath;
    std::string sign;
    std::string vendor;
    std::string product;
    int position;
    int pageSize;
};

struct GetResResponse {
    std::string code;      // 状态码
    std::string msg;   // 状态描述
    bool successful;       // 是否成功
    std::vector<ResNode> data;// 节点数组
};

struct DictionnaryRequest {
    std::string q;
    std::string langType;
    std::string appKey;
    std::string salt;
    std::string sign;
    std::string signType;
    std::string curtime;
    std::string docType;
    std::string dicts;
};

// 发音信息结构
struct PhoneticInfo {
    std::string phonetic;      // 音标
    std::string pronunciation; // 发音（中文用，例如“好”的拼音）
    std::string speech;        // 发音URL
};

// 基础释义条目
struct ExplainEntry {
    std::string pos;    // 词性
    std::string text;   // 文本（可能是短语或解释的原文）
    std::string trans;  // 翻译
};

// 单词形式变化
struct WordFormat {
    std::string name;  // 形式名称，例如：复数
    std::string value; // 变化后的形式
};

// 基础释义结构
struct DictionaryBasic {
    // 英文发音部分
    std::string phonetic;      // 通用音标（英文用例中直接出现）
    std::string usPhonetic;    // 美式音标（英文用例）
    std::string ukPhonetic;    // 英式音标（英文用例）
    std::string usSpeech;       // 美式发音URL
    std::string ukSpeech;       // 英式发音URL

    // 中文发音部分（多个发音）
    std::vector<PhoneticInfo> phonetics; // 发音信息数组

    // 其他
    std::vector<std::string> examType;   // 考试类型
    std::vector<ExplainEntry> explains;  // 解释列表
    std::vector<WordFormat> wordFormats; // 单词形式变化
};

// 网络释义结构
struct WebDefinition {
    std::string phrase;              // 词组
    std::vector<std::string> meanings; // 含义列表
};

// 近义词组
struct SynonymGroup {
    std::string pos;     // 词性
    std::vector<std::string> words; // 近义词列表
    std::string trans;   // 释义
};

// 短语结构（与网络释义类似，但用于专门的phrases字段）
struct PhraseEntry {
    std::string phrase;
    std::vector<std::string> meanings;
};

// 例句
struct ExampleSentence {
    std::string sentence;      // 原句
    std::string sentenceBold;  // 加粗版本（查询词被加粗）
    std::string translation;   // 翻译
};

// 词典结果
struct DictionaryEntry {
    std::vector<std::string> returnPhrase;   // 返回短语
    DictionaryBasic basic;                   // 基础释义
    std::vector<WebDefinition> web;          // 网络释义
    std::vector<SynonymGroup> synonyms;      // 近义词
    std::vector<PhraseEntry> phrases;        // 短语列表
    std::vector<ExampleSentence> sentenceSample; // 例句
    std::string dict;        // 词典deeplink
    std::string webDict;     // 网页词典链接
    bool isWord;             // 是否为单词
    std::string lang;        // 语言（如"en"或"zh-CHS"）
};

struct DictionaryResponse {
    std::string code;
    std::string requestId;
    std::string msg;
    std::map<std::string, DictionaryEntry> results; // 词典名称到结果的映射
};

//struct to json
std::string get_default_topic_request_to_json(const GetDefaultTopicRequest &req);
std::string generate_topic_request_to_json(const GenerateTopicRequest &req);
std::string generate_dialog_request_to_json(const GenerateDialogRequest &req);
std::string generate_recommendation_request_to_json(const GenerateRecommendationRequest &req);
std::string enzh_translation_request_to_json(const EnZhTranslationRequest &req);
std::string asr_request_to_json(const AsrRequest &req);
std::string tts_request_to_json(const TtsRequest &req);
std::string get_res_request_to_json(const GetResRequest &req);
std::string dictionary_request_to_json(const DictionnaryRequest &req);

//json to struct
GetDefaultTopicResponse parse_get_default_topic_response(const std::string &req);
GenerateTopicResponse parse_generate_topic_response(const std::string &req);
GenerateDialogResponse parse_generate_dialog_response(const std::string &req);
GenerateRecommendationResponse parse_generate_recommendation_response(const std::string &req);
EnZhTranslationResponse parse_enzh_translation_response(const std::string &req);
AsrResponse parse_asr_response(const std::string &req);
TtsResponse parse_tts_response(const std::string &req);
GetResResponse parse_get_res_response(const std::string &req);
DictionaryResponse parse_dictionary_response(const std::string &jsonStr);





#endif