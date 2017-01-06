/*
�������յ�����ܹ���

author:taoliu
time: 2016.11


������ְ����������յ��ܹ��̣��������Ե�ָ�����õ�ʵ�ʵĻ���list��
���ʵ�ʻ��յ������ȵȡ�


*/

#ifndef GC_H
#define GC_H


typedef struct gc_list_data
{
	int64_t gc_containerid;
	int32_t gc_chunk_shift;

};

typedef struct gc_list_type
{
	struct gc_list_data gc_data;
	struct gc_list_type *next;
};

struct gc_list_type *gchead;

//static int64_t gc_count=0;

void gc_list_AddEnd (struct gc_list_data *gc_data);

void start_garbage_collection();

void garbage_collection_method_selection();

void gc_reference_time_map();

void gc_reference_count();

void gc_mark_and_sweep();

void gc_reference_time_map();

int64_t gc_reference_time_map_alone(int deleteversion);

int64_t gc_reference_time_map_patch(int deleteversion);




#endif                                      