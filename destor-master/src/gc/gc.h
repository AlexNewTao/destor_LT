/*
�������յ�����ܹ���

author:taoliu
time: 2016.11


������ְ����������յ��ܹ��̣��������Ե�ָ�����õ�ʵ�ʵĻ���list��
���ʵ�ʻ��յ������ȵȡ�


*/

#ifndef GC_H
#define GC_H



struct gc_list_type *gchead=NULL;

//static int64_t gc_count=0;
void Destory_gc_list();
void gc_list_AddEnd (struct gc_list_type *gc_data);

void start_garbage_collection();

void garbage_collection_method_selection();

void gc_reference_time_map();

void get_delete_message();

void gc_reference_count();

void gc_mark_and_sweep();

void gc_reference_time_map();

int64_t gc_reference_time_map_alone(int deleteversion);

int64_t gc_reference_time_map_patch(int deleteversion);

int array_contains(int check,int *array,int border);


#endif                                      