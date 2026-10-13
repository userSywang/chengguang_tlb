#include "player.h"
#ifndef C_PC_SIM
#include "LvR9/main/online_res_ap/cpp_api_bridge.h"
#endif



play_book_locate_t play_list_book_locate = { 0 };
play_book_locate_t play_text_book_locate = { 0 };
play_text_state_t play_text_state = { 0 };
Playlist current_playlist = {0};

lv_obj_t* play_main_win = NULL;   // 播放主页面容器
lv_obj_t* play_list_win = NULL;   // 播放列表页面容器
lv_obj_t* play_text_win = NULL;   // 播放文本内容页面容器
lv_obj_t* play_repeat_win = NULL; // 跟读页面容器

#ifndef C_PC_SIM
extern int yd_cloud_ap_get_media_self(const char* path, void (*funcPtr)(const char*, const char*));
extern bool yd_cloud_ap_check_isDownload(const char* path);
#endif

PlayerRequestState g_player_request_state = {
    .is_requesting = false,
    .requesting_index = -1,
    .type = 0,
    .retry_count = 0
};

void player_request_init() {
    memset(&g_player_request_state, 0, sizeof(PlayerRequestState));
    g_player_request_state.requesting_index = -1;
}

bool player_acquire_request_lock(int index, const char* path, MusicSourceType source_type) {
    if (g_player_request_state.is_requesting) return false;

    g_player_request_state.is_requesting = true;
    g_player_request_state.requesting_index = index;
    g_player_request_state.type = source_type;
    g_player_request_state.retry_count = 0;

    
    if (g_player_request_state.requesting_path) {
        free(g_player_request_state.requesting_path);
    }
    g_player_request_state.requesting_path = strdup(path);

    return true;
}

void player_release_request_lock() {
    if (g_player_request_state.requesting_path) {
        free(g_player_request_state.requesting_path);
    }

    memset(&g_player_request_state, 0, sizeof(PlayerRequestState));
    g_player_request_state.requesting_index = -1;
}

int player_increment_retry_count() {

    return  g_player_request_state.retry_count++;  
}


// 释放全局播放列表资源
void free_global_playlist() {
    if (current_playlist.items) {
        for (int i = 0; i < current_playlist.count; i++) {
            if (current_playlist.items[i].location) {
                heap_caps_free(current_playlist.items[i].location);
            }
            if (current_playlist.items[i].title) {
                heap_caps_free(current_playlist.items[i].title);
            }
        }
        heap_caps_free(current_playlist.items);
        current_playlist.items = NULL;
        current_playlist.count = 0;
    }

    memset(&current_playlist, 0, sizeof(Playlist));
}

// 创建播放列表项
PlaylistItem create_playlist_item(MusicSourceType type, const char* location, const char* title, int32_t cover) {
    PlaylistItem item = { 0 };
    item.source_type = type;
    item.cover = cover;

    if (location) {
        item.location = strdup(location);
    }

    if (title) {
        item.title = strdup(title);
    }

    return item;
}


void request_media_playback(const char* media_path, MusicSourceType source_type) {
    if (!media_path) {
        LV_LOG_ERROR("Null media path provided");
        return;
    }

    char* path_copy = strdup(media_path);
    if (!path_copy) {
        LV_LOG_ERROR("Memory allocation failed for media path");
        return;
    }

    char local_buf[400];
    char* local_path = local_buf;
    char* hash = NULL, * path = NULL;
    bool use_hash = false;
    bool is_cached = false;

    ESP_LOGI("request_media_playback", "before check download \r\n");
    // 处理资源缓存检查
    if (source_type == RES_YD_ONLINE) {
#ifndef C_PC_SIM
        is_cached = check_res_down(source_type, media_path, &local_path, 0, false);
#endif
}
    else if (source_type == RES_BAIDU_CLOUD) {
        split_hash_path(media_path, &hash, &path);
        if (path) {
#ifndef C_PC_SIM
            is_cached = yd_cloud_ap_check_isDownload(path);
#endif
        }
    }
    ESP_LOGI("request_media_playback","source_type :%d,is_cached :%d\r\n", source_type, is_cached);

    // 如果已缓存，直接播放并返回
    if (is_cached) {
        const char* play_path = (source_type == RES_BAIDU_CLOUD) ? path : local_path;
        if (play_path) {
            LV_LOG_USER("Playing cached resource: %s", play_path);
            tz_play_audio(play_path, 0, -1, true);
        }
        player_release_request_lock();
        free(path_copy);
        if (source_type == RES_BAIDU_CLOUD) {
            free(hash);
            free(path);
        }
        return;
    }

#ifndef C_PC_SIM
    // 准备请求参数
    const char* request_param = path_copy;
    if (source_type == RES_BAIDU_CLOUD && hash) {
        // 使用哈希值作为请求参数，并释放不再需要的路径副本
        free(path_copy);
        path_copy = strdup(hash);  // 创建哈希的副本用于请求状态
        request_param = hash;
        use_hash = true;

        // 释放不再需要的拆分路径
        if (path) free(path);
        path = NULL;
    }

    // 设置请求状态
    g_player_request_state.is_requesting = true;
    g_player_request_state.requesting_path = path_copy;  // 这里存储的是哈希或原始路径
    g_player_request_state.retry_count = 0;
    g_player_request_state.type = source_type;  // 存储资源类型用于后续处理

    // 发起请求
    int result = 0;
    if (source_type == RES_YD_ONLINE) {
        ESP_LOGI("MediaRequest", "RES_YD_ONLINE Starting request: %s", request_param);
        result = online_play_ap_get_media_self(request_param, media_playback_handler);
    }
    else if (source_type == RES_BAIDU_CLOUD) {
        ESP_LOGI("MediaRequest", "RES_BAIDU_CLOUD Starting request: %s", request_param);
        result = yd_cloud_ap_get_media_self(request_param, media_playback_handler);
    }

    // 请求失败处理
    if (result != 0) {
        LV_LOG_ERROR("Request initiation failed: %d", result);
        player_release_request_lock();
        free(path_copy);
        if (source_type == RES_BAIDU_CLOUD) {
            free(hash);
            if (path) free(path);
    }
    }
    else {
        // 请求成功 - 释放拆分出的路径（哈希将在回调中释放）
        if (source_type == RES_BAIDU_CLOUD && !use_hash) {
            free(hash);
            if (path) free(path);
        }
    }
#endif
}



//处理返回的音频数据
// 修改回调函数签名
void media_playback_handler(const char* media_url) {
#define MAX_RETRY 3
    if (!media_url) {
        LV_LOG_ERROR("Media request failed");

        // 检查索引有效性
        bool index_valid = (
            g_player_request_state.requesting_index >= 0 &&
            g_player_request_state.requesting_index < current_playlist.count
            );

        if (player_increment_retry_count() < MAX_RETRY && index_valid) {
            LV_LOG_USER("Retrying request (attempt %d)",
                g_player_request_state.retry_count);

            // 使用请求时保存的路径
            request_media_playback(g_player_request_state.requesting_path, g_player_request_state.type);
            return;
        }

        // 最终失败处理
        LV_LOG_ERROR("Request failed after %d attempts", MAX_RETRY);
        //show_error_ui("Media unavailable");  // 添加的UI通知函数

        player_release_request_lock();
        play_next_unit();
        return;
    }
    ESP_LOGI("media_playback_handler","URL NOT NULL\r\n");

    // 验证索引有效性
    if (g_player_request_state.requesting_index == -1 ||
        g_player_request_state.requesting_index >= current_playlist.count) {
        LV_LOG_WARN("Invalid index in response");
        free((void*)media_url);
        player_release_request_lock();
        return;
    }

    // 成功处理
    int play_index = g_player_request_state.requesting_index;
    play_text_state.cur_play_unit_num = play_index;

    //// 更新UI前检查窗口是否仍然有效
    //if (lv_obj_is_valid(play_text_win)) {
    //    PlaylistItem* current = &current_playlist.items[play_index];
    //}

    LV_LOG_USER("Playback started: %s", media_url);
    tz_play_audio(media_url, 0, -1, true);

    // 释放资源
    free((void*)media_url);
    player_release_request_lock();
}


void split_hash_path(const char* combined, char** hash, char** path) {
    // 初始化输出参数
    *hash = NULL;
    *path = NULL;

    if (!combined) return;

    char* separator = strchr(combined, '#');
    if (!separator) return;


    size_t hash_len = separator - combined;


    *hash = (char*)malloc(hash_len + 1);
    if (!*hash) return;
    strncpy(*hash, combined, hash_len);
    (*hash)[hash_len] = '\0';


    *path = strdup(separator + 1);
}


void free_split_result(char* hash, char* path) {
    free(hash);
    free(path);
}











player_style_t player_poem_style = {
    .aud_img = PLAYER_IMG_LABAPOEM,
    .btn_def_color = 0xECDBC3,
    .btn_focus_color = 0xD3A97A,
    .no_content_img = PLAYER_IMG_EMPTY_POEM, //PLAYER_IMG_ZANWUNEIRONGPOEM,
    .page_bg_img1 = PLAYER_IMG_BEIJINGPOEM1,
    .page_bg_img2 = PLAYER_IMG_BEIJINGPOEM2,
    .record_btn_img1 = PLAYER_IMG_GDBOFANGPOEM1,
    .record_motion_img = {PLAYER_IMG_REC_MOTION_POEM_1, PLAYER_IMG_REC_MOTION_POEM_2, PLAYER_IMG_REC_MOTION_POEM_3, PLAYER_IMG_REC_MOTION_POEM_4},
    .text_def_color = 0xD8D8D8,
    .text_focus_color = 0xC6A179,
};

player_style_t player_normal_style = {
    .aud_img = PLAYER_IMG_LABANORMAL,
    .btn_def_color = 0xE8E8EE,
    .btn_focus_color = 0x393F4D,
    .no_content_img = PLAYER_IMG_EMPTY_NORMAL, //PLAYER_IMG_ZANWUNEIRONGNORMAL,
    .page_bg_img1 = PLAYER_IMG_BEIJINGNORMAL1,
    .page_bg_img2 = PLAYER_IMG_BEIJINGNORMAL1,
    .record_btn_img1 = PLAYER_IMG_GDBOFANGNORMAL1,
    .record_motion_img = {PLAYER_IMG_REC_MOTION_NORMAL_1, PLAYER_IMG_REC_MOTION_NORMAL_2, PLAYER_IMG_REC_MOTION_NORMAL_3, PLAYER_IMG_REC_MOTION_NORMAL_4},
    .text_def_color = 0xD8D8D8,
    .text_focus_color = 0xFFE675,
};

player_style_t player_style = { //现在用于播放器使用 -----04.19 wsy
    .aud_img = PLAYER_IMG_LABANORMAL,
    .btn_def_color = 0xF5F7FA,
    .btn_focus_color = 0xE6F6FF,
    .no_content_img = PLAYER_IMG_EMPTY_NORMAL, //PLAYER_IMG_ZANWUNEIRONGNORMAL,
    .page_bg_img1 = PLAYER_IMG_PLAYER_BG,
    .page_bg_img2 = PLAYER_IMG_PLAYER_BG,
    .record_btn_img1 = PLAYER_IMG_GDBOFANGNORMAL1,
    .record_motion_img = {PLAYER_IMG_REC_MOTION_NORMAL_1, PLAYER_IMG_REC_MOTION_NORMAL_2, PLAYER_IMG_REC_MOTION_NORMAL_3, PLAYER_IMG_REC_MOTION_NORMAL_4},
    .text_def_color = 0x18191B,
    .lyric_text_color = 0xD8D8D8,
    .text_focus_color = 0x008CFF,
};

player_style_t player_style_mao = {  //学生证大猫 //现在用于播放器使用 -----04.19 wsy
    .aud_img = PLAYER_IMG_LABANORMAL,
    .btn_def_color = 0xF5F7FA,
    .btn_focus_color = 0xE6F6FF,
    .no_content_img = PLAYER_IMG_EMPTY_NORMAL, //PLAYER_IMG_ZANWUNEIRONGNORMAL,
    .page_bg_img1 = PLAYER_IMG_PLAYER_BG,
    .page_bg_img2 = PLAYER_IMG_PLAYER_BG,
    .record_btn_img1 = PLAYER_IMG_GDBOFANGNORMAL1,
    .record_motion_img = {PLAYER_IMG_REC_MOTION_NORMAL_1, PLAYER_IMG_REC_MOTION_NORMAL_2, PLAYER_IMG_REC_MOTION_NORMAL_3, PLAYER_IMG_REC_MOTION_NORMAL_4},
    .text_def_color = 0x18191B,
    .lyric_text_color = 0xD8D8D8,
    .text_focus_color = 0x008CFF,
};


player_style_t player_style_chs_classic = {  //学生证大猫 //现在用于播放器使用 -----04.19 wsy
    .aud_img = PLAYER_IMG_LABANORMAL,
    .btn_def_color = 0xF5F7FA,
    .btn_focus_color = 0xE6F6FF,
    .no_content_img = PLAYER_IMG_EMPTY_NORMAL, //PLAYER_IMG_ZANWUNEIRONGNORMAL,
    .page_bg_img1 = PLAYER_IMG_PLAYER_BG,
    .page_bg_img2 = PLAYER_IMG_PLAYER_BG,
    .record_btn_img1 = PLAYER_IMG_GDBOFANGNORMAL1,
    .record_motion_img = {PLAYER_IMG_REC_MOTION_NORMAL_1, PLAYER_IMG_REC_MOTION_NORMAL_2, PLAYER_IMG_REC_MOTION_NORMAL_3, PLAYER_IMG_REC_MOTION_NORMAL_4},
    .text_def_color = 0x18191B,
    .lyric_text_color = 0xD8D8D8,
    .text_focus_color = 0x008CFF,
};



//替换定义
int audio_stream_set_pause(void) {}
int audio_stream_stop_play(void){}
int audio_stream_set_play_speed(music_play_speed_t speed){}
int audio_stream_set_resume(void) {}
music_play_status_t audio_stream_get_play_status(void) { return 0; }
void audio_stream_player_seek(int seek_ms) {}


