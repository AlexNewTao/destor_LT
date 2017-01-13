

/*
垃圾回收的更新container_bit_table模块
垃圾回收的更新reference_time_map模块

author:taoliu
time: 2016.11.25

主要内容包括
id_shift的更新，记录chunk重复的情况，container_bit_table的更新，container_bit_table之间的相互操作；
container_bit_table写磁盘；container_bit_table从磁盘中读出来；
reference_time_map的初始化，更新，得到实际gc的reference_time_map。


*/


#ifndef GC_RTM_H
#define GC_RTM_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

//=======================id_shift======================

#define SHIFT 5
#define MASK 0x1F

int current_bv;

int32_t globle_shift;
GHashTable *id_shift_hash;

struct _id_shift_type* ishead;

int64_t container_count_start;
int64_t container_count_end;

struct RTMdata* RTMhead;

struct _id_shift
{
	int64_t container_id;
	int32_t chunk_shift;
	int32_t chunk_num;

};

struct _id_shift_type
{
	struct _id_shift id_shift_data;
	struct _id_shift_type *next;
};


void init_id_shift();


void destory_id_shift_hash();

void close_id_shift_and_hashtable();

struct _id_shift_type *_id_shift_AddEnd (struct _id_shift id_shift_add_data);

int get_shift_by_id(int64_t id);

int get_next_shift_by_id(int64_t id);

void Destory_id_shift();






//============================index_bit===========================


#define index_bit_size  1<<26

int32_t index_bit[index_bit_size];

void init_index_bit();



void set_index_bit(int n);


int get_index_bit(int n);

int check_index_bit_equal_one(int start,int end);




//============================container_bit_table=======================
#define container_size 1<<10
int CBT[container_size];

int current_bv;

int64_t contaienr_count_start;
int64_t contaienr_count_end;

void init_container_bit_table(int backupversion);

int64_t get_container_count_add();




int64_t get_storage_container_number();

void set_container_bit_table(int n, int zero_or_one);


//更新container_bit_table
//新增引用为01
//部分引用为10
//完全引用为11

void update_container_bit_table_and_RTM(int backupversion);

int get_attribute(int64_t id);

void update_container_bit_table_attribute(int shift,int attribute);




int* merge_container_bit_table(int* v1,int* v2);


int* get_container_bit_table(int backupversion);


void write_container_bit_table_to_disk();

void updata_container_bit_table_attribute(int shift,int attribute);

void show_cbt();

int* get_newest_container_bit_map(int backupversion);




//============================reference_time_map=======================

struct RTMdata
{
	int64_t id;
	int16_t len;
	int16_t *rtm;
	struct RTMdata* next;
};



void Init_RTM();

void read_RTM_from_disk() ;

void write_RTM_to_disk() ;

void Init_RTMlist();

void show_RTM();

void RTMlist_AddEnd (struct RTMdata* rtmdata);

void update_reference_time_map(int64_t id);

int* get_newest_container_bit_table(int last);



int get_CBT(int n);

int get_CBT_array(int n,int *array);

void update_RTM_to_same_backupversion(int n,int backupversion);


void check_last_container_bit_table(GSequence *seq,int quit);

void get_real_reference_time_map();

int* get_first_container_bit_table();


void check_write_cce();

int get_container_bit_end();
void write_container_count_end_to_disk();
void  read_RTM_from_disk_in_gc(int broad);
void Destory_RTM();



#endif