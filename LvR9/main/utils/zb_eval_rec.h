#ifndef  _ZB_EVAL_REC_H_
#define  _ZB_EVAL_REC_H_


#define C_REC_AUD_LEN_MAX (32000 * 10)

#define MY_RING_BUF_LEN   (1024*180) 

#define REC_ONCE_BUF_LEN  (32 * 150)


typedef struct  {
	int8_t * m_pBuf;
	int  m_itemLen;
	int32_t  m_itemNum;
	volatile int32_t  m_writePos;
	volatile int32_t  m_contentCnt;  // 已填入项数 
} my_ringbuf_t;


enum e_zb_olrec_status {
	ZB_OLREC_STAT_IDLE,
	ZB_OLREC_STAT_PREPARE,
	ZB_OLREC_STAT_PROCESSING,
};

#define zb_ringbuf_get_content_bytes(pHndlr)  (pHndlr->m_itemLen * pHndlr->m_contentCnt)


my_ringbuf_t * zb_ringbuf_init(int itemLen, int itemNum);

void zb_ringbuf_free(my_ringbuf_t * pHndlr);

int8_t * zb_ringbuf_read_item(my_ringbuf_t * pHndlr);




void _online_rec_set_parm(int8_t mode, my_ringbuf_t * ringbufHndl);

void _online_rec_clear_parm(void);  // 清除参数，在录音工作彻底结束后调用.


void aud_record_to_ringbuff_task(void * arg);

void _online_rec_set_mode(int8_t mode);

void _online_rec_request_stop(void);

bool _online_rec_has_done(void);


//----------------------------------------------
// 语音评测、识别接口公共的声明、常量 

#define SYS_TIME_SEC_MIN  1714100000L

#define ZBOL_ERRNO_TM_STAMP_INVALID  409

extern bool zb_is_wifi_connect();


#endif

