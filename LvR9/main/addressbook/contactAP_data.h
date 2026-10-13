#ifndef _WRDDATA_H_
#define _WRDDATA_H_

#include "stdint.h"

#define PHONE_NUM_MAX_LEN   20

// 联系人记录项
typedef struct {
	char name[30];              // 联系人姓名
	char phone[PHONE_NUM_MAX_LEN];             // 联系人电话
	uint8_t num_img;            //头像编号
} TBCD_CONTACT_RECORD_ITEM;

// 联系人记录文件头
typedef struct {
	uint32_t magic;          // 文件标识
	uint16_t version;        // 版本号
	uint16_t record_count;   // 记录数量
} TBCD_CONTACT_RECORD_HEADER;


//当前使用/编辑联系人数据
extern TBCD_CONTACT_RECORD_ITEM new_contact; 

//当前通话联系人数据
extern TBCD_CONTACT_RECORD_ITEM calling_contact;

//来电弹窗联系人数据
extern TBCD_CONTACT_RECORD_ITEM popup_contact;

#define TBCD_CONTACT_RECORD_MAGIC 0x434F4E54  // "CON"
#define TBCD_CONTACT_RECORD_VERSION 1

#define CONTACT_FILE_PATH                "/sdcard/_data/contact_record"

// 数据操作
extern int tbcd_load_contact_record(const char* file_path);
extern int tbcd_save_contact_record(const char* file_path);
extern bool tbcd_contact_has_phone(const char* phone, TBCD_CONTACT_RECORD_ITEM* calling_contact);
extern bool tbcd_contact_sos(const char* name, TBCD_CONTACT_RECORD_ITEM* calling_contact);
#endif


