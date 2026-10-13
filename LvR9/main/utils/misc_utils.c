/** 界面无关的 一些小工具 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <dirent.h>

#ifndef C_PC_SIM
#include <unistd.h>
#else
#include <io.h>
#endif

#ifndef C_PC_SIM
#include <time.h>
#endif

#include <sys/types.h>
#ifndef C_PC_SIM
#else
#include <direct.h>
#endif

#ifndef C_PC_SIM
#include <sys/statvfs.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#endif


#include "misc_utils.h"

#include <lvgl/src/misc/lv_log.h>
#include <lvgl/src/misc/lv_timer.h>
#include <lvgl/src/misc/lv_math.h>
#include <lvgl/src/misc/lv_mem.h>
#include <lvgl/src/misc/lv_async.h>
#include <lvgl/src/misc/lv_printf.h>
#include <lvgl/src/misc/lv_fs.h>

#ifndef errno
extern int errno;
#endif


extern int UTF8ToUTF16(char* pStrSrc, wchar_t* pwcStrDst, int iDstBuffSize, int param);


#define TAG  "miscUtl"

//----------------------------------------------
static int16_t g_openedFileN;

volatile bool  g_hasClockCorrectionByNtp;
static int8_t  g_ntpServerNo; 
static int32_t g_lastNtpSyncTmSinceNetCnct;  // last sync time time (unit: ms)

#ifndef C_PC_SIM
static int64_t g_timeProcessSt;
#endif

//----------------------------------------------

const char * g_p____bar = "--------------------";


#if 1

/* 内存申请 (可以精细控制, 比如有性能要求、地址对齐要求 等)
 */
void * heap_caps_malloc(size_t size, uint32_t flag) {
	return malloc(size);
}

void * heap_caps_realloc(void * ptr, size_t size, uint32_t flag) {
	return realloc(ptr, size);
}

void heap_caps_free(void * ptr) {
	free(ptr);
}
#endif


/* 缺省的内存申请 (没有特别要求)
 */
void * heap_caps_malloc_dflt(size_t size) {
	return malloc(size);
}

void * heap_caps_realloc_dflt(void * ptr, size_t size) {
	return realloc(ptr, size);
}

#ifdef C_PC_SIM
static void cvtLogicPathToReal(const char * filePath, char * pathOnPc, int bufLen) {
#define STR_PATH_A_HEAD "/userdata/"
#define STR_PATH_A_SIM_BASE   "../diskData/A/"
#define STR_PATH_S_HEAD "/spiffs/"
#define STR_PATH_S_SIM_BASE   "../diskData/S/"

	pathOnPc[0] = '\0';
	if (memcmp(filePath, STR_PATH_A_HEAD, sizeof(STR_PATH_A_HEAD) - 1) == 0) {
		strncat(pathOnPc, STR_PATH_A_SIM_BASE, bufLen - 1 - 8);
		strncat(pathOnPc, filePath + sizeof(STR_PATH_A_HEAD) - 1, bufLen - 1 - sizeof(STR_PATH_A_SIM_BASE));
	}
	else if (memcmp(filePath, STR_PATH_S_HEAD, sizeof(STR_PATH_S_HEAD) - 1) == 0) {
		strncat(pathOnPc, STR_PATH_S_SIM_BASE, bufLen - 1 - 8);
		strncat(pathOnPc, filePath + sizeof(STR_PATH_S_HEAD) - 1, bufLen - 1 - sizeof(STR_PATH_S_SIM_BASE));
	}
	else {
		strncat(pathOnPc, filePath, bufLen - 1);
	}
}
#endif


FILE * zb_fopen(const char * filePath, const char * mode) {
#ifndef C_PC_SIM
	FILE * fileA = fopen(filePath, mode);
	ESP_LOGI(TAG, "****----- fopen %s", filePath);
	if (!fileA) {
		ESP_LOGE(TAG, "fopen errno is %d, %s", errno, filePath);
	} else {
		g_openedFileN++;
		ESP_LOGI(TAG, "---fopen fp:%p", fileA);
	}
	return fileA;
#else
	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];
	wchar_t aModeUni[8] = {0};

	ESP_LOGI(TAG, "****----- fopen %s", filePath);
	cvtLogicPathToReal(filePath, acPath, _MAX_PATH);

	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	UTF8ToUTF16(mode, aModeUni, M_ARRAY_LEN(aModeUni), 0);
	FILE * fp = _wfopen(aPathUni, aModeUni);
	if (fp) {
		g_openedFileN++;
	}
	return fp;
#endif
}

/*
 * @return 0 means success 
 */
int zb_fclose( FILE * fp ) {
	if (!fp)
		return EOF;
	int ret = fclose(fp);
	if (0 == ret) {
		g_openedFileN--;
	}
	else {
		ESP_LOGE(TAG, "fail to close fp:%p", fp);
	}
	return ret;
}

int zb_getOpenedFileNum() {
	return g_openedFileN;
}


int zb_fread(void *ptr, int size, int nmemb, FILE *stream) {
	return  fread(ptr, size, nmemb, stream);
}

int zb_fwrite(void *ptr, int size, int nmemb, FILE *stream) {
	return  fwrite(ptr, size, nmemb, stream);
}

int zb_mkdir(const char *path, int mode) {
	int iFuncRet = -1;
#ifndef C_PC_SIM
	if (0 == mode) {
		mode = 0777;
	}
	iFuncRet = mkdir(path, mode);
	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "mkdir err:%d", errno);
	}
#else
	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];

	cvtLogicPathToReal(path, acPath, _MAX_PATH);
	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	iFuncRet = _wmkdir(aPathUni);
#endif
	return iFuncRet;
}

int zb_remove(const char *path) {
	int iFuncRet = -1;
#ifndef C_PC_SIM
	iFuncRet = remove(path);
	if (iFuncRet != 0) {
		iFuncRet = rmdir(path);
	}
#else
	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];

	cvtLogicPathToReal(path, acPath, _MAX_PATH);
	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	iFuncRet = _wremove(aPathUni);
#endif
	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "%s [%d], remove fail:%d", __FUNCTION__, __LINE__,errno);
	}
	return iFuncRet;
}

int zb_rmdir(const char *path) {
	int iFuncRet = -1;
#ifndef C_PC_SIM
	iFuncRet = rmdir(path);

#else
	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];

	cvtLogicPathToReal(path, acPath, _MAX_PATH);
	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	iFuncRet = _wrmdir(aPathUni);
#endif
	if (iFuncRet < 0) 
	{
		ESP_LOGI(TAG, "%s [%d] zb_rmdir:%s, fail: %d", __FUNCTION__, __LINE__, path, errno);
	}
	else
	{
		ESP_LOGI(TAG, "%s [%d] zb_rmdir: [%s] ok. \n", __FUNCTION__, __LINE__, path);
	}
	return iFuncRet;
}


int zb_rename(const char *old_name, const char *new_name) {
	int iFuncRet = -1;
#ifndef C_PC_SIM
	iFuncRet = rename(old_name, new_name);
#else
	wchar_t aPathUniOld[_MAX_PATH];
	wchar_t aPathUniNew[_MAX_PATH];

	{
		char acPathOld[_MAX_PATH];
		cvtLogicPathToReal(old_name, acPathOld, _MAX_PATH);
		aPathUniOld[0] = '\0';
		UTF8ToUTF16(acPathOld, aPathUniOld, M_ARRAY_LEN(aPathUniOld), 0);
	}
	{
		char acPathNew[_MAX_PATH];
		cvtLogicPathToReal(new_name, acPathNew, _MAX_PATH);
		aPathUniNew[0] = '\0';
		UTF8ToUTF16(acPathNew, aPathUniNew, M_ARRAY_LEN(aPathUniNew), 0);
	}

	iFuncRet = _wrename(aPathUniOld, aPathUniNew);
#endif
	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "rename fail:%d", errno);
	}
	return iFuncRet;
}


/**
 * @return  0 means success 
 */
int zb_access(const char *pathname, int mode)
{
	//ESP_LOGE(TAG, "已进入：zb_access");
	int ret = 0;
#ifndef C_PC_SIM
	ret = access(pathname, mode);
#else
	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];

	cvtLogicPathToReal(pathname, acPath, _MAX_PATH);
	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	ret = _waccess(aPathUni, mode);
#endif
	if (ret < 0) {
		ESP_LOGE(TAG, "access ret = %d", ret);
	}
	return ret;
}


int zb_stat__st_mode(const char *pathname, int * p_st_mode)
{
	//ESP_LOGI(TAG, "zb_stat__st_mode A");
	int ret = 0;
	if (!p_st_mode)
		return -1;
#ifndef C_PC_SIM
	struct stat stats = { 0 };
	ret = stat(pathname, &stats);
	if (0 == ret) {
		(*p_st_mode) = stats.st_mode;
	}
#else
	struct __stat64 stats = { 0 };

	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];

	cvtLogicPathToReal(pathname, acPath, _MAX_PATH);
	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	ret = _wstat64(aPathUni, &stats);
	if (0 == ret) {
		(*p_st_mode) = stats.st_mode;
	}
#endif
	if (ret < 0) {
		ESP_LOGE(TAG, "fail to call stat %s", pathname);
		return 0;
	}
	//ESP_LOGI(TAG, "ret = %d", ret);
	return ret;
}


int64_t zb_getFileLength(const char *pathname) {
	//ESP_LOGE(TAG, "已进入：zb_getFileLength");
	int ret = 0;
#ifndef C_PC_SIM
	struct stat stats = { 0 };
	ret = stat(pathname, &stats);
#else
	struct __stat64 stats = { 0 };

	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];

	cvtLogicPathToReal(pathname, acPath, _MAX_PATH);
	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	ret = _wstat64(aPathUni, &stats);
#endif
	if (ret != 0) {
		ESP_LOGE(TAG, "fail to call stat %s", pathname);
		return 0;
	}
	ESP_LOGE(TAG, "ret = %d", ret);
	return stats.st_size;
}


int64_t zb_stat__mtime(const char *pathname)
{
	//ESP_LOGI(TAG, "zb_stat__mtime A");
	int64_t ret = -1;
	if (!pathname)
		return -3;
#ifndef C_PC_SIM
	struct stat stats = { 0 };
	ret = stat(pathname, &stats);
	if (0 == ret) {
		ret = stats.st_mtime;
	}
#else
	struct __stat64 stats = { 0 };

	char acPath[_MAX_PATH];
	wchar_t aPathUni[_MAX_PATH];

	cvtLogicPathToReal(pathname, acPath, _MAX_PATH);
	aPathUni[0] = '\0';
	UTF8ToUTF16(acPath, aPathUni, M_ARRAY_LEN(aPathUni), 0);
	ret = _wstat64(aPathUni, &stats);
	if (0 == ret) {
		ret = stats.st_mtime;
	}
#endif
	if (ret < 0) {
		ESP_LOGE(TAG, "fail to call stat__mtime %s", pathname);
		return 0;
	}
	//ESP_LOGI(TAG, "ret = %d", ret);
	return ret;
}


int sys_get_sdcard_capacity( uint64_t * total_space , uint64_t * free_space) 
{
	int iFuncRet = -1;
	if (total_space) {
		(*total_space) = 0;
	}
	if (free_space) {
		(*free_space) = 0;
	}
#ifndef C_PC_SIM
	struct statvfs stat;
	
	if (statvfs(ZB_WORK_DIR, &stat) != 0) {
		// error happens, just quits here
		return -1;
	}
	
	(*total_space) = (uint64_t)stat.f_bsize * stat.f_blocks;
	(*free_space) = (uint64_t)stat.f_bsize * stat.f_bavail;
	iFuncRet = 0;
#else

	if (total_space) {
		(*total_space) = 1024*1024*1024LL * 32;
	}
	if (free_space) {
		(*free_space) = 1024*1024*1024LL * 2;
	}
	iFuncRet = 0;
#endif
	return iFuncRet;
}


//----------------------------------------------

#ifndef C_PC_SIM
int zb_get_time_init(void)
{
    struct timespec ts; 
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return -1;
    }
    int64_t elapseMs = (ts.tv_sec * 1000LL) + ((int64_t)ts.tv_nsec / (1000* 1000));
    g_timeProcessSt = elapseMs;
    return 0;
}


int32_t zb_get_time_ms_since_boot(void)
{
    struct timespec ts; 
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return 0;
    }
    int64_t elapseMs = (ts.tv_sec * 1000LL) + ((int64_t)ts.tv_nsec / (1000* 1000));
    int64_t timeElapse = (elapseMs - g_timeProcessSt);
    if (timeElapse < 0) {
        g_timeProcessSt = elapseMs;
        timeElapse = 0;
    }
    return (int32_t)timeElapse;
}

static void time_sync_notification_cb(struct timeval *tv)
{
	ESP_LOGI(TAG, "Notification of a time synchronization event\n");
#if PRJ_RTC_TYPE != 0
	app_rtc_set_date();  // sync time to clockIc; 
#endif

	time_t now;
	struct tm timeinfoA;
	time(&now);
	setenv("TZ", "CST-8", 1);
	tzset();
	localtime_r(&now, &timeinfoA);
	
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	g_hasClockCorrectionByNtp = true;
	int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
	ESP_LOGW(TAG, "time_us = %lld  ", time_us);
}
#endif

/* 用 ntp 同步时间
 */
void  sys_time_sync_init_ex(void) 
{
	ESP_LOGI(TAG, "%s A", __func__);
	return;
}


void zb_initForNxCnctSyncTm(void) {
	g_lastNtpSyncTmSinceNetCnct = 0;
	g_ntpServerNo = 0;
	return;
}

bool zb_isTimeToSyncTime(void) {
	if (g_hasClockCorrectionByNtp)
		return false;
	int32_t timeMsSinceBoot = zb_get_time_ms_since_boot();
	if ((g_lastNtpSyncTmSinceNetCnct != 0) && (timeMsSinceBoot > g_lastNtpSyncTmSinceNetCnct + 12000)) {
		return true;
	}
	return false;
}

//----------------------------------------------
/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
#ifdef C_PC_SIM
size_t strlcpy(char *dst, const char *src, size_t dstBufLen)
{
	const char *osrc = src;
	size_t nleft = dstBufLen;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dstBufLen != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}
	return(src - osrc - 1);	/* count does not include NUL */
}
#endif

char * zb_strdup(const char * s)
{
	if (!s)
		return NULL;

	int len = strlen(s);
	char * pNew = heap_caps_malloc_dflt(len + 1);
	memcpy(pNew, s, len + 1);
	return pNew;
}

/** 字串替换 
 * @param pcSrc 原字串;  pcDest 目标 buffer;  pcSubOld 要被换掉的子串;  pcSubNew 替换成新的子串; 
 * @return  length of new string, >=0 means success 
 */
int zb_str_replace(const char * pcSrc, char * pcDest, int bufLen, const char * pcSubOld, const char * pcSubNew)
{
	if (!pcSrc || !pcDest || !pcSubOld || !pcSubNew)
		return -1;
	if (!pcSubOld[0] || bufLen < 2)
		return -3;

	int iPosI = 0, iPosO = 0;
	int iSrcLen = strlen(pcSrc);
	const int16_t subOldLen = strlen(pcSubOld);
	const int16_t subNewLen = strlen(pcSubNew);
	char chOld1st = pcSubOld[0];
	while (iPosO + 1 < bufLen) {
		char ch = pcSrc[iPosI];
		if (!ch) break;
		int16_t readLen = 1,  wrtLen = 1;
		if ((chOld1st == ch) && (memcmp(pcSrc + iPosI, pcSubOld, subOldLen) == 0)) { // match susStr
			readLen = subOldLen;
			wrtLen  = subNewLen;
			if (iPosO + subNewLen >= bufLen)  // the buff has no room to write subNew str
				break;
			memcpy(pcDest + iPosO, pcSubNew, wrtLen);
		}
		else {
			readLen = wrtLen = 1;
			pcDest[iPosO] = ch;
		}
		iPosO += wrtLen;
		iPosI += readLen;
	}
	pcDest[iPosO] = '\0';
	return 	iPosO ;
}


/* 把数据 buffer 转为相应的 hex 码字串格式, 形如 8a 75 c9 32 ... 
 */
int zb_bin2hex(const char *bin, size_t len, char* hexBuf, int bufSize) {
	int iDst = 0;
	size_t  i;

	if (bin == NULL || len == 0)
		return -1;

	for (i = 0; i < len; i++) {
		unsigned char ch;
		int nibbleL;
		int nibbleH;
		if (iDst + 4 >= bufSize) { // 
			break;
		}
		ch = bin[i];
		nibbleH = (ch >> 4);
		nibbleL = (ch & 0x0F);
		
		hexBuf[iDst++] = (nibbleH < 10 ? 0x30 + nibbleH : 'A' + (nibbleH - 10));
		hexBuf[iDst++] = (nibbleL < 10 ? 0x30 + nibbleL : 'A' + (nibbleL - 10));
		
		if ((i + 1) % 16 == 0) {
			hexBuf[iDst++] = '\n';
		}
		else {
			hexBuf[iDst++] = 0x20;
			if ((i + 1) % 8 == 0) {
				hexBuf[iDst++] = 0x20;
			}
		}
	}
	if (iDst > 0 && hexBuf[iDst - 1] != '\n') {  // 
		hexBuf[iDst - 1] = '\n';
	}
	hexBuf[iDst] = '\0';
	return iDst;
}


//--------------------------------------
static const char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64_url_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static char * base64_gen_encode(const unsigned char *src, size_t len,
				size_t *out_len, const char *table, int add_pad)
{
	char *out, *pos;
	const unsigned char *end, *in;
	size_t olen;
	int line_len;

	if (len >= SIZE_MAX / 4)
		return NULL;
	olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	if (add_pad)
		olen += olen / 72; /* line feeds */
	olen++; /* nul termination */
	if (olen < len)
		return NULL; /* integer overflow */
	out = heap_caps_malloc_dflt(olen);
	if (out == NULL)
		return NULL;

	end = src + len;
	in = src;
	pos = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = table[(in[0] >> 2) & 0x3f];
		*pos++ = table[(((in[0] & 0x03) << 4) | (in[1] >> 4)) & 0x3f];
		*pos++ = table[(((in[1] & 0x0f) << 2) | (in[2] >> 6)) & 0x3f];
		*pos++ = table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
		if (add_pad && line_len >= 72) {
			*pos++ = '\n';
			line_len = 0;
		}
	}

	if (end - in) {
		*pos++ = table[(in[0] >> 2) & 0x3f];
		if (end - in == 1) {
			*pos++ = table[((in[0] & 0x03) << 4) & 0x3f];
			if (add_pad)
				*pos++ = '=';
		} else {
			*pos++ = table[(((in[0] & 0x03) << 4) |
					(in[1] >> 4)) & 0x3f];
			*pos++ = table[((in[1] & 0x0f) << 2) & 0x3f];
		}
		if (add_pad)
			*pos++ = '=';
		line_len += 4;
	}

	if (add_pad && line_len)
		*pos++ = '\n';

	*pos = '\0';
	if (out_len)
		*out_len = pos - out;
	return out;
}



/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
char * zb_base64_encode(const void *src, size_t len, size_t *out_len)
{
	char * base64Str = base64_gen_encode(src, len, out_len, base64_table, 1);
	if (base64Str) {
		char * pTmp = strchr(base64Str, '\n');
		if (pTmp) {
			(*pTmp) = '\0';
		}
	}
	return base64Str;
}


char * zb_base64_url_encode(const void *src, size_t len, size_t *out_len)
{
	return base64_gen_encode(src, len, out_len, base64_url_table, 0);
}

//------------------------------
#if 1
static const uint32_t crc32_le_table[256] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
    0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
    0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
    0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
    0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
    0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
    0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
    0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
    0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
    0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
    0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
    0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
    0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,

    0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
    0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
    0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
    0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
    0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
    0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
    0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
    0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
    0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
    0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
    0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
    0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
    0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

uint32_t  esp_rom_crc32_le(uint32_t crc, uint8_t const * buf, uint32_t len)
{
    uint32_t i;
    crc = ~crc;
    for(i=0;i<len;i++){
        crc = crc32_le_table[(crc^buf[i])&0xff]^(crc>>8);
    }
    return ~crc;
}
#endif


extern void MD5Init(struct MD5ContextEx * context);
extern void MD5Update(struct MD5ContextEx * context, unsigned char const * buf, unsigned len);
extern void MD5Final(unsigned char digest[16], struct MD5ContextEx * context);

void MD5InitEx(struct MD5ContextEx * context)
{
#if 0
	MD5Init(context);
#endif
}

void MD5UpdateEx(struct MD5ContextEx * context, unsigned char const * buf, unsigned len)
{
#if 0
	MD5Update(context, buf, len);
#endif
}

void MD5FinalEx(unsigned char digest[16], struct MD5ContextEx * context)
{
#if 0
	MD5Final(digest, context);
#endif
}


//=========================================================
static bool file_exists(const char *file_path)
{
    struct stat buffer;
    return (stat(file_path, &buffer) == 0);
}

#ifndef C_PC_SIM

int mkdir_p(const char *dir, mode_t mode)
{
	const char *subdir = dir;
	int len;
	char buffer[256];
//	ESP_LOGI(TAG, "%s [%d] dir:%s \n", __FUNCTION__,__LINE__, dir);
	while ((subdir = strchr(subdir, '/')) != NULL)
	{
//		ESP_LOGI(TAG, "%s [%d] subdir: %p, dir: %p \n", __FUNCTION__,__LINE__, subdir, dir);
		len = subdir - dir;
		if (len) 
		{
			memcpy(buffer, dir, len);
			buffer[len] = '\0';
			
//			ESP_LOGI(TAG, "%s [%d] dir: %s \n", __FUNCTION__,__LINE__, buffer);
			struct stat s = {0};
			bool directory_exists = stat(buffer, &s) == 0;
			if (!directory_exists)
			{
//			  ESP_LOGI(TAG, "%s [%d] make dir: %s \n", __FUNCTION__,__LINE__, buffer);
			  if (mkdir(buffer, mode) != 0) 
			  {
				  ESP_LOGE(TAG, "mkdir failed with errno: %s\n", strerror(errno));
				  return(0);
			  }
			}
			
		}
		++subdir;
	}
	ESP_LOGI(TAG, "%s [%d] mkdir: %s \n", __FUNCTION__,__LINE__, dir);
	return mkdir(dir, mode);
}

FILE*  download_create_file(char * pathname) 
{
    char *directory;
    char *file_path = pathname;
//	int i;
	char dir_buf[256];
	int len = 0;
	ESP_LOGI(TAG, "%s [%d] pathname:%s \n", __FUNCTION__,__LINE__, pathname);

	directory = strrchr(pathname, '/');  //从右向左找第一个'/'
	len = directory - pathname;
	memcpy(dir_buf, pathname, len);
	dir_buf[len] = '\0';
	
//	ESP_LOGI(TAG, "%s [%d] dir: %s len:%d\n", __FUNCTION__,__LINE__, dir_buf, len);
	FILE *f = NULL;

	if (!file_exists(file_path)) 
	{
		
		mkdir_p(dir_buf, 0777);
		ESP_LOGI(TAG, "Creating file：%s", file_path);
		f = zb_fopen(file_path, "w+b");
		if (f == NULL) 
		{
		  ESP_LOGE(TAG, "Failed to open file for writing");
		  return NULL;
		}
		ESP_LOGI(TAG, "%s [%d] open :%p \n", __FUNCTION__,__LINE__, f);
//	  fprintf(f, "Hello World!\n");
//	  fclose(f);
		return(f);	
	}
	else
	{
		ESP_LOGI(TAG, "%s [%d] open : %s", __FUNCTION__,__LINE__, file_path);
		f = zb_fopen(file_path, "w+b");
	}
	
	if(f)
	{
		ESP_LOGI(TAG, "%s [%d] open :%p \n", __FUNCTION__,__LINE__, f);
		return(f);	
	}
	return NULL;

}
#endif
//---------------------------------------------
// 删除目录内所有文件 
int zb_delete_directory(const char * d_path) 
{
#define CHR_BE_ITERED_MARK  '\x7c'  //  '|'

#define DIR_PATH_PTR_BUF_CAPACITY_ST 30
#define MY_PATH_BUF_LEN 510
	char * pathname = NULL;
	int bufCapacity = DIR_PATH_PTR_BUF_CAPACITY_ST;

	ESP_LOGI(TAG,"%s [%d] d_path: %s\n", __FUNCTION__, __LINE__, d_path);

	char ** ppcFolders = (char **)heap_caps_malloc_dflt(sizeof(void *) * bufCapacity);
	memset(ppcFolders, 0, sizeof(void *) * bufCapacity);
	pathname = (char *)heap_caps_malloc_dflt(MY_PATH_BUF_LEN);
	ppcFolders[0] = (char *)heap_caps_malloc_dflt(MY_PATH_BUF_LEN + 1);
	int16_t folderNum = 1;
	ppcFolders[0][0] = 0x20;  // mark 0 means not be iter.
	strlcpy(ppcFolders[0] + 1, d_path, MY_PATH_BUF_LEN);
	int iFuncRet = -1;

	while (1) {
		if (folderNum <= 0) {
			iFuncRet = 0;  break;
		}
		if (ppcFolders[folderNum - 1][0] != '\x20') { // dir has be itered
			int ret = zb_rmdir(ppcFolders[folderNum - 1] + 1);
			if (ret != 0) {
				ESP_LOGI(TAG, "rmdir fail  %d", ret);
				iFuncRet = -3;  break;
			}
			heap_caps_free(ppcFolders[folderNum - 1]);  ppcFolders[folderNum - 1] = NULL;
			folderNum--;
		}
		else {
			lv_fs_dir_t dir;
			lv_fs_res_t res;
			const char * pcFolderPath = ppcFolders[folderNum - 1] + 1;
			if(lv_fs_dir_open(&dir, pcFolderPath) != LV_FS_RES_OK) {
				ESP_LOGI(TAG,"Cannot open directory: %s\n", pcFolderPath);
				iFuncRet = -5;  break;
			}

			ppcFolders[folderNum - 1][0] = CHR_BE_ITERED_MARK; // mark dir has be itered.
			while (1) {
				char filename[CONFIG_FATFS_MAX_LFN + 1];
				filename[0] = 0;
				res = lv_fs_dir_read(&dir, filename); 
				if (res != LV_FS_RES_OK) {
					break;
				}

				if (0 == filename[0]) {
					ESP_LOGI(TAG,"Not more files to read!");
					break;
				}
				
				if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
					continue;

				int skipLen = ('/' == (*filename) ? 1 : 0);
				snprintf(pathname, MY_PATH_BUF_LEN, "%s/%s", pcFolderPath, filename + skipLen);
				if (*filename == '/') //为目录
				{
					if ((folderNum >= bufCapacity) && (bufCapacity < 4000)) {
						int diff = (bufCapacity / 2);
						if (diff < 12) {
							diff = 12;
						}
						int bufCapacityOld = bufCapacity;
						bufCapacity += diff;
						ppcFolders = (char **)heap_caps_realloc_dflt(ppcFolders, sizeof(void *) * bufCapacity);
						memset(ppcFolders + bufCapacityOld, 0, sizeof(void *) * diff);
					}
					if (folderNum < bufCapacity) {
						ppcFolders[folderNum] = (char *)heap_caps_malloc_dflt(MY_PATH_BUF_LEN + 1);
						ppcFolders[folderNum][0] = 0x20;  // mask not be iter.
						strlcpy(ppcFolders[folderNum] + 1, pathname, MY_PATH_BUF_LEN);
						folderNum++;
					}
				}
				else {  // is a file 
					ESP_LOGI(TAG, "zb_remove: %s", pathname);
					int ret = zb_remove(pathname);
					/* if (ret != 0) {
						zb_chmod(pathname, 0777);
						int ret = zb_remove(pathname);
					} */
				}
			}

			lv_fs_dir_close(&dir);
		}

		//ESP_LOGI(TAG, "%s [%d] zb_rmdir: [%s]", __FUNCTION__, __LINE__, d_path);
		//zb_rmdir(d_path);
	} while (0);

	if (pathname != NULL) {
		heap_caps_free(pathname);  pathname = NULL;
	}
	if (ppcFolders != NULL) {
		int16_t no = 0;
		for (no = 0; no < folderNum; no++) {
			if (ppcFolders[no]) {
				heap_caps_free(ppcFolders[no]);	ppcFolders[no] = NULL;
			}
		}
		heap_caps_free(ppcFolders);  ppcFolders = NULL;
	}
	if (iFuncRet) {
		ESP_LOGI(TAG, "delDirEx fail %d", iFuncRet);
	}
	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "fail to delDir, %d", iFuncRet);
	}
	return iFuncRet;
}

int zb_del_dir(const char* path) 
{
    char *dir = path;
    zb_delete_directory(dir);
    ESP_LOGI(TAG, "Directory deleted.");
    return 0;
}

//--------------------------------------------
void zb_mem_print(void) 
{
#if 0
	char buffer[128];
	snprintf(buffer, sizeof(buffer), " Biggest /	 Free /	Total\n"
	"\t SRAM  : [%8d / %8d / %8d]\n"
	"\t PSRAM : [%8d / %8d / %8d]",
	heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
	heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
	heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
	heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
	heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
	heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
	ESP_LOGW("UMEM", "%s", buffer);	
	
#endif		
}


//--------------------------------------------

