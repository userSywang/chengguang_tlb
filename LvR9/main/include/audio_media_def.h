#ifndef _AUDIO_MEDIA_DEF_H_
#define _AUDIO_MEDIA_DEF_H_


#ifdef __cplusplus
extern "C" {
#endif


#define ESP_ERR_PLAYER_BASE                  (0x80000 + 0x1000) /*!< Starting number of player error codes */

typedef enum {
    ESP_ERR_PLAYER_NO_ERROR                  = 0,
    ESP_ERR_PLAYER_FAIL                      = -1,

    /* player errors */
    ESP_ERR_PLAYER_NO_INPUT_STREAM           = ESP_ERR_PLAYER_BASE + 1,  // 0x0x81001
    ESP_ERR_PLAYER_NO_OUTPUT_STREAM          = ESP_ERR_PLAYER_BASE + 2,
    ESP_ERR_PLAYER_NO_CODEC                  = ESP_ERR_PLAYER_BASE + 3,
    ESP_ERR_PLAYER_HAL_FAIL                  = ESP_ERR_PLAYER_BASE + 4,
    ESP_ERR_PLAYER_MEMORY_LACK               = ESP_ERR_PLAYER_BASE + 5,
    ESP_ERR_PLAYER_INVALID_URI               = ESP_ERR_PLAYER_BASE + 6,
    ESP_ERR_PLAYER_INVALID_PATH              = ESP_ERR_PLAYER_BASE + 7,
    ESP_ERR_PLAYER_INVALID_PARAMETER         = ESP_ERR_PLAYER_BASE + 8,
    ESP_ERR_PLAYER_NOT_READY                 = ESP_ERR_PLAYER_BASE + 9,
    ESP_ERR_PLAYER_NOT_SUPPORT               = ESP_ERR_PLAYER_BASE + 10,
    ESP_ERR_PLAYER_TIMEOUT                   = ESP_ERR_PLAYER_BASE + 11,
    ESP_ERR_PLAYER_ALREADY_EXISTS            = ESP_ERR_PLAYER_BASE + 12,
    ESP_ERR_PLAYER_LINK_FAIL                 = ESP_ERR_PLAYER_BASE + 13,
    ESP_ERR_PLAYER_UNKNOWN                   = ESP_ERR_PLAYER_BASE + 14,
    ESP_ERR_PLAYER_OUT_OF_RANGE              = ESP_ERR_PLAYER_BASE + 15,
    ESP_ERR_PLAYER_STOP_BY_USER              = ESP_ERR_PLAYER_BASE + 16,

    ESP_ERR_PLAYER_OPEN_TIMEOUT              = ESP_ERR_PLAYER_BASE + 0x100,// 0x81100
    ESP_ERR_PLAYER_INPUT                     = ESP_ERR_PLAYER_BASE + 0x101,
    ESP_ERR_PLAYER_PROCESS                   = ESP_ERR_PLAYER_BASE + 0x102,
    ESP_ERR_PLAYER_OUTPUT                    = ESP_ERR_PLAYER_BASE + 0x103,
    ESP_ERR_PLAYER_CLOSE                     = ESP_ERR_PLAYER_BASE + 0x104,
} media_player_err_t;


typedef enum {
    MUSIC_PLAY_SPEED_UNKNOW = -1,
    MUSIC_PLAY_SPEED_0_50   = 0,
    MUSIC_PLAY_SPEED_0_75   = 1,
    MUSIC_PLAY_SPEED_1_00   = 2,
    MUSIC_PLAY_SPEED_1_25   = 3,
    MUSIC_PLAY_SPEED_1_50   = 4,
    MUSIC_PLAY_SPEED_1_75   = 5,
    MUSIC_PLAY_SPEED_2_00   = 6,
    MUSIC_PLAY_SPEED_MAX    = 7,
} music_play_speed_t;
	

typedef enum {
	MUSIC_STATUS_UNKNOWN  = 0,
	MUSIC_STATUS_PLAYING  = 1,
	MUSIC_STATUS_PAUSE    = 2,
	MUSIC_STATUS_STOP	  = 3,
	MUSIC_STATUS_FINISH   = 4,
	MUSIC_STATUS_ERROR	  = 5,
} music_play_status_t;


typedef enum {
    MUSIC_SOURCE_TYPE_SD = 0  ,
    MUSIC_SOURCE_TYPE_HTTP = 1    ,
	MUSIC_SOURCE_TYPE_RAW = 2	 ,
} music_source_type_t;

typedef void (*music_player_status_callback)(music_play_status_t status , void * user_data);



#ifdef __cplusplus
}
#endif

#endif // _AUDIO_MEDIA_DEF_H_
