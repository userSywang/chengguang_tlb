#ifndef __APP_CODEC_H__
#define __APP_CODEC_H__

#ifndef C_PC_SIM
#include "esp_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	AMPLIFIER_STATUS_OFF  = 0,
	AMPLIFIER_STATUS_ON   = 1,
} amplifier_status_t;


typedef enum {
    CODEC_MIC_GAIN_0DB = 0,
    CODEC_MIC_GAIN_6DB ,
    CODEC_MIC_GAIN_12DB,
    CODEC_MIC_GAIN_18DB,
    CODEC_MIC_GAIN_24DB,
    CODEC_MIC_GAIN_30DB,
    CODEC_MIC_GAIN_36DB,
    CODEC_MIC_GAIN_42DB,
    CODEC_MIC_GAIN_MAX
}mic_gain_t;

/*
* @brief codec init
* @param - suspend = 1  codec Low power mode , 0 codec Normal mode
* @return
*	  - ESP_OK	  Success
*	  - ESP_FAIL  Fail
*/
int app_codec_init( int suspend);

/*
* @brief codec set colume
* @param	- volume  range ：0 - 10  
* @return
*	  - ESP_OK	  Success
*	  - ESP_FAIL  Fail
*/
int app_codec_set_volume(int volume);

/*
* @brief codec get colume
* @param	- volume  range ：0 - 10  
* @return
*	  - ESP_OK	  Success
*	  - ESP_FAIL  Fail
*/
int app_codec_get_volume(int *volume);


/**
 * @brief codec Set mic gain
 * @param - gain_db     :  gain db of mic gain
 * @param - digital_vol :  digital volume range from 0x0 - 0xff (default 0xbf )
 * @return
 *     - ESP_OK    Success
 *     - ESP_FAIL  Fail
 */
int app_codec_set_mic_gain(mic_gain_t gain_db , uint8_t digital_vol);

/*
* @brief get mic gain db
* @param - gain_db mic_gain_t  
* @param  mic_gain_t  
* @return
*	  - ESP_OK	  Success
*	  - ESP_FAIL  Fail
*/
int app_codec_get_mic_gain(mic_gain_t *gain_db , uint8_t *digital_vol);

/**
 * @brief codec mode ctrl start
 *
 * @return
 *	   - ESP_OK    Success
 *	   - ESP_FAIL  Fail
 */
int app_codec_ctrl_state();

/*
* @brief codec set suspend mode
* @return
*	  - ESP_OK	  Success
*	  - ESP_FAIL  Fail
*/
int app_codec_suspend();

/*
* @brief codec set mute
* @param - mute : true  codec mute , false  unmute  
* @return
*	  - ESP_OK	  Success
*	  - ESP_FAIL  Fail
*/
int app_codec_set_mute(bool        mute );

/*
* @brief get codec mute status 
* @return
*	  - true mute , false unmute 
*	  - other  Fail
*/
int app_codec_get_mute();

/*
* @brief setting headphones mute

* @param	- amplifier_status_t    
*/

void app_codec_headphones_mute(bool mute);

/*
* @brief setting amplifier on off

* @param	- amplifier_status_t    
*/
void set_amplifier_onoff(amplifier_status_t status);


/*
* @brief get amplifier status
* @return	- amplifier_status_t    
*/
amplifier_status_t get_amplifier_status(void);


#ifdef __cplusplus
}
#endif

#endif //__APP_CODEC_H__