

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
//首先定义id_shift记录索引中的container，以及对应的偏移量

#define SHIFT 5
#define MASK 0x1F

int current_bv;

int32_t globle_shift;
GHashTable *id_shift_hash;

struct _id_shift_type* ishead;

int64_t container_count_start;
int64_t container_count_end;

//GHashTable* id_shift_hash;

//struct _id_shift_type* ishead;

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
//在id_shift的链表中追加结点；
//void _id_shift_AddEnd (struct _id_shift id_shift_add_data);



void destory_id_shift_hash();

void close_id_shift_and_hashtable();

struct _id_shift_type *_id_shift_AddEnd (struct _id_shift id_shift_add_data);
//void _id_shift_AddEnd (struct _id_shift id_shift_add_data);
//2016.11.17.根据给定的container id查找id_shift,找到对应container初始的偏移量。
//_id_shift_type *id_shift_find_node(_id_shift_type *head,containerid id);

int get_shift_by_id(int64_t id);

int get_next_shift_by_id(int64_t id);

//释放空间
void Destory_id_shift();






//============================index_bit===========================





//初始化一个数组的n位的所有位为0；
//size为 index中chunk的数目/32，
//index_bit_size的大小表示chunk的数目，采用4TB的数据为准

#define index_bit_size  1<<26

int32_t index_bit[index_bit_size];

void init_index_bit();


//把某一位设置为1
void set_index_bit(int n);

//得到index_bit中的某一位是1还是0；
int get_index_bit(int n);

int check_index_bit_equal_one(int start,int end);




//============================container_bit_table=======================
#define container_size 1<<10
int CBT[container_size];

int current_bv;

int64_t contaienr_count_start;
int64_t contaienr_count_end;

void init_container_bit_table(int backupversion);
//得到版本新增加的container的数目；
int64_t get_container_count_add();

//初始化container_bit_table
//不初始化，直接用二维数组表示
//void init_container_bit_table();


int64_t get_storage_container_number();

void set_container_bit_table(int n, int zero_or_one);


//更新container_bit_table
//新增引用为01
//部分引用为10
//完全引用为11

void update_container_bit_table_and_RTM(int backupversion);

int get_attribute(int64_t id);

void update_container_bit_table_attribute(int shift,int attribute);


//两个备份版本的container_bit_table进行或操作

int* merge_container_bit_table(int* v1,int* v2);

//把二位数组的第一位用来存储备份版本数，16位或者32位也无所谓。但是要注意边界。
//得到container_bit_table
int* get_container_bit_table(int backupversion);

//2016.11.18把container bit table以文件的形式写到磁盘上，每个版本都可以后缀添加
void write_container_bit_table_to_disk();

void updata_container_bit_table_attribute(int shift,int attribute);

void show_cbt();

int* get_newest_container_bit_map(int backupversion);




//============================reference_time_map=======================
//初始化reference_time_map
//以一个链表中嵌套另一个链表的方式去初始化reference_time_map
//

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


//void change_RTM_backupversition(RTMlist *head,int backupversion,int n,containerid id);

//int *get_RTMarray_accord_containerid(containerid id,RTMlist *head);



//得到实际的reference_time_map
//一定要注意，container bit table的顺序和RTM的顺序是一致的。

int get_CBT(int n);

int get_CBT_array(int n,int *array);

void update_RTM_to_same_backupversion(int n,int backupversion);



void check_last_container_bit_table(GSequence *seq);
void get_real_reference_time_map();




void check_write_cce();

int get_container_bit_end();
void write_container_count_end_to_disk();
void  read_RTM_from_disk_in_gc(int broad);
void Destory_RTM();



#endif