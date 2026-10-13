
#ifndef _ZB_AUD_API_H_
#define _ZB_AUD_API_H_

#include "../utils/misc_utils.h"


#ifdef C_PC_SIM
// 定义播放状态枚举
typedef enum {
	TZ_PLAYIDLE,
	TZ_PLAYING,
	TZ_PLAYPAUSED,
	TZ_PLAYSTOPPED,
	TZ_PLAYERROR
} TZ_PLAYSTATE_T;


//TZ_AUD_SRC_STREAM_T 类型定义
typedef struct {
	size_t audSrcLength;
	int (*audSrc_read)(void* buf, size_t size);
	int (*audSrc_seek)(long offset, int whence);
	void (*audSrc_close)();
} TZ_AUD_SRC_STREAM_T;

#endif


#if 0
#include "../include/audio_media_def.h"
#include "../include/audio_player.h"
#endif

/**
 * @brief 按文件句柄、位置和偏移 来播放语音 
 * @param speed  值的范围见 music_play_speed_t, 中速为 MUSIC_PLAY_SPEED_1_00 (2)
 * @return 0 means success,  < 0 means fail 
 * aud_file_part_play()的对应的播放控制函数有 audio_stream_stop_play(),
 *     audio_stream_set_pause() ... (见 main/include/audio_stream.h );
 *  播放器的 status 见 music_play_status_t ( audio_media_def.h ).
 *  播放器的状态变化时, 当前主窗体的第一个 callback 将会收到 EVENT_EX_AUD_STREAM 消息, 其 parm 为 int *, 所指向 int 类数值为播放器状态.
 */
int  aud_file_part_play_ex(void * xQueue,   FILE * fp, int32_t offset, int32_t audLen, 
		uint8_t encParm, uint8_t extra, int16_t speed, int32_t offsetMs);


#define aud_file_part_play(xQueue,fp,offset,audLen,encParm,speed)  aud_file_part_play_ex(xQueue,fp,offset,audLen,encParm,0,speed, 0)




/**
 * @brief 播放 buffer 里的语音(一般用于播放录音的数据) 
 */
int  aud_buffer_play(void * xQueue,         const void *pAud, int32_t audLen, int16_t speed);



//----------------------------------------------------------------------

#if 0
int zb_music_player_set_play_uri(const char * uri, music_source_type_t type);
#endif


#endif

