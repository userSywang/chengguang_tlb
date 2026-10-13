#ifndef HTTP_GET_ONLINE_RES_H
#define HTTP_GET_ONLINE_RES_H
#include "json_get_online_res.h"

#define AI_TEACH_APP_KEY "0f471c7bfbcaa288"
#define AI_TEACH_APP_SECRET "bwFVR2L3SHqdcn4J13herRaqaYZLsXW9" 
#define AI_TEACH_API_GET_DEFAULT_TOPIC "https://openapi.youdao.com/ai_dialog/get_default_topic"
#define AI_TEACH_API_GENERATE_TOPIC "https://openapi.youdao.com/ai_dialog/generate_topic"
#define AI_TEACH_API_GENERATE_DIALOG "https://openapi.youdao.com/ai_dialog/generate_dialog"
#define AI_TEACH_API_GENERATE_RECOMMENDATION "https://openapi.youdao.com/ai_dialog/generate_recommendation"

#define TRANSLATE_APP_KEY "0f471c7bfbcaa288"
#define TRANSLATE_APP_SECRET "bwFVR2L3SHqdcn4J13herRaqaYZLsXW9"
#define TRANSLATE_APP_URL "https://openapi.youdao.com/api"

#define ASR_APP_KEY "0f471c7bfbcaa288"
#define ASR_APP_SECRET "bwFVR2L3SHqdcn4J13herRaqaYZLsXW9"
#define ASR_APP_URL "https://openapi.youdao.com/asrapi"

#define TTS_APP_KEY "0f471c7bfbcaa288"
#define TTS_APP_SECRET "bwFVR2L3SHqdcn4J13herRaqaYZLsXW9"
#define TTS_APP_URL "https://openapi.youdao.com/ttsapi"

#define GET_ONLINE_RES_KEY "b15ac13fca3c6eab7d53b85e8732721d901c25ea9fe92197e1fd38d0818c74cd"
#define GET_ONLINE_RES_URL "https://openapi.youdao.com/student-card/data/catalog"
#define GET_ONLINE_ALL_RES_URL "https://openapi.youdao.com/student-card/data/flat-catalog"

#define DICTIONNARY_KEY "0f471c7bfbcaa288"
#define DICTIONNARY_SECRET "bwFVR2L3SHqdcn4J13herRaqaYZLsXW9"
#define DICTIONNARY_URL "https://openapi.youdao.com/v2/dict"

// struct GetDefaultTopicRequest;
// struct GetDefaultTopicResponse;

//send request function
GetDefaultTopicResponse send_get_default_topic_request(const GetDefaultTopicRequest& req);
GenerateDialogResponse send_generate_dialog_request(const GenerateDialogRequest& req);
GenerateTopicResponse send_generate_topic_request(const GenerateTopicRequest& req);
GenerateRecommendationResponse send_generate_recommendation_request(const GenerateRecommendationRequest& req);
EnZhTranslationResponse send_enzh_translation_request(const EnZhTranslationRequest& req);
AsrResponse send_Asr_request(const AsrRequest& req);
TtsResponse send_Tts_request(const TtsRequest& req);
GetResResponse send_GetRes_request(const GetResRequest& req);
GetResResponse send_GetAllMediaRes_request(const GetResRequest& req);
DictionaryResponse send_Dictionary_request(const DictionnaryRequest& req);



void get_online_res_req(std::string path,int pageSize,std::unique_ptr<GetResResponse>& res,
    std::function<void()> callback);
void get_online_media_res_req(std::string path,int pageSize,std::unique_ptr<GetResResponse>& res,
    std::function<void()> callback);


#endif