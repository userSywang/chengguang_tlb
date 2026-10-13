#ifndef _BOOK_DATA_MANAGE_H
#define _BOOK_DATA_MANAGE_H
#pragma once

#define TITLE_MAX_LENGTH	120

typedef struct {
	char grade;
	char book_id[80];
	char book_title[TITLE_MAX_LENGTH];
} book_version_t;

//typedef struct
//{
//	uint16_t num;
//	book_version_t* pbv;
//} down_book_t;

enum enum_book_op
{
	BOOK_DEFAULT = 0,
	BOOK_SELECT = 1,
	BOOK_DELETE = 2
};
typedef struct bv_node {
	book_version_t data;
	struct bv_node* next;
} bv_node;

#endif

