#include <lvgl/lvgl.h>

extern const lv_img_dsc_t MMN_1;
extern const lv_img_dsc_t MMN_2;
extern const lv_img_dsc_t MMN_3;
extern const lv_img_dsc_t MMN_4;
extern const lv_img_dsc_t MMN_5;
extern const lv_img_dsc_t MMN_battery_charging_0;
extern const lv_img_dsc_t MMN_battery_charging_1;
extern const lv_img_dsc_t MMN_battery_charging_2;
extern const lv_img_dsc_t MMN_battery_charging_3;
extern const lv_img_dsc_t MMN_battery_charging_4;
extern const lv_img_dsc_t MMN_battery_charging_5;
extern const lv_img_dsc_t MMN_battery_L_0;
extern const lv_img_dsc_t MMN_battery_L_1;
extern const lv_img_dsc_t MMN_battery_L_2;
extern const lv_img_dsc_t MMN_battery_L_3;
extern const lv_img_dsc_t MMN_battery_L_4;
extern const lv_img_dsc_t MMN_wifi_L1;
extern const lv_img_dsc_t MMN_wifi_L2;
extern const lv_img_dsc_t MMN_wifi_L3;
extern const lv_img_dsc_t simDis;

const lv_img_dsc_t* g_signal_tlb[] = { //4g信号ui
    &MMN_1,
    &MMN_2,
    &MMN_3,
    &MMN_4,
    &MMN_5
};
const lv_img_dsc_t* g_battery_charging_tlb[] = {  //电池充电ui     //&MMN_battery_charging_5
    &MMN_battery_charging_0,
    &MMN_battery_charging_1,
    &MMN_battery_charging_2,
    &MMN_battery_charging_3,
    &MMN_battery_charging_4
};
const lv_img_dsc_t* g_battery_L_tlb[] = { //电池放电ui
    &MMN_battery_L_0,
    &MMN_battery_L_1,
    &MMN_battery_L_2,
    &MMN_battery_L_3,
    &MMN_battery_L_4
};
const lv_img_dsc_t* g_wifi_tlb[] = {//wifiui
    &MMN_wifi_L1,
    &MMN_wifi_L2,
    &MMN_wifi_L3
};