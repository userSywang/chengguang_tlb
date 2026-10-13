set(LV_DEMO_DIR ../components/lvgl__lvgl/demos)
set(TEST_DEMO_DIR ./test_demo)

file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)
file(GLOB_RECURSE IMAGE_SOURCES images/*.c)
file(GLOB_RECURSE FONT_SOURCES font/*.c)
file(GLOB_RECURSE EX_WIDGET ex_widget/*.c)
file(GLOB_RECURSE UTILS utils/*.c)
file(GLOB_RECURSE EXAMPLE example/*.c)
file(GLOB_RECURSE MAINMENU mainmenu/*.c)
file(GLOB_RECURSE BAIDU_NETDISK baidu_netdisk/*.c)
file(GLOB_RECURSE COLLECT collect/*.c)
file(GLOB_RECURSE DICT dict/*.c)
file(GLOB_RECURSE RECENTLYPLAYED recentlyPlayed/*.c)
file(GLOB_RECURSE ADDRESSBOOK addressbook/*.c)
file(GLOB_RECURSE SEARCH search/*.c)

file(GLOB_RECURSE EAR_PLAN ear_plan/*.c)
file(GLOB_RECURSE PHONICS phonics/*.c)
file(GLOB_RECURSE UCC ucc/*.c)
file(GLOB_RECURSE UPD_DATA_PARSE upd_data_parse/*.c)
file(GLOB_RECURSE XIMALY ximaLY/*.c ximaLY/images/*.c)

file(GLOB_RECURSE MY_DOWNLOAD my_download/*.c)
file(GLOB_RECURSE LISTENAI_ORAL listenai_oral/*c)
file(GLOB_RECURSE LISTENAI_CHAT listenai_chat/*c)
file(GLOB_RECURSE SETTING setting/*.c)
file(GLOB_RECURSE PINYIN_STUDY pinyin_study/*.c)
file(GLOB_RECURSE PLAYER player/*.c)
file(GLOB_RECURSE PRODUCT_TEST product_test/*.c)
file(GLOB_RECURSE TEST_DEMO_SOURCES ${TEST_DEMO_DIR}/*.c)
file(GLOB_RECURSE WIFI src/wifi/*.c)
file(GLOB_RECURSE SPIFFS src/spiffs/*.c)
file(GLOB_RECURSE HTTP src/http/*.c)

set (DATA_PARSE
    data_parse/aud_lyric.c
    data_parse/analyze_listening.c
    data_parse/dict_parse.c
    data_parse/dict_variant.c
    data_parse/img_res.c
    data_parse/img_std_res.c
    data_parse/chn_mh.c
)

idf_component_register(
   SRCS "app_main.c" "lvgl_main.c" "alarm_sdcard_evt_proc.c"
   ${LV_DEMOS_SOURCES}
   ${FONT_SOURCES}
   ${DATA_PARSE}
   ${EX_WIDGET}
   ${UTILS}
   ${EXAMPLE}
   ${MAINMENU}
   ${BAIDU_NETDISK}
   ${WIFI}
   ${SPIFFS}
   ${HTTP}
   ${MY_DOWNLOAD}
   ${LISTENAI_ORAL}
   ${LISTENAI_CHAT}
   ${SETTING}
   ${PINYIN_STUDY}
   ${PLAYER}
   ${PRODUCT_TEST}
   ${IMAGE_SOURCES}
   ${AUDIO_DIR_SOURCES}
   ${TEST_DEMO_SOURCES}
   ${COLLECT}
   ${DICT}
   ${RECENTLYPLAYED}
   ${STUDYWORD} 
   ${SEARCH}
   ${EAR_PLAN}
   ${PHONICS}
   ${UCC}
   ${UPD_DATA_PARSE}
   ${XIMALY}
    INCLUDE_DIRS "." "./include" "./include/bsp"  "./src/include/" "./test_demo/"
	${LV_DEMO_DIR}   
	${TEST_DEMO_DIR}
	EMBED_TXTFILES music-16b-2c-8000hz.mp3
	EMBED_TXTFILES music-short-a.mp3

)

set_source_files_properties(
    ${LV_DEMOS_SOURCES}
    PROPERTIES COMPILE_OPTIONS
    -DLV_LVGL_H_INCLUDE_SIMPLE
)

add_prebuilt_library(key_driver "./lib/libkey_driver.a" PRIV_REQUIRES main)
add_prebuilt_library(bsp "./lib/libbsp.a"  PRIV_REQUIRES  main )
add_prebuilt_library(app_codec "./lib/libapp_codec.a" PRIV_REQUIRES main)
add_prebuilt_library(audio_media "./lib/libaudio_media.a"  PRIV_REQUIRES main)
add_prebuilt_library(esp32_sdcard "./lib/libesp32_sdcard.a" PRIV_REQUIRES main)
add_prebuilt_library(adc_battery "./lib/libadc_battery.a" PRIV_REQUIRES main )
add_prebuilt_library(xmly "./lib/libxmly.a")
add_prebuilt_library(ximalaya_api "./lib/libximalaya_api.a" PRIV_REQUIRES main  )
add_prebuilt_library(baidu_netdisk_api "./lib/libbaidu_netdisk_api.a" PRIV_REQUIRES main )
add_prebuilt_library(factory_uart "./lib/libfactory_uart.a" PRIV_REQUIRES main )
add_prebuilt_library(sleep_mode "./lib/libsleep_mode.a" PRIV_REQUIRES main )
add_prebuilt_library(esp_device_auth "./lib/libesp_device_auth.a" PRIV_REQUIRES main )

target_link_libraries(${COMPONENT_LIB} PRIVATE key_driver bsp audio_media app_codec esp32_sdcard adc_battery sleep_mode xmly esp_device_auth ximalaya_api baidu_netdisk_api)
