#ifndef CHANNEL_MAP_H
#define CHANNEL_MAP_H

//channel_map �����������ֵ� channel ��ӳ�䣬�����Ϳ������¼�����ʱ�������¼����Ͷ�Ӧ���׽��ֿ����ҵ� channel ��������¼�������

#include "channel.h"

/*
 * channelӳ���, keyΪ��Ӧ��socket������, value �� channel ����ĵ�ַ
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