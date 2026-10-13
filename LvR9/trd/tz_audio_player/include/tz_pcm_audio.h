// audio_player.h
#ifndef TZ_PCMAUDIO_H
#define TZ_PCMAUDIO_H

/*
 * audio_player.h - define export management header file
 *
 * Copyright (C) 2025
 * version:1.0.0
 * Author: ly@
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	/*root@rv1106: / output# tinypcminfo
		Info for card 0, device 0:
	PCM out :
    Access:   0x000009
    Format[0] : 0x000444
    Format[1] : 00000000
    Format Name : S16_LE, S24_LE, S32_LE
    Subformat : 0x000001
    Rate : min = 8000Hz      max = 192000Hz
    Channels : min = 2           max = 2
    Sample bits : min = 16          max = 32
    Period size : min = 32          max = 65536
    Period count : min = 2           max = 4096

    PCM in :
    Access:   0x000009
    Format[0] : 0x000444
    Format[1] : 00000000
    Format Name : S16_LE, S24_LE, S32_LE
    Subformat : 0x000001
    Rate : min = 8000Hz      max = 192000Hz
    Channels : min = 2           max = 4
    Sample bits : min = 16          max = 32
    Period size : min = 16          max = 65536
    Period count : min = 2           max = 8192*/

    //录音：主要是口语对话音频要求：音频格式要求：采样率16K，单声道，wav
    //filename 文件名：*.wav  录音时间:s save_mono:是否保存单通道
	int tz_record_wav(const char* filename, int record_time, int save_mono);

	//filename 文件名：*.wav  该播放对应录音接口 录音的文件播放，播放音频，单通道转双通道
    int tz_play_wav(const char* filename);


#ifdef __cplusplus
}
#endif

#endif
