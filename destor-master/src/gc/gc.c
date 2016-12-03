

/*
垃圾回收的设计总过程

author:taoliu
time: 2016.11


这个部分包括垃圾回收的总过程，包括策略的指定，得到实际的回收list；
最后实际回收的数量等等。


*/

// 开始垃圾回收函数
#include <stdio.h>
#include <stdlib.h>
#include "gc_rth.h"



typedef struct gc_list_data
{
	int64_t gc_containerid;
	int32_t gc_chunk_shift;

};

typedef struct gc_list_type
{
	struct gc_list_data gc_data
	struct gc_list_type *next;
};

static int64_t gc_count=0;

gc_list_type *gc_list_AddEnd (gc_list_type *head,gc_list_data gc_data)
{
    gc_list_type *node,*htemp;
    if (!(node=(gc_list_type *)malloc(sizeof(gc_list_type))))//分配空间
    {
        printf("内存分配失败！\n");
        return NULL;
    }
    else
    {
        node->gc_list_data=gc_data;//保存数据
        node->next=NULL//设置结点指针为空，即为表尾；
        if (head==NULL)
        {
            head=node;
            return head;
        }
        htemp=head;
        while(htemp->next!=NULL)
        {
            htemp=htemp->next;
        }
        htemp->next=node;
        return head;
    }
}




void start_garbage_collection()
{
	garbage_collection_method_selection();

}



//  3 选择不同的垃圾回收机制，给其他的垃圾回收方法留有接口

void garbage_collection_method_selection()
{
	printf("choose the method to garbage collection\n");
	printf("1:means reference_time_map  method\n");
	printf("2:means reference_count method\n");
	printf("3:means mark_and_sweep  method\n");
	int choose;
	scanf("%d",choose);
	switch(choose)
	{
		case 1:
			gc_reference_time_map();
		case 2:
			gc_reference_count();
		case 3:
			gc_mark_and_sweep();
		defalut:
			printf("worry choose garbage collection method!\n");
	}

}

//采用RTM方法进行垃圾回收
long gc_reference_time_map(int deleteversion)
{
	int *cbt;
	cbt=get_container_bit_table();//得到删除版本的container_bit_map

	RTMlist*  rtm;
	rtm=get_real_reference_time_map();//得到实际的RTM
	
	gc_list_type* gc_head=NULL;

	//gc_list_type *head1;

	RTMlist* rtm_head;

	while(rtm_head!=NULL)
	{
		int i=0;
		while(rtm_head->RTMdata.rtm[i]!=-1)//
		{
			if (rtm_head->RTMdata.rtm[i++]==deleteversion)
			{

				head1=gc_list_AddEnd(head1,rtm_head->RTMdata);
				gc_count++;
			}
		}
		rtm_head = rtm_head->next;
	}
	//根据RTM_check_list,检测RTM，得到实际回收的大小

	return gc_count;

}




//批量回收
long patch_gc_reference_time_map(int deleteversion)
{
	int *cbt;
	cbt=get_container_bit_table();//得到删除版本的container_bit_map

	RTMlist*  rtm;
	rtm=get_real_reference_time_map();//得到实际的RTM
	
	gc_list_type* gc_head=NULL;

	gc_list_type *head1;

	RTMlist* rtm_head=rtm;

	while(rtm_head!=NULL)
	{
		int i=0;
		while(rtm_head->RTMdata.rtm[i]!=0)
		{
			if (rtm_head->RTMdata.rtm[i]<=deleteversion)
			{
				head1=_id_shift_AddEnd(head1,rtm_head->RTMdata);
				gc_count++;
			}
		}
	}
	//根据RTM_check_list,检测RTM，得到实际回收的大小
	return gc_count;
}




