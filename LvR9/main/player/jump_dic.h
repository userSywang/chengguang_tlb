#ifndef _JUMP_DIC_H_
#define _JUMP_DIC_H_

#include <stdint.h>

/** 进入跳查版面
 *  输入字串(现在支持英文的)要通过 zb_global_str_write(pcText) 来写入.
 */
int dictScreen_init(void);


/** 释放跳查版面 
 */
void dictScreen_del(void);


#define dictScreen_deinit()  dictScreen_del()


extern int zb_global_str_write(const char * pcText);

extern int play_text_return_init(int parm);

#endif


