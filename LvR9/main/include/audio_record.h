#ifndef _AUDIO_RECORD_H_
#define _AUDIO_RECORD_H_

#include "audio_media_def.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	RECORD_PCM_DATA  = 0,
	RECORD_WAV_SD   ,
}record_data_type_t;
	
typedef enum {
	AUDIO_RECORD_INIT  = 0,
	AUDIO_RECORD_START	,
	AUDIO_RECORD_STOP	,
	AUDIO_RECORD_DEINIT	,
}record_status_t;




typedef struct audio_record_cfg{
	record_data_type_t type ; 
	int rate ;  //Sample rate  eg: 16000
	int task_prio ; //Task priority
	int task_core ;//Task task_core
	char *record_path;//record save file path  eg: /sdcard/test.wav

}audio_record_cfg_t;



/**
 * @brief    record init .
 *
 * @Note: When playing audio with different sampling rates, re recording requires setting the sampling rate, otherwise the sampling rate may be incorrect
 *
 * @param	
 		 audio_record_cfg_t: record init param  .
 * @return
 *     - ESP_OK on success
 *     - ESP_OK < fail
 */
int audio_record_task_init(audio_record_cfg_t *cfg);


/**
 * @brief    record deinit .
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_OK < fail
 */
int audio_record_task_deinit(void);


/**
 * @brief get record data.
 *
 * @Note: When playing audio with different sampling rates, re recording requires setting the sampling rate, otherwise the sampling rate may be incorrect
 *
 * @param	
 		 char * buffer : record buffer  .
 		 int data_len  : data len 
 * @return
 *     - ESP_OK on success
 *     - ESP_OK < fail
 */
int read_record_pcm_data(char * buffer , int data_len );

/**
 * @brief set record clk .
 *
 * @param	
 		 - rate  :sampling rate
 		 - bits  : default 16
 		 - ch    : default 1
 * @return
 *     - ESP_OK on success
 *     - ESP_OK < fail
 */
int audio_record_set_clk(int rate, int bits, int ch);


#ifdef __cplusplus
}
#endif

#endif // _AUDIO_RECORD_H_
