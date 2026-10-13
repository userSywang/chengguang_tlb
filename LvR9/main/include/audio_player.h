#ifndef _AUDIO_PLAY_H_
#define _AUDIO_PLAY_H_

#include "audio_media_def.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MP3_HTTPS_TEST_URL "https://dl.espressif.com/dl/audio/ff-16b-2c-44100hz.mp3"
#define MP3_SD_TEST_URL "file://sdcard/music/1.mp3"


typedef struct music_player_cfg{
	bool auto_mute ; //Playback automatically turns on the amplifier, pauses, stops playback, and turns off the amplifier
	int task_prio ; //Task priority
	int task_core ;//Task task_core
	
	void *user_data ; 
}music_player_cfg_t;


//typedef void (*music_player_status_callback)(music_play_status_t status , void * user_data);

/**
 * @brief Create music player 
 *
 *
 * @param[in] music player initialization configuration
 *
 * @return
 *      - NULL: Error
 *      - Others: esp_audio instance fully certifying
 */
int music_player_init(music_player_cfg_t *cfg);


/**
 * @brief Specific music player  instance will be destroyed
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: no instance to free, call esp_audio_init first
 */
int music_player_destroy();


/**
 * @brief A synchronous interface for stop the music player.
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 *      - ESP_ERR_AUDIO_NOT_READY: The status is not AUDIO_STATUS_RUNNING or AUDIO_STATUS_PAUSED or element has not created
 *      - ESP_ERR_AUDIO_TIMEOUT: timeout(8000ms) the stop activity.
 */
int music_player_stop(void);



/**
 * @brief Play the given uri
 *
 * There is a rule that music player will select input stream, codec and output stream by URI field.
 * 
 * @param - uri : player uri or path

 * Now esp_audio_play support follow URIs.
 *     - "https://xxxx.mp3"
 *     - "http://xxx.wav"
 *     - "file://sdcard/test.mp3"
 * 
 * @param - type:  music_source_type_t
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_TIMEOUT: timeout the play activity
 *      - ESP_ERR_AUDIO_NOT_SUPPORT: Currently status is AUDIO_STATUS_RUNNING
 *      - ESP_ERR_AUDIO_INVALID_URI: URI is illegal
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 *      - ESP_ERR_AUDIO_STOP_BY_USER: Exit without play due to esp_audio_stop has been called.


 
 */
int music_player_set_play_uri(const char *uri,music_source_type_t type);



/* @brief Call this function before play to change the music source type.
 *
 * @note Music source type has been kept after music finished.
 *
 * @param[in] type      Specific media source type will be set.
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 */
int music_player_set_play_type(music_source_type_t type);


/**
 * @brief Seek the position in second of currently played music.
 *
 * @param[out] seek_time_ms , A pointer to int that indicates esp_audio decoding position.
 * 
 * @note : Only the MP3 format supports seek milliseconds , and other formats seek are accurate to the second
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_FAIL: codec or allocation fail
 *      - ESP_ERR_AUDIO_TIMEOUT: timeout for sync the element status
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: no esp_audio instance
 *      - ESP_ERR_AUDIO_NOT_SUPPORT: codec has finished
 *      - ESP_ERR_AUDIO_OUT_OF_RANGE: the seek_time_ms is out of the range
 *      - ESP_ERR_AUDIO_NOT_READY: the status is neither running nor paused
 */
int music_player_set_seek(int seek_time_ms);


/**
 * @brief Pause the music player
 *
 * @note 1. Only support music. If user queue has been registered by evt_que, AUDIO_STATUS_PAUSED event for success
 *       or AUDIO_STATUS_ERROR event for error will be received.
 *       2. The Paused music must be stoped by `music_player_stop` before new playing, otherwise got block on new play.
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 *      - ESP_ERR_AUDIO_NOT_READY: the status is not running
 *      - ESP_ERR_AUDIO_TIMEOUT: timeout the pause activity.
 */
int music_player_pause();


/**
 * @brief Resume the music paused
 *
 * @note Only support music . If user queue has been registered by evt_que, AUDIO_STATUS_RUNNING event for success
 *       or AUDIO_STATUS_ERROR event for error will be received.
 *
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 *      - ESP_ERR_AUDIO_TIMEOUT: timeout the resume activity.
 */
int music_player_resume();



/**
 * @brief Get the position in microseconds of currently played music.
 *
 * @note This function works only with decoding music.
 *
 * @param[out] total_time_ms     A pointer to int that indicates music player decoding length.
 
 * @param[out] cur_pos_ms     A pointer to int that indicates music player decoding position.
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: no esp_audio instance
 *      - ESP_ERR_AUDIO_NOT_READY: no out stream
 */
int music_player_get_play_time(int* total_time_ms , int* cur_pos_ms );


/**
 * @brief Get music player status
 *
 * @param[out] status    A pointer to music_play_status_t that indicates esp_audio status.
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: no music player instance or music player does not playing
 */
music_play_status_t music_player_state_get();

/**
 * @brief Use speed_index which is from "music_play_speed_t" enum to set esp_audio play speed.
 *
 * @param[in] speed_index     Value from "music_play_speed_t" enum.  
 * 
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
 */
int music_player_play_speed_set(music_play_speed_t speed_index);


/* @brief Set music_player status event callback.
 *
 * @param[in] cb        A pointer to music_player_status_callback
 *
 * @return
 *      - ESP_ERR_AUDIO_NO_ERROR: on success
 *      - ESP_ERR_AUDIO_INVALID_PARAMETER: invalid arguments
*/
int music_player_set_status_callback(music_player_status_callback callback);


/* @brief Set music player element stream clk
 *
 * @param[in]
 *		- rate  
 *		- bits  
 *		- ch  
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_stream_set_clk(int rate, int bits, int ch);


/* @brief Set music player play raw data.
 *
 * @param[in]
 *		- buffer   
 *		- data_len  
 *		- timeout
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_player_raw_data(char *buffer , int data_len , TickType_t timeout);;

/* @brief Set music player stream config param.
 *
 * @param[in]
 *		-config ： AUD_SRC_STREAM_T 
 *		 Note:  auto_mute 、task_prio parameters are invalid
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
void audio_player_set_stream_config(void *config);


/* @brief Set music player stream seek play.
 *
 * @param[in]:seek_time_ms
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_set_stream_seek(int seek_time_ms);




/* @brief Set music player set stream seek play.
 *
 * @param[in]:
 *		-seek_ms : 
 *		-speed
 *		-audSrcLength
 *		-musicTotalLength
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/

int music_player_set_stream_seek_play(int seek_ms ,music_play_speed_t speed ,int audSrcLength , int musicTotalLength );



/* @brief Set music player stream stream length.
 *
 * @param[in]:length
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_set_stream_length(int length);

/* @brief Set music player stream seek bytePos. note : Set offset only and do not play
 *
 * @param[in]:bytePos
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_set_stream_seek_bytePos(int bytePos);


/* @brief Set music player set stream offset bytePos play
 *
 * @param[in]:bytePos
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_set_stream_offset_bytePos_play(int bytePos);



/* @brief music player set stream music total length.
 *
 * @param[in]:length
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_set_stream_music_total_length(int64_t length);

/* @brief get music player element handle.
 *
 *
 * @return
 *      element handle
*/
void *get_element_handle(void);;



/* @brief Set music player play raw data.  note:Non-blocking playback , Variable speed not supported
 *
 * @param[in]
 *		- int(*pcmPlayPointer)(char *, int)   
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail
*/
int music_player_play_pcm_data(int(*pcmPlayPointer)(char *, int ));


/* @brief music player stop pcm play
 *
 * @return
 *      - ESP_OK: on success
 *      - ESP_FAIL: fail

 * note: music_player_stop 、 music_player_destroy 、audio_stream_destroy、audio_stream_stop_play ,  pcm play Will also stop
*/
int music_player_stop_pcm_play(void);


#ifdef __cplusplus
}
#endif

#endif // _AUDIO_PLAY_H_
