#include "json_utils.h"
#include <iostream>

using json = nlohmann::json;

//----------------------- Request序列化 -----------------------
std::string get_default_topic_request_to_json(const GetDefaultTopicRequest &req) {
    json j;
    j["appKey"] = req.appKey;
    j["salt"] = req.salt;
    j["curtime"] = req.curtime;
    j["signType"] = req.signType;
    j["sign"] = req.sign;
    j["q"] = req.q;  // 固定值"topics"
    return j.dump();
}

std::string generate_topic_request_to_json(const GenerateTopicRequest &req) {
    json j;
    j["appKey"] = req.appKey;
    j["salt"] = req.salt;
    j["curtime"] = req.curtime;
    j["signType"] = req.signType;
    j["sign"] = req.sign;
    j["topic"] = req.topic;
    return j.dump();
}

std::string generate_dialog_request_to_json(const GenerateDialogRequest &req) {
    json j;
    j["appKey"] = req.appKey;
    j["salt"] = req.salt;
    j["curtime"] = req.curtime;
    j["signType"] = req.signType;
    j["sign"] = req.sign;
    j["taskId"] = req.taskId;
    
    // Scene对象
    json scene;
    scene["mainTask"] = req.scene.mainTask;
    scene["emoji"] = req.scene.emoji;
    scene["subTasks"] = req.scene.subTasks;
    scene["botGender"] = req.scene.botGender;
    scene["botRole"] = req.scene.botRole;
    scene["userRole"] = req.scene.userRole;
    scene["content"] = req.scene.content;
    j["scene"] = scene;
    
    j["userLevel"] = req.userLevel;
    
    // History数组
    json history = json::array();
    for (const auto& item : req.history) {
        json his;
        his["speaker"] = item.speaker;
        his["content"] = item.content;
        history.push_back(his);
    }
    j["history"] = history;
    
    return j.dump();
}

std::string generate_recommendation_request_to_json(const GenerateRecommendationRequest &req) {
    json j;
    j["appKey"] = req.appKey;
    j["salt"] = req.salt;
    j["curtime"] = req.curtime;
    j["signType"] = req.signType;
    j["sign"] = req.sign;
    j["taskId"] = req.taskId;
    json scene;
    scene["mainTask"] = req.scene.mainTask;
    scene["emoji"] = req.scene.emoji;
    scene["subTasks"] = req.scene.subTasks;
    scene["botGender"] = req.scene.botGender;
    scene["botRole"] = req.scene.botRole;
    scene["userRole"] = req.scene.userRole;
    scene["content"] = req.scene.content;
    j["scene"] = scene;
    j["userLevel"] = req.userLevel;
    json history = json::array();
    for (const auto& item : req.history) {
        json his;
        his["speaker"] = item.speaker;
        his["content"] = item.content;
        history.push_back(his);
    }
    j["history"] = history;
    j["indexArr"] = req.indexArr;
    j["count"] = req.count;
    return j.dump();
}
std::string enzh_translation_request_to_json(const EnZhTranslationRequest &req) {
    std::string jsonStr;
    jsonStr += "q=" + req.q + "&";
    jsonStr += "appKey=" + req.appKey + "&";
    jsonStr += "salt=" + req.salt + "&";
    jsonStr += "sign=" + req.sign + "&";
    jsonStr += "signType=" + req.signType + "&";
    jsonStr += "curtime=" + req.curtime + "&";
    jsonStr += "from=" + req.from + "&";
    jsonStr += "to=" + req.to;
    return jsonStr;
}
std::string asr_request_to_json(const AsrRequest &req){
    std::string jsonStr;
    jsonStr += "q=" + req.q + "&";
    jsonStr += "langType=" + req.langType + "&";
    jsonStr += "appKey=" + req.appKey + "&";
    jsonStr += "salt=" + req.salt + "&";
    jsonStr += "sign=" + req.sign + "&";
    jsonStr += "signType=" + req.signType + "&";
    jsonStr += "curtime=" + req.curtime + "&";
    jsonStr += "format=" + req.format + "&";
    jsonStr += "rate=" + req.rate + "&";
    jsonStr += "channel=" + req.channel + "&";
    jsonStr += "type=" + req.type;
    return jsonStr;
}
std::string tts_request_to_json(const TtsRequest &req){
    std::string jsonStr;
    jsonStr += "q=" + req.q + "&";
    jsonStr += "appKey=" + req.appKey + "&";
    jsonStr += "salt=" + req.salt + "&";
    jsonStr += "sign=" + req.sign + "&";
    jsonStr += "signType=" + req.signType + "&";
    jsonStr += "curtime=" + req.curtime + "&";
    jsonStr += "format=" + req.format + "&";
    jsonStr += "speed=" + req.speed + "&";
    jsonStr += "volume=" + req.volume + "&";
    jsonStr += "voiceName=" + req.voiceName;
    return jsonStr;
}

//----------------------- Response解析 -----------------------
GetDefaultTopicResponse parse_get_default_topic_response(const std::string &jsonStr) {
    GetDefaultTopicResponse res;
    try {
        json j = json::parse(jsonStr);
        res.code = j.value("code", "");
        res.msg = j.value("msg", "");
        
        if (j.contains("data")) {
            auto& data = j["data"];
            if (data.contains("topicList")) {
                for (auto& category : data["topicList"]) {
                    TopicCategory tc;
                    tc.tag = category.value("tag", "");
                    for (auto& topic : category["topics"]) {
                        Topic t;
                        t.emoji = topic.value("emoji", "");
                        t.name = topic.value("name", "");
                        t.enName = topic.value("enName", "");
                        tc.topics.push_back(t);
                    }
                    res.data.topicList.push_back(tc);
                }
            }
        }
    } catch (const json::exception& e) {
        printf("JSON_PARSE1: %s", e.what());
        res.code = "PARSE_ERROR";
    }
    return res;
}

GenerateTopicResponse parse_generate_topic_response(const std::string &jsonStr) {
    GenerateTopicResponse res;
    try {
        json j = json::parse(jsonStr);
        res.code = j.value("code", "");
        res.msg = j.value("msg", "");
        
        if (j.contains("data")) {
            auto& data = j["data"];
            res.data.taskId = data.value("taskId", "");
            if (data.contains("scene") && data["scene"].is_object()) {
                auto& scene = data["scene"];
                res.data.scene.mainTask = scene.value("mainTask", "");
                res.data.scene.emoji = scene.value("emoji", "");
                if (scene.contains("subTasks") && scene["subTasks"].is_array()) {
                    for (auto& task : scene["subTasks"]) {
                        res.data.scene.subTasks.push_back(task.get<std::string>());
                    }
                }
                res.data.scene.botGender = scene.value("botGender", "Female");
                res.data.scene.botRole = scene.value("botRole", "");
                res.data.scene.userRole = scene.value("userRole", "");
                res.data.scene.content = scene.value("content", "");
            }
        }
    } catch (const json::type_error& e) {
        printf("TYPE_ERROR2: %s\n", e.what());
        res.code = "TYPE_ERROR";
    } catch (const json::out_of_range& e) {
        printf("MISSING_FIELD2: %s\n", e.what());
        res.code = "MISSING_FIELD";
    }
    return res;
}

GenerateDialogResponse parse_generate_dialog_response(const std::string &jsonStr) {
    GenerateDialogResponse res;
    try {
        json j = json::parse(jsonStr);
        res.code = j.value("code", "");
        res.msg = j.value("msg", "");
        
        if (j.contains("data")) {
            auto& data = j["data"];
            res.data.endCode = data.value("endCode", 0);
            res.data.taskId = data.value("taskId", "");
            
            for (auto& result : data["resultArr"]) {
                ResultItem item;
                for (auto& str : result["result"]) {
                    item.result.push_back(str.get<std::string>());
                }
                item.score = result.value("score", 0);
                for (auto& ach : result["achievement"]) {
                    item.achievement.push_back(ach.get<std::string>());
                }
                item.index = result.value("index", -1);
                res.data.resultArr.push_back(item);
            }
        }
    } catch (const json::type_error& e) {
        printf("TYPE_ERROR3: %s\n", e.what());
        res.code = "TYPE_ERROR";
    } catch (const json::out_of_range& e) {
        printf("MISSING_FIELD3: %s\n", e.what());
        res.code = "MISSING_FIELD";
    }
    return res;
}

GenerateRecommendationResponse parse_generate_recommendation_response(const std::string &jsonStr) {
    GenerateRecommendationResponse res;
    try {
        json j = json::parse(jsonStr);
        res.code = j.value("code", "");
        res.msg = j.value("msg", "");
        
        if (j.contains("data")) {
            auto& data = j["data"];
            res.data.endCode = data.value("endCode", 0);
            res.data.taskId = data.value("taskId", "");
            
            for (auto& result : data["resultArr"]) {
                ResultItem item;
                for (auto& str : result["result"]) {
                    item.result.push_back(str.get<std::string>());
                }
                item.score = result.value("score", 0);
                for (auto& ach : result["achievement"]) {
                    item.achievement.push_back(ach.get<std::string>());
                }
                item.index = result.value("index", -1);
                res.data.resultArr.push_back(item);
            }
        }
    } catch (const json::type_error& e) {
        printf("TYPE_ERROR4: %s\n", e.what());
        res.code = "TYPE_ERROR";
    } catch (const json::out_of_range& e) {
        printf("MISSING_FIELD4: %s\n", e.what());
        res.code = "MISSING_FIELD";
    }
    return res;
}

EnZhTranslationResponse parse_enzh_translation_response(const std::string &req) {
    EnZhTranslationResponse res;
    
    try {
        json j = json::parse(req);
        std::cout << "Res JSON: " << j.dump() << std::endl;  // 改用标准输出
        res.code = j.value("errorCode", "");
        res.msg = "";
        res.query = j.value("query", "");
        
        if (j.contains("translation") && j["translation"].is_array()) {
            for (auto& item : j["translation"]) {
                res.translation.push_back(item.get<std::string>());
            }
        }
        res.l = j.value("l", "");
        if (j.contains("dict") && j["dict"].is_object()) {
            res.dict = j["dict"].value("url", "");
        }
        
        // 修正webdict字段处理
        if (j.contains("webdict") && j["webdict"].is_object()) {
            res.webdict = j["webdict"].value("url", "");
        }
        res.tSpeakUrl = j.value("tSpeakUrl", "");
        res.speakUrl = j.value("speakUrl", "");
        
    } catch (const json::type_error& e) {
        printf("TRANSLATION_TYPE_ERROR: %s\n", e.what());
        res.code = "PARSE_ERROR";
    } catch (const json::out_of_range& e) {
        printf("TRANSLATION_MISSING_FIELD: %s\n", e.what());
        res.code = "MISSING_FIELD";
    }
    return res;
}
AsrResponse parse_asr_response(const std::string &jsonStr) {
    AsrResponse res;
    try {
        json j = json::parse(jsonStr);
        res.code = j.value("errorCode", "");
        res.msg = "" ;
        
        if (j.contains("result") && j["result"].is_array()) {
            for (auto& item : j["result"]) {
                res.result.push_back(item.get<std::string>());
            }
        }
    } catch (const json::exception& e) {
        printf("ASR_PARSE_ERROR: %s\n", e.what());
        res.code = "PARSE_ERROR";
    }
    return res;
}
TtsResponse parse_tts_response(const std::string &jsonStr) {
    TtsResponse res;
    try {
        json j = json::parse(jsonStr);
        res.code = j.value("errorCode", "");
        res.msg = "" ;
    } catch (const json::exception& e) {
        printf("TTS_PARSE_ERROR: %s\n", e.what());
        res.code = "PARSE_ERROR";
    }
    return res;
}