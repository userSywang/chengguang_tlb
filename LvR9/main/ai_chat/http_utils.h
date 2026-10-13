#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

extern "C"{
    #include "../include/curl/curl.h"
}
#include "json_parse_struct.h"

#define AI_TEACH_APP_KEY "12420cceb4087e81"
#define AI_TEACH_APP_SECRET "cPzefBSJ2hnA8hWr6F5VY6S0N8KqhtfL" 
#define AI_TEACH_API_GET_DEFAULT_TOPIC "https://openapi.youdao.com/ai_dialog/get_default_topic"
#define AI_TEACH_API_GENERATE_TOPIC "https://openapi.youdao.com/ai_dialog/generate_topic"
#define AI_TEACH_API_GENERATE_DIALOG "https://openapi.youdao.com/ai_dialog/generate_dialog"
#define AI_TEACH_API_GENERATE_RECOMMENDATION "https://openapi.youdao.com/ai_dialog/generate_recommendation"


#define TRANSLATE_APP_KEY "1fbac303b50b40f3"
#define TRANSLATE_APP_SECRET "RYZbZx7w0jHPUX8qrnEsnDJhzPWD9EDs"
#define TRANSLATE_APP_URL "https://openapi.youdao.com/api"

#define ASR_APP_KEY "33d2faabb95e86fd"
#define ASR_APP_SECRET "6jIYlO5ezUsZJQCY3vyPrJH9tWeG1bSa"
#define ASR_APP_URL "https://openapi.youdao.com/asrapi"

#define TTS_APP_KEY "3acab205c212f5bd"
#define TTS_APP_SECRET "THFKBGsGKMR4EI2j3HdJzoQcs7AIzeSF"
#define TTS_APP_URL "https://openapi.youdao.com/ttsapi"

struct ResponseData {
    std::string content;
    long http_code = 0;
    std::string content_type;
};



GetDefaultTopicResponse send_get_default_topic_request(const GetDefaultTopicRequest& req);
GenerateDialogResponse send_generate_dialog_request(const GenerateDialogRequest& req);
GenerateTopicResponse send_generate_topic_request(const GenerateTopicRequest& req);
GenerateRecommendationResponse send_generate_recommendation_request(const GenerateRecommendationRequest& req);
EnZhTranslationResponse send_enzh_translation_request(const EnZhTranslationRequest& req);
AsrResponse send_Asr_request(const AsrRequest& req);
TtsResponse send_Tts_request(const TtsRequest& req);




#endif