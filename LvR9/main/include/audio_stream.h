#ifndef _AUDIO_MEDIA_H_
#define _AUDIO_MEDIA_H_

#include "audio_media_def.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*audio_stream_status_callback)(music_play_status_t status ,void *user_data );

typedef struct S_AUD_SRC_STREAM { 
	int audSrcLength;  // byte length of audio, read only for driver;
	void (*audSrc_close)(struct S_AUD_SRC_STREAM * stream);  // function close stream, NULL mean do not need to close. 
	int  (*audSrc_seek) (struct S_AUD_SRC_STREAM * stream, int position);   // return 0 表示 ok 
	int  (*audSrc_read) (struct S_AUD_SRC_STREAM * stream, void * buf, int len);  //  return real number of read bytes,    AEL_IO_DONE The reading data is complete stop play . other means fail.
	
	int64_t  musicTotalLength ; // calculate the total audio duration   Units: bytes
	bool auto_mute ; //Playback automatically turns on the amplifier, pauses, stops playback, and turns off the amplifier
	int task_prio ; //Task priority
	void * user_data ; //callback param user_data
} AUDIO_SRC_STREAM_T; //




/**
 * @brief  Audio stream decoder init 
 *
 */
int audio_stream_init(AUDIO_SRC_STREAM_T *audSrcStream);

/**
 * @brief  Audio stream decoder destroy 
 *
 */
void audio_stream_destroy(void);


/**
 * @brief  Audio stream player seek 
 *
 * @para   -seek_ms :set audio stream offset in milliseconds
 */
void audio_stream_player_seek(int seek_ms);

/**
 * @brief    Start Audio stream play.
 *
 *           With this function audio_pipeline will create tasks for all elements,
 *           that have been linked using the linking functions.
 * @param	
 		- length : audio_stream length
 		- speed : aduio_stream play speed music_play_speed_t .
 		- seek_ms: aduio_stream play pos
 * @return
 *     - ESP_OK on success
 *     - ESP_FAIL when any errors
 */
int audio_stream_start_play(int length , music_play_speed_t speed );


/**
 * @brief    Start Audio stream seek play.
 *
 *           With this function audio_pipeline will create tasks for all elements,
 *           that have been linked using the linking functions.
 * @param	
 		- length : audio_stream length
 		- speed : aduio_stream play speed music_play_speed_t .
 		- seek_ms: aduio_stream play pos
 		- audSrcLength: play audio_stream length
 		- musicTotalLength : audio file length
 * @return
 *     - ESP_OK on success
 *     - ESP_FAIL when any errors
 */
 
int audio_stream_set_seek_play(int seek_ms ,music_play_speed_t speed ,int audSrcLength , int musicTotalLength );

/**
 * @brief    set audio stream offset byte position play .
 *
 *           With this function audio_pipeline will create tasks for all elements,
 *           that have been linked using the linking functions.
 * @param	
 		- length : audio_stream length
 		- speed : aduio_stream play speed music_play_speed_t .
 		- seek_pos: aduio stream play offset bytes pos
 * @return
 *     - ESP_OK on success
 *     - ESP_FAIL when any errors
 */
int audio_stream_offset_bytePos_play(int length , music_play_speed_t speed, int seek_pos );

/**
 * @brief     Stop all of the linked elements. Used with `audio_pipeline_wait_for_stop` to keep in sync.
 *            The link state of the elements in the pipeline is kept, events are still registered.
 *            The stopped audio_pipeline restart by `audio_pipeline_resume`.
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_FAIL when any errors
 */
int audio_stream_stop_play(void);

/**
 * @brief Pause the audio player
 *
 * @note 1. Only support music and without live stream. If user queue has been registered by evt_que, AUDIO_STATUS_PAUSED event for success
 *       or AUDIO_STATUS_ERROR event for error will be received.
 *       2. The Paused music must be stoped by `esp_audio_stop` before new playing, otherwise got block on new play.
 *
 * @param[in] handle The esp_audio instance
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL : fail
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 *      - ESP_ERR_AUDIO_NOT_READY: the status is not running
 *      - ESP_ERR_AUDIO_TIMEOUT: timeout the pause activity.
 */
int audio_stream_set_pause(void);

/**
 * @brief Resume the audio player
 *
 * @note Only support music and without live stream. If user queue has been registered by evt_que, AUDIO_STATUS_RUNNING event for success
 *       or AUDIO_STATUS_ERROR event for error will be received.
 *
 * @param[in] handle The esp_audio instance
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL 
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 *      - ESP_ERR_AUDIO_TIMEOUT: timeout the resume activity.
 */
int audio_stream_set_resume(void);


/**
 * @brief    setting audio player speed.
 *
 * @param	
 		 speed : aduio_stream play speed music_play_speed_t .
 * @return
 *     - ESP_OK on success
 *     - ESP_FAIL when any errors
 */
int audio_stream_set_play_speed(music_play_speed_t speed);

/**
 * @brief Get the current playback time and total music duration
 *
 *
 * @param total_time_ms:   total music duration
 * 		    cur_time_ms:   current playback time
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: no player instance
 *      - ESP_ERR_AUDIO_NOT_READY: no codec element
 */
int audio_stream_get_play_time(uint32_t* play_total_time_ms , uint32_t* play_cur_time_ms );

/**
 * @brief Set the playback state callback function.
 *
 * @param audio_stream_status_callback
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
 */
int audio_stream_set_status_callback( music_player_status_callback callback);

/**
 * @brief      Get music stream playback status.
 *
 * @return
 *     - music_play_status_t
 *     - STATUS_ERROR 
 */
music_play_status_t audio_stream_get_play_status(void);

/* @brief audio stream set stream music total length.
 *
 * @param[in]:length
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int audio_stream_set_music_total_length(int64_t length);

#ifdef __cplusplus
}
#endif

#endif // _AUDIO_MEDIA_H_
