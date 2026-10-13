#if 0
//#include "audio_stream_example.h"
#include "audio_stream.h"


#if ESP_IDF_VERSION < 0x50200
#include "audio_element.h"
#include "audio_pipeline.h"
#endif
#include "lvgl_main.h"
#include <string.h>

#include "../utils/misc_utils.h"

static char *TAG = "audMemPly";

static struct marker {
    int pos;
    const uint8_t *start;
    const uint8_t *end;
} file_marker;

QueueHandle_t SYS_getMainQueue(void);
void  audio_stream_play_status(music_play_status_t status , void *user_data);



#define  AUDIO_STREAM_TEST 

//----------------------------------------------

static void set_file_marker(uint8_t * pAudStart, const uint8_t * pAudEnd)
{
	file_marker.start = pAudStart;
	file_marker.end   = pAudEnd;
    file_marker.pos = 0;
}

static int music_data_read_cb(AUD_SRC_STREAM_T *stream , void * buf, int len ){
	ESP_LOGI(TAG , " music_data_read_cb   stream->audSrcLength =%d len =%d file_marker.pos =%d \n" , stream->audSrcLength ,len ,file_marker.pos  );
	int read_size = 0  ; 
	if( stream->audSrcLength < 0 ){
		return ESP_FAIL;
	}
#ifdef  AUDIO_STREAM_TEST
	read_size = stream->audSrcLength - file_marker.pos;

	if (read_size <= 0) {
		return AEL_IO_DONE;
	} else if (read_size > len) {
		read_size = len;
	}
	memcpy(buf, file_marker.start + file_marker.pos, read_size);
	file_marker.pos += read_size;
#endif
	return read_size; 
}

static int music_data_seek_cb(AUD_SRC_STREAM_T *stream , int pos ){
	file_marker.pos = pos ; 
	ESP_LOGI(TAG , "  music_data_seek_cb stream->audSrcLength =%d   pos = %d \n" , stream->audSrcLength , pos );
	return AEL_IO_OK; 
}

static void music_data_close_cb(AUD_SRC_STREAM_T *stream ){
	ESP_LOGI(TAG , "  music_data_close_cb   \n");
}


int audio_stream_mem_play(void *xQueue, const uint8_t * pAudStart, const uint8_t * pAudEnd) {
	// Note, 自 24-05-10号版本后，此处不可(也不需要)调用 audio_stream_destroy() 了;
	if (!xQueue) {
		xQueue = SYS_getMainQueue();
	}
	int audLength =(pAudEnd - pAudStart) ;

	AUD_SRC_STREAM_T audioPara = {
		.musicTotalLength = audLength,
		.audSrcLength = audLength,
		.audSrc_read = music_data_read_cb ,
		.audSrc_seek = music_data_seek_cb ,
		.audSrc_close = music_data_close_cb ,
		.auto_mute = true ,
		.task_prio = 10 ,
		.user_data = xQueue ,
	};
	ESP_LOGI(TAG, "play test A");
	set_file_marker(pAudStart, pAudEnd);
	audio_stream_init(&audioPara);
 	vTaskDelay(pdMS_TO_TICKS(50));
	audio_stream_set_status_callback(audio_stream_play_status);

	int iRet = audio_stream_start_play( audioPara.audSrcLength, MUSIC_PLAY_SPEED_1_00); 
	if (0 == iRet) {
		ESP_LOGI(TAG, "start ok");
		audio_stream_set_music_total_length(audLength);
	}
	return iRet;
}

#endif
