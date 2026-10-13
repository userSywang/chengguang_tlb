#ifndef _PLAYER_LIST_H
#define _PLAYER_LIST_H

#include "player.h"
#include "../data_parse/analyze_listening.h"


extern int addMusicCol_collect(char *path, char *name, uint16_t num, bool mode);
extern int hasSameMusicName(const char *musicName, bool mode);
extern int addRecord(char *path, char *name, uint16_t num);
extern uint8_t player_backstage_flag;

#endif
