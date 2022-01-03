#ifndef CHANNEL_MAP_H
#define CHANNEL_MAP_H

//channel_map 保存了描述字到 channel 的映射，这样就可以在事件发生时，根据事件类型对应的套接字快速找到 channel 对象里的事件处理函数

#include "channel.h"

/*
 * channel映射表, key为对应的socket描述字, value 是 channel 对象的地址
 */
struct channel_map 
{
	void **entries;

	int nentries;
};


int map_make_space(struct channel_map *map, int slot, int msize);

void map_init(struct channel_map *map);

void map_clear(struct channel_map *map);

#endif