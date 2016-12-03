/*
垃圾回收的设计总过程

author:taoliu
time: 2016.11


这个部分包括垃圾回收的总过程，包括策略的指定，得到实际的回收list；
最后实际回收的数量等等。


*/

#ifndef GC_H
#define GC_H

//垃圾回收的触发过程
void gc_trigger();

//开始垃圾回收过程；
void start_garbage_collection();

//初始化
void gc_init();

//采用reference_time_map的方式进行垃圾回收
long gc_reference_time_map(deleteversion);

//
list *get_RTM_check_list();

void check_RTM();

//批量回收
long patch_gc(deleteversion);


//删除数据
void delete_data();

//删除元数据

void delete_meta_data(delete_list);

void garbage_collection_free() 



#endif                                      