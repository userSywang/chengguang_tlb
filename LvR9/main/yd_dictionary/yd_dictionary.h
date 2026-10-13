#ifndef YD_DICTIONARY_H
#define YD_DICTIONARY_H

#include "../online_res_ap/online_res.h"
#include <list>

#ifdef __cplusplus
extern "C" {
#endif
#include <lvgl/lvgl.h>
extern int YdDictionary_ap_init(void);
extern int YdDictionary_ap_handler(lv_event_code_t msg, int param);
#ifdef __cplusplus
}
#endif


#define		YdDictionary_MAIN			    0
#define 	YdDictionary_SEARCH	            1		
#define 	YdDictionary_CONTENT			2
#define     YdDictionary_FOLLOW             3
#define     YdDictionary_DIS                4

class YdDictionary : public ApBase{
public:
    YdDictionary():ApBase(){};
    ~YdDictionary(){
        AP_MainWin_deinit();
    };
    int AP_MainWin_init() override;
    int AP_ContentWin_init();
    int AP_DisWin_init();
    int AP_SearchWin_init();
    int AP_FollowWin_init();

    int AP_MainWin_handler(lv_event_code_t msg, int param) override;
    void Ap_MainWin_clean_res() override;
    int AP_MainWin_deinit() override;
    void Ap_MainWin_del() override;

    void Ap_send_dictionnary_request(std::string& word,bool is_en,std::function<void()> callback);


    void Ap_set_search_word(std::string& word);
    void Ap_tts_gen(const std::string& q,const std::string& language,
        std::function<void()> callback);


    std::unique_ptr<DictionaryResponse> g_DictionaryRes = nullptr;
    std::unique_ptr<TtsResponse> g_TtsRes = nullptr;
    bool is_en = true;
private:
    std::string m_search_word;
};

extern const lv_img_dsc_t serch_icon;
extern const lv_img_dsc_t youdao_logo;
extern const lv_img_dsc_t record_icon;
extern const lv_img_dsc_t speak_out_icon;
extern const lv_img_dsc_t speak_out_icon_half;
extern const lv_img_dsc_t speak_out_big;
#endif