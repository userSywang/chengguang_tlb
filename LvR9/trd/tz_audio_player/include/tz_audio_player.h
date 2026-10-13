// audio_player.h
#ifndef TZ_AUDIO_PLAYER_H
#define TZ_AUDIO_PLAYER_H

/*
 * tz_audio_player.h - define export management header file
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


   /*################################本地或网络MP3文件播放接口######################################################*/
	// 函数声明
	// 播放 MP3 文件 时间设置为ms start：开始时间默认0 end结束时间默认-1
	void tz_play_audio(const char* audioname, int64_t start, int64_t end);

	// 暂停播放
	void tz_play_pause_audio();

	// 继续播放
	void tz_play_resume_audio();

	// 停止播放
	void tz_play_stop_audio();

	// 设置音量0-100
	void tz_play_set_volume(int new_volume);

	// 设置播放速度
	void tz_play_set_speed(float new_speed);

	// 新增：动态设置开始时间的函数 时间设置为ms start：开始时间 end结束时间
	void tz_play_set_start_end_time(int64_t start, int64_t end);

	//新增:播放状态
	TZ_PLAYSTATE_T get_play_audio_state();

	//调用者使用
	//extern void audio_play_state(TZ_PLAYSTATE_T state);
	////调用者使用state current_time:s audio_time:s
	extern void audio_play_timestate(TZ_PLAYSTATE_T state,int64_t current_time, int64_t audio_time);



	/*################################MP3数据流播放接口######################################################*/
	// 函数声明
	// 播放 MP3数据
	int tz_play_mem_audio_start(TZ_AUD_SRC_STREAM_T* audioPara);

	// MP3数据 暂停播放
	void tz_play_pause_mem_audio();

	// MP3数据 继续播放
	void tz_play_resume_mem_audio();

	// MP3数据 停止播放
	void tz_play_stop_mem_audio();

	// MP3数据 设置音量0-100
	void tz_play_set_mem_volume(int new_volume);

	// MP3数据 播放状态
	TZ_PLAYSTATE_T get_play_audio_mem_state();


#ifdef __cplusplus
}
#endif

#endif
