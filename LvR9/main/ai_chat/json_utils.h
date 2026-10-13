#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "../include/nlohmann/json.hpp"
#include "json_parse_struct.h"



//struct to json
std::string get_default_topic_request_to_json(const GetDefaultTopicRequest &req);
std::string generate_topic_request_to_json(const GenerateTopicRequest &req);
std::string generate_dialog_request_to_json(const GenerateDialogRequest &req);
std::string generate_recommendation_request_to_json(const GenerateRecommendationRequest &req);
std::string enzh_translation_request_to_json(const EnZhTranslationRequest &req);
std::string asr_request_to_json(const AsrRequest &req);
std::string tts_request_to_json(const TtsRequest &req);

//json to struct
GetDefaultTopicResponse parse_get_default_topic_response(const std::string &req);
GenerateTopicResponse parse_generate_topic_response(const std::string &req);
GenerateDialogResponse parse_generate_dialog_response(const std::string &req);
GenerateRecommendationResponse parse_generate_recommendation_response(const std::string &req);
EnZhTranslationResponse parse_enzh_translation_response(const std::string &req);
AsrResponse parse_asr_response(const std::string &req);
TtsResponse parse_tts_response(const std::string &req);
#endif // JSON_UTILS_H