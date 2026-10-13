//======================================================[]
//部分供外部调用的函数
//======================================================[]
#ifndef _COM_VOL_H_
#define _COM_VOL_H_

#ifdef __cplusplus
extern "C" {
#endif 

extern void lv_com_vol(int vol);

extern bool is_com_vol_shown();

extern int volume_incOrDec(int diff, int extra);


extern void vol_box_del(void);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_COM_VOL_H_
//======================================================[]
//end.
//======================================================[]
