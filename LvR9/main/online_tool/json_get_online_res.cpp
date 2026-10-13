#include <iostream>
#include "json_get_online_res.h"

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

std::string get_res_request_to_json(const GetResRequest &req){
    std::string jsonStr;
    jsonStr += "parentPath=" + req.parentPath + "&";
    jsonStr += "sign=" + req.sign + "&";
    jsonStr += "vendor=" + req.vendor + "&";
    jsonStr += "product=" + req.product + "&";
    jsonStr += "position=" + std::to_string(req.position) + "&";
    jsonStr += "pageSize=" + std::to_string(req.pageSize);
    return jsonStr;
}

std::string dictionary_request_to_json(const DictionnaryRequest &req){
    std::string jsonStr;
    jsonStr += "q=" + req.q + "&";
    jsonStr += "langType=" + req.langType + "&";
    jsonStr += "appKey=" + req.appKey + "&";
    jsonStr += "dicts=" + req.dicts + "&";
    jsonStr += "salt=" + req.salt + "&";
    jsonStr += "sign=" + req.sign + "&";
    jsonStr += "signType=" + req.signType + "&";
    jsonStr += "curtime=" + req.curtime + "&";
    jsonStr += "docType=" + req.docType;
    return jsonStr;
}

//----------------------- Response解析 -----------------------
GetDefaultTopicResponse parse_get_default_topic_response(const std::string &jsonStr) {
    GetDefaultTopicResponse res;
    try {
        json j = json::parse(jsonStr);
        
        // 使用安全字段访问
        if(j.contains("code") && !j["code"].is_null()) 
            res.code = j["code"].get<std::string>();
        if(j.contains("msg") && !j["msg"].is_null())
            res.msg = j["msg"].get<std::string>();
        
        if (j.contains("data") && j["data"].is_object()) {
            auto& data = j["data"];
            if (data.contains("topicList") && data["topicList"].is_array()) {
                for (auto& category : data["topicList"]) {
                    TopicCategory tc;
                    if(category.contains("tag") && category["tag"].is_string())
                        tc.tag = category["tag"].get<std::string>();
                    
                    if(category.contains("topics") && category["topics"].is_array()) {
                        for (auto& topic : category["topics"]) {
                            Topic t;
                            if(topic.contains("emoji") && topic["emoji"].is_string())
                                t.emoji = topic["emoji"].get<std::string>();
                            if(topic.contains("name") && topic["name"].is_string())
                                t.name = topic["name"].get<std::string>();
                            if(topic.contains("enName") && topic["enName"].is_string())
                                t.enName = topic["enName"].get<std::string>();
                            tc.topics.push_back(t);
                        }
                    }
                    res.data.topicList.push_back(tc);
                }
            }
        }
    } catch (const json::exception& e) {
        printf("JSON_PARSE1: %s", e.what());
        res.code = "PARSE_ERROR";
        res.msg = "Invalid response format";
    }
    return res;
}

GenerateTopicResponse parse_generate_topic_response(const std::string &jsonStr) {
    GenerateTopicResponse res;
    try {
        json j = json::parse(jsonStr);
        
        // 安全访问code字段
        if(j.contains("code") && j["code"].is_string()) {
            res.code = j["code"].get<std::string>();
        } else {
            res.code = "INVALID_CODE";
        }
        
        // 安全访问msg字段
        if(j.contains("msg") && j["msg"].is_string()) {
            res.msg = j["msg"].get<std::string>();
        }

        if (j.contains("data") && j["data"].is_object()) {
            auto& data = j["data"];
            
            // 安全访问taskId字段
            if(data.contains("taskId") && data["taskId"].is_string()) {
                res.data.taskId = data["taskId"].get<std::string>();
            }

            if (data.contains("scene") && data["scene"].is_object()) {
                auto& scene = data["scene"];
                
                // 安全访问scene对象字段
                if(scene.contains("mainTask") && scene["mainTask"].is_string()) {
                    res.data.scene.mainTask = scene["mainTask"].get<std::string>();
                }
                
                if(scene.contains("emoji") && scene["emoji"].is_string()) {
                    res.data.scene.emoji = scene["emoji"].get<std::string>();
                }

                if (scene.contains("subTasks") && scene["subTasks"].is_array()) {
                    for (auto& task : scene["subTasks"]) {
                        if(task.is_string()) {
                            res.data.scene.subTasks.push_back(task.get<std::string>());
                        }
                    }
                }
                
                // 使用带默认值的字段访问
                res.data.scene.botGender = scene.value("botGender", "Female");
                res.data.scene.botRole = scene.value("botRole", "");
                res.data.scene.userRole = scene.value("userRole", "");
                res.data.scene.content = scene.value("content", "");
            }
        }
    } 
    catch (const json::exception& e) {
        printf("JSON_PARSE_ERROR: %s\n", e.what());
        res.code = "PARSE_ERROR";
        res.msg = std::string("解析错误: ") + e.what();
    }
    return res;
}

GenerateDialogResponse parse_generate_dialog_response(const std::string &jsonStr) {
    GenerateDialogResponse res;
    try {
        json j = json::parse(jsonStr);
        
        // 安全访问顶层字段
        if(j.contains("code") && j["code"].is_string()) {
            res.code = j["code"].get<std::string>();
        } else {
            res.code = "INVALID_CODE";
        }
        
        if(j.contains("msg") && j["msg"].is_string()) {
            res.msg = j["msg"].get<std::string>();
        }
        
        if (j.contains("data") && j["data"].is_object()) {
            auto& data = j["data"];
            
            // 安全访问data字段
            if(data.contains("endCode") && data["endCode"].is_number_integer()) {
                res.data.endCode = data["endCode"].get<int>();
            }
            
            if(data.contains("taskId") && data["taskId"].is_string()) {
                res.data.taskId = data["taskId"].get<std::string>();
            }
            
            if (data.contains("resultArr") && data["resultArr"].is_array()) {
                for (auto& result : data["resultArr"]) {
                    ResultItem item;
                    
                    // 安全访问result数组
                    if(result.contains("result") && result["result"].is_array()) {
                        for (auto& str : result["result"]) {
                            if(str.is_string()) {
                                item.result.push_back(str.get<std::string>());
                            }
                        }
                    }
                    
                    if(result.contains("score") && result["score"].is_number_integer()) {
                        item.score = result["score"].get<int>();
                    }
                    
                    if(result.contains("achievement") && result["achievement"].is_array()) {
                        for (auto& ach : result["achievement"]) {
                            if(ach.is_string()) {
                                item.achievement.push_back(ach.get<std::string>());
                            }
                        }
                    }
                    
                    if(result.contains("index") && result["index"].is_number_integer()) {
                        item.index = result["index"].get<int>();
                    }
                    
                    res.data.resultArr.push_back(item);
                }
            }
        }
    } catch (const json::exception& e) {
        printf("JSON_PARSE_ERROR3: %s\n", e.what());
        res.code = "PARSE_ERROR";
        res.msg = std::string("解析错误: ") + e.what();
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

GetResResponse parse_get_res_response(const std::string &jsonStr) {
    GetResResponse res;
    try {
        json j = json::parse(jsonStr);
        res.code = j.value("code", "");
        res.msg = j.value("msg", "");
        res.successful = j.value("successful", false);

        if (j.contains("data") && j["data"].is_array()) {
            for (auto& item : j["data"]) {
                ResNode node;
                node.path = item.value("path", "");
                node.nodeType = item.value("nodeType", 0);
                node.mediaType = item.value("mediaType", 0);
                if(node.nodeType == 1){
                    if(!item["format"].is_null()) node.format = item.value("format", "");
                    if(!item["mediaUrl"].is_null()) node.mediaUrl = item.value("mediaUrl", "");
                    if(!item["zhSubtitleUrl"].is_null()) node.zhSubtitleUrl = item.value("zhSubtitleUrl", "");
                    if(!item["enSubtitleUrl"].is_null()) node.enSubtitleUrl = item.value("enSubtitleUrl", "");
                    if(!item["mixedSubtitleUrl"].is_null()) node.mixedSubtitleUrl = item.value("mixedSubtitleUrl", "");
                }else{
                    if(!item["coverUrl"].is_null()) node.coverUrl = item.value("coverUrl", "");
                }
                node.position = item.value("position", 0);
                node.name = item.value("name", "");
                res.data.push_back(node);
            }
        }
    } catch (const json::type_error& e) {
        printf("TYPE_ERROR: %s\n", e.what());
        res.code = "PARSE_ERROR";
    } catch (const json::out_of_range& e) {
        printf("MISSING_FIELD: %s\n", e.what());
        res.code = "MISSING_FIELD";
    }
    return res;
}

DictionaryResponse parse_dictionary_response(const std::string &jsonStr) {
    DictionaryResponse res;
    try {
        std::cout << "JSON string: " << jsonStr << std::endl;  // 打印原始JSON字符串
        json j = json::parse(jsonStr);
        
        // 解析顶级字段
        res.code = j.value("errorCode", "");
        res.requestId = j.value("requestId", "");
        res.msg = j.value("msg", "");
        
        // 解析result数组
        if (j.contains("result") && j["result"].is_array()) {
            for (auto& item : j["result"]) {
                // 每个item是一个词典结果对象 (如ec、ce等)
                for (auto& dictEntry : item.items()) {
                    const std::string dictName = dictEntry.key(); // 词典名称 (如"ec")
                    json dictValue = dictEntry.value();
                    
                    DictionaryEntry result;
                    
                    // 解析词典结果字段
                    result.returnPhrase = dictValue["returnPhrase"].get<std::vector<std::string>>();
                    result.dict = dictValue.value("dict", "");
                    result.webDict = dictValue.value("webDict", "");
                    result.isWord = dictValue.value("isWord", false);
                    result.lang = dictValue.value("lang", "");
                    
                    // 解析basic对象
                    if (dictValue.contains("basic") && dictValue["basic"].is_object()) {
                        json basic = dictValue["basic"];
                        // 处理音标信息
                        if (basic.contains("phonetics") && basic["phonetics"].is_array()) {
                            for (auto& ph : basic["phonetics"]) {
                                PhoneticInfo pi;
                                pi.phonetic = ph.value("phonetic", "");
                                pi.pronunciation = ph.value("pronunciation", "");
                                pi.speech = ph.value("speech", "");
                                result.basic.phonetics.push_back(pi);
                            }
                        } else {
                            result.basic.phonetic = basic.value("phonetic", "");
                            result.basic.usPhonetic = basic.value("usPhonetic", "");
                            result.basic.ukPhonetic = basic.value("ukPhonetic", "");
                            result.basic.usSpeech = basic.value("usSpeech", "");
                            result.basic.ukSpeech = basic.value("ukSpeech", "");
                        }
                        
                        // 解析解释列表
                        if (basic.contains("explains") && basic["explains"].is_array()) {
                            for (auto& exp : basic["explains"]) {
                                ExplainEntry ee;
                                ee.pos = exp.value("pos", "");
                                ee.text = exp.value("text", "");
                                ee.trans = exp.value("trans", "");
                                result.basic.explains.push_back(ee);
                            }
                        }
                        
                        // 解析词形变化
                        if (basic.contains("wordFormats") && basic["wordFormats"].is_array()) {
                            for (auto& wf : basic["wordFormats"]) {
                                WordFormat format;
                                format.name = wf.value("name", "");
                                format.value = wf.value("value", "");
                                result.basic.wordFormats.push_back(format);
                            }
                        }
                        
                        // 解析考试类型
                        if (basic.contains("examType") && basic["examType"].is_array()) {
                            result.basic.examType = basic["examType"].get<std::vector<std::string>>();
                        }
                    }
                    
                    // 解析web释义
                    if (dictValue.contains("web") && dictValue["web"].is_array()) {
                        for (auto& webItem : dictValue["web"]) {
                            WebDefinition wd;
                            wd.phrase = webItem.value("phrase", "");
                            if (webItem.contains("meanings") && webItem["meanings"].is_array()) {
                                wd.meanings = webItem["meanings"].get<std::vector<std::string>>();
                            }
                            result.web.push_back(wd);
                        }
                    }
                    
                    // 解析近义词
                    if (dictValue.contains("synonyms") && dictValue["synonyms"].is_array()) {
                        for (auto& synItem : dictValue["synonyms"]) {
                            SynonymGroup sg;
                            sg.pos = synItem.value("pos", "");
                            if (synItem.contains("words") && synItem["words"].is_array()) {
                                sg.words = synItem["words"].get<std::vector<std::string>>();
                            }
                            sg.trans = synItem.value("trans", "");
                            result.synonyms.push_back(sg);
                        }
                    }
                    
                    // 解析短语
                    if (dictValue.contains("phrases") && dictValue["phrases"].is_array()) {
                        for (auto& phraseItem : dictValue["phrases"]) {
                            PhraseEntry pe;
                            pe.phrase = phraseItem.value("phrase", "");
                            if (phraseItem.contains("meanings") && phraseItem["meanings"].is_array()) {
                                pe.meanings = phraseItem["meanings"].get<std::vector<std::string>>();
                            }
                            result.phrases.push_back(pe);
                        }
                    }
                    
                    // 解析例句
                    if (dictValue.contains("sentenceSample") && dictValue["sentenceSample"].is_array()) {
                        for (auto& sentItem : dictValue["sentenceSample"]) {
                            ExampleSentence es;
                            es.sentence = sentItem.value("sentence", "");
                            es.sentenceBold = sentItem.value("sentenceBold", "");
                            es.translation = sentItem.value("translation", "");
                            result.sentenceSample.push_back(es);
                        }
                    }
                    
                    // 将词典结果添加到响应中
                    res.results[dictName] = result;
                }
            }
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        res.code = "PARSE_ERROR";
    } catch (const json::type_error& e) {
        std::cerr << "JSON type error: " << e.what() << std::endl;
        res.code = "TYPE_ERROR";
    } catch (const json::out_of_range& e) {
        std::cerr << "JSON missing field: " << e.what() << std::endl;
        res.code = "MISSING_FIELD";
    } catch (...) {
        std::cerr << "Unknown error during JSON parsing" << std::endl;
        res.code = "UNKNOWN_ERROR";
    }
    
    return res;
}