
#ifndef _ZB_AUD_MEM_H_
#define _ZB_AUD_MEM_H_

#include "../utils/misc_utils.h"

//------------------------------------------------------
#ifndef C_PC_SIM
extern const uint8_t lr_mp3_start[] asm("_binary_music_16b_2c_8000hz_mp3_start");
extern const uint8_t lr_mp3_end[]   asm("_binary_music_16b_2c_8000hz_mp3_end");

extern const uint8_t short_a_aud_start[] asm("_binary_music_short_a_mp3_start");
extern const uint8_t short_a_aud_end[]   asm("_binary_music_short_a_mp3_end");
#endif


/**
 * @return  0 means success 
 *
 * audio_stream_mem_play()的对应的播放控制函数有 audio_stream_stop_play(),
 *     audio_stream_set_pause() ... (见 main/include/audio_stream.h );
 *  播放器的 status 见 music_play_status_t ( audio_media_def.h ).
 */
int  audio_stream_mem_play(void *xQueue, const uint8_t * pAudStart, const uint8_t * pAudEnd);


#endif

