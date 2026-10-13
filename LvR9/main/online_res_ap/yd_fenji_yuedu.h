#ifndef YD_FENJI_YUEDU_H
#define YD_FENJI_YUEDU_H

#include"online_res.h"


#define YD_FENJI_MAINWIN 0
#define YD_FENJI_CONTENTWIN 1
#define YD_FENJI_DISWIN 2

#define YD_FENJI_PATH "/FJYD"

class YD_FenjiYuedu : public ApBase{
    public:
    YD_FenjiYuedu():
        ApBase(){};
    ~YD_FenjiYuedu(){
        AP_MainWin_deinit();
    };
    int AP_MainWin_init() override;
    int AP_ContentWin_init();
    int AP_DisWin_init();

    int AP_MainWin_handler(lv_event_code_t msg, int param) override;
    void Ap_MainWin_clean_res() override;
    int AP_MainWin_deinit() override;
    void Ap_MainWin_del() override;
    int get_cur_win(){
        return mwin_CurWin;
    }
private:

};




#endif