
//#include "../../components/lvgl__lvgl/lvgl.h"
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"

#include "../data_parse/analyze_listening.h"

#include "contactAP_data.h"
#include "contact_ap.h"


static const char* TAG = "contactDat";
//--------------------------------------------------------------




// 加载联系人记录
int tbcd_load_contact_record(const char* file_path) {

    if (!file_path ) return -1;


    if (zb_isFileExist("/userdata/_data")== 0)
    {
        zb_mkdir("/userdata/_data",0);
        printf("mkdir /userdata/_data file\r\n ");
    }

    FILE* file = zb_fopen(file_path, "rb");
    if (!file) {
        ESP_LOGI(TAG, "联系人文件不存在");//在保存的时候新建
        FILE* file = zb_fopen(file_path, "wb");
        zb_fclose(file);
        return 0; 
    }

    TBCD_CONTACT_RECORD_HEADER header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        ESP_LOGE(TAG, "读取文件头失败");
        zb_fclose(file);
        return -1;
    }


    if (header.magic != TBCD_CONTACT_RECORD_MAGIC ||
        header.version != TBCD_CONTACT_RECORD_VERSION ||
        header.record_count > TBCD_CONTACT_MAX) { 
        ESP_LOGE(TAG, "无效的文件头或记录数超标");
        zb_fclose(file);
        return -1;
    }

    size_t read_count = fread(g_pContactAp.contacts, sizeof(TBCD_CONTACT_RECORD_ITEM),
        header.record_count, file);
    if (read_count != header.record_count) {
        ESP_LOGE(TAG, "读取联系人记录失败，期望 %d 实际 %d",
            header.record_count, read_count);
        zb_fclose(file);
        return -1;
    }

    g_pContactAp.contact_count = header.record_count;
    zb_fclose(file);
    return 0;
}

// 保存联系人记录
int tbcd_save_contact_record(const char* file_path) {
    if (!file_path) return -1;

    FILE* file = zb_fopen(file_path, "wb");
    if (!file) {
        ESP_LOGE(TAG, "无法创建文件");
        return -1;
    }

    TBCD_CONTACT_RECORD_HEADER header = {
        .magic = TBCD_CONTACT_RECORD_MAGIC,
        .version = TBCD_CONTACT_RECORD_VERSION,
        .record_count = g_pContactAp.contact_count
    };


    if (fwrite(&header, sizeof(header), 1, file) != 1) {
        ESP_LOGE(TAG, "写入文件头失败");
        zb_fclose(file);
        return -1;
    }

    if (g_pContactAp.contact_count > 0) {
        size_t written = fwrite(g_pContactAp.contacts, sizeof(TBCD_CONTACT_RECORD_ITEM),
            g_pContactAp.contact_count, file);
        if (written != g_pContactAp.contact_count) {
            ESP_LOGE(TAG, "写入联系人数据不完整");
            zb_fclose(file);
            return -1;
        }
    }

    zb_fclose(file);
    return 0;
}

#if 1
bool tbcd_contact_has_phone(const char* phone, TBCD_CONTACT_RECORD_ITEM* calling_contact) {

    if (!phone || !calling_contact) {
        printf("Error: Invalid input parameters\n");
        return false;
    }


    FILE* file = zb_fopen(CONTACT_FILE_PATH, "rb");
    if (!file) {
        file = zb_fopen(CONTACT_FILE_PATH, "wb");
        zb_fclose(file);
        printf("Error: Failed to open contact file\n");
        return false;
    }

    TBCD_CONTACT_RECORD_HEADER header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to read file header\n");
        zb_fclose(file);
        return false;
    }

    if (header.magic != TBCD_CONTACT_RECORD_MAGIC ||
        header.version != TBCD_CONTACT_RECORD_VERSION ||
        header.record_count > TBCD_CONTACT_MAX) {
        printf("Error: Invalid file format\n");
        zb_fclose(file);
        return false;
    }

    TBCD_CONTACT_RECORD_ITEM* temp_contacts = malloc(sizeof(TBCD_CONTACT_RECORD_ITEM) * header.record_count);
    if (!temp_contacts) {
        printf("Error: Memory allocation failed\n");
        zb_fclose(file);
        return false;
    }


    size_t read_count = zb_fread(temp_contacts, sizeof(TBCD_CONTACT_RECORD_ITEM), header.record_count, file);
    zb_fclose(file);


    if (read_count != header.record_count) {
        printf("Error: Incomplete contact data read\n");
        free(temp_contacts);
        return false;
    }

    bool found = false;
    for (uint16_t i = 0; i < header.record_count; ++i) {
        if (strncmp(temp_contacts[i].phone, phone, PHONE_NUM_MAX_LEN) == 0) {
        
            memcpy(calling_contact, &temp_contacts[i], sizeof(TBCD_CONTACT_RECORD_ITEM));
            found = true;
            break;
        }
    }

    if (found == false) {
        printf("contactlist no this person\r\n");
    }

    free(temp_contacts);
    return found;
}

#else
bool tbcd_contact_has_phone(const char* phone, TBCD_CONTACT_RECORD_ITEM* calling_contact) {

    if (!phone || !calling_contact) {
        ESP_LOGE(TAG, "无效输入参数");
        return false;
    }

    FILE* file = zb_fopen(CONTACT_FILE_PATH, "rb");
    if (!file) {
        file = zb_fopen(CONTACT_FILE_PATH, "wb");
        zb_fclose(file);
        ESP_LOGE(TAG, "无法打开联系人文件");
        return false;
    }

    // 读取文件头
    TBCD_CONTACT_RECORD_HEADER header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        ESP_LOGE(TAG, "文件头读取失败");
        zb_fclose(file);
        return false;
    }

    // 验证文件头
    if (header.magic != TBCD_CONTACT_RECORD_MAGIC ||
        header.version != TBCD_CONTACT_RECORD_VERSION ||
        header.record_count > TBCD_CONTACT_MAX) {
        ESP_LOGE(TAG, "无效的文件格式");
        zb_fclose(file);
        return false;
    }

    TBCD_CONTACT_RECORD_ITEM* temp_contacts = malloc(sizeof(TBCD_CONTACT_RECORD_ITEM) * header.record_count);
    if (!temp_contacts) {
        ESP_LOGE(TAG, "内存分配失败");
        zb_fclose(file);
        return false;
    }

    size_t read_count = zb_fread(temp_contacts, sizeof(TBCD_CONTACT_RECORD_ITEM), header.record_count, file);
    zb_fclose(file);

    if (read_count != header.record_count) {
        ESP_LOGE(TAG, "联系人数据读取不完整");
        free(temp_contacts);
        return false;
    }

    // 搜索目标电话号码
    bool found = false;
    for (uint16_t i = 0; i < header.record_count; ++i) {
        if (strncmp(temp_contacts[i].phone, phone, PHONE_NUM_MAX_LEN) == 0) {
            memcpy(calling_contact, &temp_contacts[i], sizeof(TBCD_CONTACT_RECORD_ITEM));
            found = true;
            //tbcd_incoming_call_scr_init(true); // 调用指定函数
            break;
        }
    }
    if(found == false) frintf("contactlist no this person\r\n");
    free(temp_contacts);
    return found;
}
#endif


bool tbcd_contact_sos(const char* name, TBCD_CONTACT_RECORD_ITEM* calling_contact) {

    if (!name || !calling_contact) {
        printf("Error: Invalid input parameters\n");
        return false;
    }


    FILE* file = zb_fopen(CONTACT_FILE_PATH, "rb");
    if (!file) {
        file = zb_fopen(CONTACT_FILE_PATH, "wb");
        zb_fclose(file);
        printf("Error: Failed to open contact file\n");
        return false;
    }

    TBCD_CONTACT_RECORD_HEADER header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to read file header\n");
        zb_fclose(file);
        return false;
    }

    if (header.magic != TBCD_CONTACT_RECORD_MAGIC ||
        header.version != TBCD_CONTACT_RECORD_VERSION ||
        header.record_count > TBCD_CONTACT_MAX) {
        printf("Error: Invalid file format\n");
        zb_fclose(file);
        return false;
    }

    TBCD_CONTACT_RECORD_ITEM* temp_contacts = malloc(sizeof(TBCD_CONTACT_RECORD_ITEM) * header.record_count);
    if (!temp_contacts) {
        printf("Error: Memory allocation failed\n");
        zb_fclose(file);
        return false;
    }


    size_t read_count = zb_fread(temp_contacts, sizeof(TBCD_CONTACT_RECORD_ITEM), header.record_count, file);
    zb_fclose(file);


    if (read_count != header.record_count) {
        printf("Error: Incomplete contact data read\n");
        free(temp_contacts);
        return false;
    }

    bool found = false;
    for (uint16_t i = 0; i < header.record_count; ++i) {
        if (strncmp(temp_contacts[i].name, name, PHONE_NUM_MAX_LEN) == 0) {

            memcpy(calling_contact, &temp_contacts[i], sizeof(TBCD_CONTACT_RECORD_ITEM));
            found = true;
            break;
        }
    }

    if (found == false) {
        printf("contactlist no this person\r\n");
    }

    free(temp_contacts);
    return found;
}
