
#include "../data_parse/cJSON.h"
#include "../utils/misc_utils.h"


#define TAG "jsonTst"

static void json_test_a(void)
{
    cJSON* item = NULL;
    cJSON* found = NULL;

    item = cJSON_Parse("{\"one\":1, \"Two\":2, \"tHree\":3}");

    found = cJSON_GetObjectItem(NULL, "test");
    //TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL pointer.");

    found = cJSON_GetObjectItem(item, NULL);
    //TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL string.");

    found = cJSON_GetObjectItem(item, "one");
    //TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    //TEST_ASSERT_EQUAL_DOUBLE(found->valuedouble, 1);

    found = cJSON_GetObjectItem(item, "tWo");
    //TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    //TEST_ASSERT_EQUAL_DOUBLE(found->valuedouble, 2);

    found = cJSON_GetObjectItem(item, "three");
    //TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find item.");
    //TEST_ASSERT_EQUAL_DOUBLE(found->valuedouble, 3);

    found = cJSON_GetObjectItem(item, "four");
    //TEST_ASSERT_NULL_MESSAGE(found, "Should not find something that isn't there.");

    cJSON_Delete(item);
}



static int test_json_b(void)
{
    const char text[] = "{\"timestamp\":\"2019-03-03 08:45:57\", \"value\":1}";

    cJSON * json, * json_value, * json_timestamp;

    json = cJSON_Parse(text);
    if (NULL == json)
    {
        ESP_LOGE(TAG, "Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    json_value = cJSON_GetObjectItem(json, "value");
    if (json_value->type == cJSON_Number)
    {
        ESP_LOGI(TAG, "value: %d\n", json_value->valueint);
    }

    json_timestamp = cJSON_GetObjectItem(json, "timestamp");
    if (json_timestamp->type == cJSON_String)
    {
        ESP_LOGI(TAG, "%s\n", json_timestamp->valuestring);
    }

    cJSON_Delete(json);
    return 0;
}




static int test_json_c(void)
{
	static const char *message = 
"{                              \
    \"name\":\"mculover666\",   \
    \"age\": 22,                \
    \"weight\": 55.5,           \
    \"address\":                \
        {                       \
            \"country\": \"China\",\
            \"zip-code\": 111111\
        },                      \
    \"skill\": [\"drawing\", \"sing\", \"dance\", \"football\" ],\
    \"student\": false          \
}";
    cJSON* cjson_test = NULL;
    cJSON* cjson_name = NULL;
    cJSON* cjson_age = NULL;
    cJSON* cjson_weight = NULL;
    cJSON* cjson_address = NULL;
    cJSON* cjson_address_country = NULL;
    cJSON* cjson_address_zipcode = NULL;
    cJSON* cjson_skill = NULL;
    cJSON* cjson_student = NULL;
    int    skill_array_size = 0, i = 0;
    cJSON* cjson_skill_item = NULL;

    /* 解析整段JSO数据 */
    cjson_test = cJSON_Parse(message);
    if(cjson_test == NULL)
    {
        ESP_LOGE(TAG, "parse fail.\n");
        return -1;
    }

    /* 依次根据名称提取JSON数据（键值对） */
    cjson_name = cJSON_GetObjectItem(cjson_test, "name");
    cjson_age = cJSON_GetObjectItem(cjson_test, "age");
    cjson_weight = cJSON_GetObjectItem(cjson_test, "weight");

    ESP_LOGI(TAG, "name: %s\n", cjson_name->valuestring);
    ESP_LOGI(TAG, "age:%d\n", cjson_age->valueint);
    ESP_LOGI(TAG, "weight:%.1f\n", cjson_weight->valuedouble);

    /* 解析嵌套json数据 */
    cjson_address = cJSON_GetObjectItem(cjson_test, "address");
    cjson_address_country = cJSON_GetObjectItem(cjson_address, "country");
    cjson_address_zipcode = cJSON_GetObjectItem(cjson_address, "zip-code");
    ESP_LOGI(TAG, "address-country:%s\naddress-zipcode:%d\n", cjson_address_country->valuestring, cjson_address_zipcode->valueint);

    /* 解析数组 */
    cjson_skill = cJSON_GetObjectItem(cjson_test, "skill");
    skill_array_size = cJSON_GetArraySize(cjson_skill);
    ESP_LOGI(TAG, "skill:[");
    for(i = 0; i < skill_array_size; i++)
    {
        cjson_skill_item = cJSON_GetArrayItem(cjson_skill, i);
        ESP_LOGI(TAG, "%s,", cjson_skill_item->valuestring);
    }
    ESP_LOGI(TAG, "\b]\n");

    /* 解析布尔型数据 */
    cjson_student = cJSON_GetObjectItem(cjson_test, "student");
    if(cjson_student->valueint == 0)
    {
        ESP_LOGI(TAG, "student: false\n");
    }
    else
    {
        ESP_LOGI(TAG, "student:error\n");
    }
    return 0;
}


int json_test(void) {
    //json_test_a();
    test_json_b();
    test_json_c();
    return 0;
}

