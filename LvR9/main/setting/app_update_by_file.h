#ifndef  __APP_UPDATE_BY_FILE__
#define  __APP_UPDATE_BY_FILE__



#define CODE_UPDATE_FILE_NAME   "tlb_xsz_code.bin"

#define CODE_UPDATE_FILE_PATH  "/userdata/" CODE_UPDATE_FILE_NAME

#define CODE_UPD_FILE_IDENTIF  "beCodeUpd^~1"

typedef struct {
	char m_acIndentifier[12];
	char acReverseA[4];
	char m_acProductName[12];
	int16_t m_versonNo;
	char m_acVersionStr[20];
	char acReverseB[46];
} code_upd_file_header_t;


//------------------------------------------------------

typedef void (* app_upd_by_file_process_cb_t)(int process);


extern int app_file_upgrade_start(app_upd_by_file_process_cb_t processCb);


#endif


