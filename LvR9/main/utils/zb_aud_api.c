
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zb_aud_api.h"

#ifndef C_PC_SIM

#include "../lvgl_main.h"
#include <string.h>
#include "../utils/misc_utils.h"

#include "../../trd/tz_audio_player/include/tz_audio_player.h"

#endif


static volatile int32_t g_audTmTotalMs;
static volatile int32_t g_audTmCurMs;

static TZ_AUD_SRC_STREAM_T g_audioPara;

//------------------------------------------------------------------------

/** thzy 约定的播放进度回调.
 */
//void audio_play_timestate(TZ_PLAYSTATE_T state, int64_t current_time, int64_t audio_time)
//{
//    char buf[100];
//
//    g_audTmCurMs = (int32_t)current_time;
//    g_audTmTotalMs = (int32_t)audio_time;
//    memset(buf, 0, sizeof(buf)); // 使用 memset 初始化所有元素为0
//    snprintf(buf, sizeof(buf), "{%lld/%lld}  %02lld:%02lld / %02lld:%02lld ", current_time, audio_time, 
//        (current_time / 1000) / 60,
//        (current_time / 1000) % 60,      //当前播放时间
//        (audio_time / 1000) / 60,
//        (audio_time / 1000) % 60
//    ); //  总时间
//
//     printf(" Playback PlayState:%d progress# %s\n", state, buf);
//    //fflush(stdout);
//}

int music_player_get_play_time(int32_t * cur_pos_ms, int32_t * total_time_ms)
{
	if (cur_pos_ms) {
		(*cur_pos_ms) = g_audTmCurMs;
	}
	if (total_time_ms) {
		(*total_time_ms) = g_audTmTotalMs;
	}
	return 0;
}


#if 1
#ifndef C_PC_SIM

extern volatile int32_t  g_pwrAutoOffTimeCnt;

//--------------------------------------------------------------
#define TAG  "audApi" 

#define SUB_TAG  "audSFPart"

typedef struct { 
	FILE  * m_file;
	int32_t  m_audSt;
	int32_t  m_audLen;
	uint8_t m_encParm;
	uint8_t m_startApNo;  // 在哪个 AP 里启动播放的 
	int32_t  m_seekPos;  // 语音内的字节偏移 
	int32_t g_startPosMs;
} AUD_SRC_FILE_PART_T; //



//------------------------------------------------------

static AUD_SRC_FILE_PART_T g_audPartInf;

//------------------------------------------------------

static void _aud_dec_dat(void * pStr, int len, uint8_t encParam)
{
	for (int idx = 0; idx < len; idx++) {
		((uint8_t *)pStr)[idx] ^= encParam;
	}
}

static int aud_file_part_read_cb(void * buf, int len ) {
    ESP_LOGI(TAG, "partRead A, buf %p, len %d", buf, len);
    if (!buf || (len <= 0)) {
        return 0;
    }
    ESP_LOGI(TAG , "%s data_read_cb   audSrcLen=%d len =%d pos =%d \n" , SUB_TAG, g_audPartInf.m_audLen , len, (int)g_audPartInf.m_seekPos);
    int read_size = 0; 
    if (g_audPartInf.m_audLen < 0) {
        return -1;
    }
    read_size = g_audPartInf.m_audLen - g_audPartInf.m_seekPos;

    if (read_size <= 0) {
        return 0;
    } else if (read_size > len) {
        g_pwrAutoOffTimeCnt = 0;
        read_size = len;
    }
    //ESP_LOGI(TAG, "readPart B");
    zb_fseek(g_audPartInf.m_file, g_audPartInf.m_audSt + g_audPartInf.m_seekPos, 0);
    //ESP_LOGI(TAG, "readPart C");
    zb_fread(buf, 1, read_size, g_audPartInf.m_file);
    //ESP_LOGI(TAG, "readPart D");
    if (g_audPartInf.m_encParm) {
        _aud_dec_dat(buf, read_size, g_audPartInf.m_encParm);
    }
    g_audPartInf.m_seekPos += read_size;
    //ESP_LOGI(TAG, "readPart Z");
    return read_size; 
}

static int aud_file_part_seek_cb(long offset, int whence) {
	g_audPartInf.m_seekPos = offset; 
	if (offset < 0 || offset > 100 * 1024 * 1024) {
		ESP_LOGE(TAG , "%s stream->audSrcLength =%d  pos = %d" , SUB_TAG,  g_audPartInf.m_audLen, offset);
	}  else {
		ESP_LOGI(TAG , "%s stream->audSrcLength =%d  pos = %d, whence=%d" , SUB_TAG,  g_audPartInf.m_audLen, offset, whence);
	} 
	return 0; 
}

static void aud_file_part_close_cb() {
	ESP_LOGI(TAG , "%s music_data_close_cb", SUB_TAG);
}


int  aud_file_part_play_ex(void * xQueue,   FILE * fp, int32_t offset, int32_t audLen, 
		uint8_t encParm, uint8_t extra, int16_t speed, int32_t offsetMs) {

	// Note, 自 24-05-10号版本后，此处不可(也不需要)调用 audio_stream_destroy() 了;
	if (!fp || !audLen) return -1;
	ESP_LOGI(TAG, "partPly A");
#if 0
	if ((0 == (extra & 0x01)) && (get_player_backstage_state() >= 0)) {
		uint8_t apNo = zb_getApNo();
		if (!M_IS_AP_START_AUD_BG(apNo)) {
			ESP_LOGI(TAG, "stop prev bgPlay!");
			play_text_page_free();  // 前面的背景播放还在进行中，先停它 
	 		return -3;
		}
	}
#endif
	TZ_PLAYSTATE_T audStat = get_play_audio_mem_state();
	if (TZ_PLAYING == audStat || TZ_PLAYPAUSED == audStat) {
		tz_play_stop_audio();
		usleep(40 * 1000);
	}

	TZ_AUD_SRC_STREAM_T audPara = {
		.audSrcLength = audLen,
		.audSrc_read = aud_file_part_read_cb,
		.audSrc_seek = aud_file_part_seek_cb,
		.audSrc_close = aud_file_part_close_cb,
	};
	g_audioPara = audPara;
	//--------------
	ESP_LOGI(TAG, "partPlay fileHdl:%p, ofst %x,len %d, parm %hx, pos %ld", fp, (int)offset, (int)audLen, (short)encParm, offsetMs);
	memset(&g_audPartInf, 0, sizeof(g_audPartInf));
	g_audPartInf.m_file = fp;
	g_audPartInf.m_audSt = offset;
	g_audPartInf.m_audLen = audLen;
	g_audPartInf.m_encParm = encParm;
	g_audPartInf.m_seekPos = 0;  // 语音内的字节偏移 
	
	ESP_LOGI(TAG, "audFPart play A");
	//set_amplifier_onoff(AMPLIFIER_STATUS_ON);

	ESP_LOGI(TAG, "audFPart play B");
	usleep(50*1000);
#if 0
	if (speed < 0 || speed >= MUSIC_PLAY_SPEED_MAX) {
		ESP_LOGW(TAG, "correct speed (%d) to 1", speed);
		speed = MUSIC_PLAY_SPEED_1_00;
	}
#endif
	int iRet = 0;
	//if (0 == offsetMs)  
	iRet = tz_play_stream_audio_start(&g_audioPara);//tz_play_mem_audio_start
	/* else {
		iRet = audio_stream_set_seek_play(offsetMs, speed, audLen, audLen);
	} */

	if (iRet >= 0) {
		ESP_LOGI(TAG, "start ok %d", iRet);
		g_audPartInf.m_startApNo = zb_getApNo();
	} else {
		ESP_LOGW(TAG, "start_ply ret %d", iRet);
		//set_amplifier_onoff(AMPLIFIER_STATUS_OFF);
	}
	return iRet;
}


//----------------------------------------------------------------------

#else  // on PC_SIM

int  aud_file_part_play_ex(void* xQueue, FILE* fp, int32_t offset, int32_t audLen,
	uint8_t encParm, uint8_t extra, int16_t speed, int32_t startTm) {

	return 0;
}

		
#endif
#endif



#ifndef C_PC_SIM
int music_player_stop(void)
{
	return 0;
}
#endif

int music_player_set_seek(int seek_time_ms) {
	return 0;
}


