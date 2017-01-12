

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
#include <glib.h>
//#include "../utils/sds.h"
#include "gc_rtm.h"
#include "gc.h"
#include "../destor.h"

static int64_t gc_count=0;
/*struct gc_list_type{
	int64_t gc_containerid;
	int32_t gc_chunk_shift;
	struct gc_list_type *next;
};





void Destory_gc_list()
{
    struct gc_list_type *p = gchead;
    while(gchead!=NULL)
    {
         p = gchead;
         free(p);
         gchead = gchead->next;
    }
}

void gc_list_AddEnd(struct gc_list_type* gc_data)
{

    struct gc_list_type *node,*htemp;
    printf("gc.c 54\n");
    if (!(node=(struct gc_list_type *)malloc(sizeof(struct gc_list_type))))//分配空间
    {
        printf("fail alloc space!\n");	
        return NULL;
    }
    else
    {
    	printf("gc.c 62\n");
    	node->gc_containerid=gc_data->gc_containerid;//保存数据
        node->gc_chunk_shift=gc_data->gc_chunk_shift;//保存数据
        node->next=NULL;//设置结点指针为空，即为表尾；
        printf("gc.c 64\n");
        if (gchead==NULL)
        {
            gchead=node;
          
        }
        else
        {
        	htemp=gchead;
        	printf("gc.c73\n");

        	while(htemp->next!=NULL)
        	{
            	htemp=htemp->next;
            	printf("1");
        	}
        	printf("gc.c 78\n");
        	htemp->next=node;
        }
        printf("gc.c80\n");
    }
}*/

/*void gc_list_AddEnd (struct gc_list_data *gc_data)
{
    struct gc_list_type *node,*htemp;
    if (!(node=(struct gc_list_type *)malloc(sizeof(struct gc_list_type))))//分配空间
    {
        printf("内存分配失败！\n");
        return NULL;
    }
    else
    {
        node->gc_data.gc_containerid=gc_data->gc_containerid;//保存数据
        node->gc_data.gc_chunk_shift=gc_data->gc_chunk_shift;//保存数据
        node->next=NULL;//设置结点指针为空，即为表尾；
        if (gchead==NULL)
        {
            gchead=node;
            //return gchead;
        }
        htemp=gchead;
        while(htemp->next!=NULL)
        {
            htemp=htemp->next;
        }
        htemp->next=node;
        //return gchead;
    }
}*/

void gc_reference_count()
{
	printf("we do garbage in the method of gc_reference_count!\n");
}
void gc_mark_and_sweep()
{
	printf("we do garbage in the method of gc_mark_and_sweep!\n");
}
//  3 选择不同的垃圾回收机制，给其他的垃圾回收方法留有接口

void garbage_collection_method_selection()
{
	printf("choose the method to garbage collection\n");
	printf("1:means reference_time_map  method\n");
	printf("2:means reference_count method\n");
	printf("3:means mark_and_sweep  method\n");
	int choose;
	scanf("%d",&choose);
	switch(choose)
	{
		case 1:
			gc_reference_time_map();
			break;
		case 2:
			gc_reference_count();
			break;
		case 3:
			gc_mark_and_sweep();
			break;
		defalut:
			printf("worry choose garbage collection method!\n");
	}

}

void start_garbage_collection()
{
	garbage_collection_method_selection();

}


void gc_reference_time_map()
{
	int deleteversion;
	printf("please input the versition you want to delete!\n");
	scanf("%d",&deleteversion);

	printf("choose the delete way\n");
	printf("1:means delete one backupversion  \n");
	printf("2:means delete patch!\n");
	int deletway;
	scanf("%d",&deletway);
	if (deletway==1)
	{
		gc_count=gc_reference_time_map_alone(deleteversion);
	
	}
	else if (deletway==2)
	{
		gc_count=gc_reference_time_map_patch(deleteversion);

	}
	else
	{
		printf("worry deletway choose!\n");
	}
	get_delete_message();
	printf("finish garbage collection\n");
}

void get_delete_message()
{
	int64_t size;
	size=gc_count*4/1024;
	printf("garbage collection finished\n");
	printf("the collection  size is %ld MB\n", size);
}

/*//采用RTM方法进行垃圾回收
int64_t gc_reference_time_map_alone(int deleteversion)
{

	get_real_reference_time_map();//得到实际的RTM
	
	show_RTM();

	struct RTMdata *htemp;
	htemp=RTMhead;

	sds gcpath = sdsdup(destor.working_directory);
    gcpath = sdscat(gcpath, "/gc_list.gc");

    FILE *fp;
    if((fp = fopen(gcpath, "a"))) 
    {
		while(htemp!=NULL)
		{
			int i=0;
			//printf("gc111\n");
			while(i<htemp->len)
			{
			//printf("gc444\n");
				if (htemp->rtm[i]==deleteversion)
				{
        			int *s=(int32_t*)malloc(sizeof(int32_t));
        			*s=i+1;
        			fwrite(RTMhead->id, sizeof(int64_t), 1, fp);
        			fwrite(s, sizeof(int32_t), 1, fp);
					gc_count=gc_count+1;
				}
				i=i+1;
			}
		//printf("gc333\n");
		htemp = htemp->next;
		}
	fclose(fp);
	}

	sdsfree(gcpath);
	//printf("gc2222\n");
	//根据RTM_check_list,检测RTM，得到实际回收的大小
	return gc_count;

	//get_delete_message();
}*/

/*int64_t gc_reference_time_map_alone(int deleteversion)
{

	get_real_reference_time_map();//得到实际的RTM
	
	show_RTM();

	int n=get_container_bit_end();
	/*int *ans_arr=(int32_t*)malloc(sizeof(int32_t));
	int k;
	for (k = 0; k < n; k++)
	{
		ans_arr[k]=0;
	}*/
/*	GHashTable *check_arr_hash=g_hash_table_new_full(g_int_hash, g_int_equal, free,NULL);

	int *check_arr=get_merge_container_bit_table(deleteversion);
	
	int i;
	int j=0;
	for (i= 33; i <= n*2; i=i+2)
	{
		if ((get_CBT_array(i,check_arr)==1)||(get_CBT_array(i+1,check_arr)==1))
		{
			int32_t* check_arr_hash_id=(int32_t*)malloc(sizeof(int32_t));
            *check_arr_hash_id=(i-33)/2;;
            g_hash_table_insert(check_arr_hash,check_arr_hash_id,check_arr_hash_id);
		}
	}


	struct RTMdata *htemp;
	htemp = RTMhead;
	while(htemp!=NULL)
	{
		if(g_hash_table_contains(check_arr_hash,&htemp->id))
		{
			htemp = htemp->next;
		}
		else
		{
			int i=0;
			while(i<htemp->len)
			{
				if (htemp->rtm[i]==deleteversion)
				{       		
					gc_count=gc_count+1;
				}
				else
				{
					htemp->rtm[i]=0;
				}
				i=i+1;
			}
			htemp = htemp->next;
		}
	}

	Destory_RTM();
	g_hash_table_destroy(check_arr_hash);
	return gc_count;
}*/

/*int64_t gc_reference_time_map_alone(int deleteversion)
{

	
	
	int n = get_container_bit_end();

	printf("the n is %d\n",n);
	
	int* ans_arr=(int32_t*)malloc(sizeof(int32_t)*n);
	int k;
	for (k = 0; k < n; k++)
	{
		ans_arr[k]=0;
	}

	get_real_reference_time_map();

	printf("222222319 \n");

	show_RTM();
	printf("aaa 324\n");

	printf("111111111111111111\n");
	printf("ccc\n");
	if (deleteversion==0)
	{
		struct RTMdata *htemp;
		htemp = RTMhead;
		while(htemp!=NULL)
		{
			int i=0;
			while(i<htemp->len)
			{
				if (htemp->rtm[i]==deleteversion)
				{       		
					gc_count=gc_count+1;
				}
				else
				{
					htemp->rtm[i]=0;
				}
				i=i+1;
			}
			htemp = htemp->next;
		}
	}
	else
	{
		printf("ee355\n");

		int *new_check_arr=get_merge_container_bit_table(deleteversion);

		int i;
		printf("ee359\n");
		int j=0;
		for (i= 33; i <= n*2; i=i+2)
		{
			if ((get_CBT_array(i,new_check_arr)==1)||(get_CBT_array(i+1,new_check_arr)==1))
			{
            	ans_arr[j]=(i-33)/2;
            	j=j+1;
			}
		}
		printf("ee369\n");

		struct RTMdata *htemp;
		htemp = RTMhead;
		while(htemp!=NULL)
		{
			if(array_contains(htemp->id,ans_arr,n)==1)
			{
				htemp = htemp->next;
			}
			else
			{
				int i=0;
				while(i<htemp->len)
				{
					if (htemp->rtm[i]==deleteversion)
					{       		
						gc_count=gc_count+1;
					}
					else
					{
						htemp->rtm[i]=0;
					}
					i=i+1;
				}
				htemp = htemp->next;
			}
		}

		//free(new_check_arr);

	}
	//free(ans_arr);
	Destory_RTM();
	
	return gc_count;
}


int array_contains(int check,int *array,int border)
{
	int i=0;
	while(i<border)
	{
		if (array[i]==check)
		{
			return 1;
		}
		i++;
	}
	if (i==border)
	{
		return 0;
	}
}*/


int64_t gc_reference_time_map_alone(int deleteversion)
{

	
	
	int n = get_container_bit_end();

	printf("the n is %d\n",n);
/*	
	int *new_check_arr1=get_container_bit_table(1);

	int j;

	for ( j = 1; j < n*2+33; j++)
	{
		printf("%d", get_CBT_array(j,new_check_arr1));
		if (j%32==0)
		{
			printf("\n");
		}
	}*/

/*	printf("\n");
	int *new_check_arr2=get_container_bit_table(2);

	int k;
	for ( k = 1; k < n*2+33; k++)
	{
		printf("%d", get_CBT_array(k,new_check_arr2));
		if (k%32==0)
		{
			printf("\n");
		}
	}*/


	int *new_check_arr3=get_container_bit_table(4);

	printf("\n");
	int s;

	for ( s = 1; s < n*2+33; s++)
	{
		printf("%d", get_CBT_array(s,new_check_arr3));
		if (s%32==0)
		{
			printf("\n");
		}
	}

	printf("\n");

	int *new_check_arr=get_merge_container_bit_table(deleteversion);


	printf("\n");
	int i;

	for ( i = 1; i < n*2+33; i++)
	{
		printf("%d", get_CBT_array(i,new_check_arr));
		if (i%32==0)
		{
			printf("\n");
		}
	}

	printf("\n");

	get_real_reference_time_map();
	//read_RTM_from_disk_in_gc(n);
	//show_RTM();
	/*printf("test 497\n");
	GHashTable *check_arr_hash=g_hash_table_new_full(g_int_hash, g_int_equal, free,NULL);
	printf("test 499\n");
	if (deleteversion==0)
	{
		struct RTMdata *htemp;
		htemp = RTMhead;
		while(htemp!=NULL)
		{
			int i=0;
			while(i<htemp->len)
			{
				if (htemp->rtm[i]==deleteversion)
				{       		
					gc_count=gc_count+1;
				}
				else
				{
					htemp->rtm[i]=0;
				}
				i=i+1;
			}
			htemp = htemp->next;
		}
	}
	else
	{

		int i;
		for (i= 33; i < n*2+33; i=i+2)
		{
			if ((get_CBT_array(i,new_check_arr)==1)||(get_CBT_array(i+1,new_check_arr)==1))
			{
            	int32_t* check_arr_hash_id=(int32_t*)malloc(sizeof(int32_t));
            	*check_arr_hash_id=(i-33)/2;;
            	g_hash_table_insert(check_arr_hash,check_arr_hash_id,check_arr_hash_id);
			}
		}
		struct RTMdata *htemp;
		htemp = RTMhead;
		while(htemp!=NULL)
		{
			if(g_hash_table_contains(check_arr_hash,&htemp->id))
			{
				htemp = htemp->next;
			}
			else
			{
				int i=0;
				while(i<htemp->len)
				{
					if (htemp->rtm[i]==deleteversion)
					{       		
						gc_count=gc_count+1;
					}
					else
					{
						htemp->rtm[i]=0;
					}
					i=i+1;
				}
				htemp = htemp->next;
			}
		}

	}

	Destory_RTM();
	g_hash_table_destroy(check_arr_hash);*/
	return gc_count;
}



/*int64_t gc_reference_time_map_alone(int deleteversion)
{

	get_real_reference_time_map();//得到实际的RTM
	
	show_RTM();

	int *check_arr=get_merge_container_bit_table(deleteversion);
	GSequence *check_gsequence=g_sequence_new(free);
	int n=get_container_bit_end();
	int i;
	for (i= 33; i <= n*2; i=i+2)
	{
		if ((get_CBT_array(i,check_arr)==1)||(get_CBT_array(i+1,check_arr)==1))
		{
			int *s=(int32_t*)malloc(sizeof(int32_t));
			*s=(i-33)/2;
            g_sequence_append(check_gsequence,s);
		}
	}

	struct RTMdata *htemp;
	htemp = RTMhead;
	int j=0;
	while(htemp!=NULL)
	{
		GSequenceIter* iter=g_sequence_get_begin_iter(check_gsequence);
    	GSequenceIter* end = g_sequence_get_end_iter(check_gsequence);
    	while(iter!=end)
    	{
    		int *pos=g_sequence_get(iter);
    		if (j==*pos)
    		{
    			break;
    		}
    		iter=g_sequence_iter_next(iter);
    	}
		if (iter!=end)
		{
			htemp = htemp->next;
			j=j+1;
		}
		else
		{
			int i=0;
			while(i<htemp->len)
			{
				if (htemp->rtm[i]==deleteversion)
				{       		
					gc_count=gc_count+1;
				}
				else
				{
					htemp->rtm[i]=0;
				}
				i=i+1;
			}
			htemp = htemp->next;
			j=j+1;
		}
	}
	g_sequence_free(check_gsequence);

	Destory_RTM();

	return gc_count;
}*/

/*struct gc_list_type
{
	int64_t gc_containerid;
	GSequence *gc_gsequence;
	struct gc_list_type *next;
};*/

/*int64_t gc_reference_time_map_alone(int deleteversion)
{
	//int *cbt;
	//cbt=get_container_bit_table(deleteversion);//得到删除版本的container_bit_map

	get_real_reference_time_map();//得到实际的RTM
	
	show_RTM();

	GSequence *gc_gsequence=g_sequence_new(free);

	struct RTMdata *htemp;
	htemp = RTMhead;
	while(htemp!=NULL)
	{
		int i=0;
		
		while(i<htemp->len)
		{
			
			if (htemp->rtm[i]==deleteversion)
			{
				
				struct gc_list_type *gc_data=(struct gc_list_type *)malloc(sizeof(struct gc_list_type));
				
				gc_data->gc_containerid=htemp->id;
				
				gc_data->gc_chunk_shift=i+1;
				
				g_sequence_append(gc_gsequence,gc_data);
				
						       		
				gc_count=gc_count+1;
			}
			i=i+1;
		}
		
		
		htemp = htemp->next;
	}
	

	g_sequence_free(gc_gsequence);
	
	return gc_count;
}
*/


/*int64_t gc_reference_time_map_alone(int deleteversion)
{
	//int *cbt;
	//cbt=get_container_bit_table(deleteversion);//得到删除版本的container_bit_map

	get_real_reference_time_map();//得到实际的RTM
	
	show_RTM();

	GSequence *gc_gsequence=g_sequence_new(free);

	struct RTMdata *htemp;
	htemp = RTMhead;
	while(htemp!=NULL)
	{
		int i=0;
		printf("gc.c 297\n");
		
		while(i<htemp->len)
		{
			printf("gc.c 301\n");
			if (htemp->rtm[i]==deleteversion)
			{
				printf("gc.c 304\n");
				struct gc_list_type *gc_data=(struct gc_list_type *)malloc(sizeof(struct gc_list_type));
				
				gc_data->gc_containerid=htemp->id;
				
				gc_data->gc_chunk_shift=i+1;
				printf("gc.c 310\n");
				g_sequence_append(gc_gsequence,gc_data);
				printf("gc.c 312\n");
						       		
				gc_count=gc_count+1;
			}
			i=i+1;
		}
		printf("gc333\n");
		sleep(5);
		htemp = htemp->next;
	}
	printf("4444\n");

	g_sequence_free(gc_gsequence);
	
	return gc_count;
}*/
//批量回收

/*int64_t  gc_reference_time_map_patch(int deleteversion)
{
	//int *cbt;
	//cbt=get_container_bit_table(deleteversion);//得到删除版本的container_bit_map
	
	get_real_reference_time_map();//得到实际的RTM

	show_RTM();

	struct RTMdata *htemp;
	htemp = RTMhead;

	while(htemp!=NULL)
	{
		int i=0;
		while(i<htemp->len)
		{
			if (htemp->rtm[i++]<=deleteversion)
			{      		
				gc_count=gc_count+1;
			}
			/*else
			{
				htemp->rtm[i]=0;
			}*/
/*			i=i+1;
		}
		htemp = htemp->next;
	}

	Destory_RTM();
	//根据RTM_check_list,检测RTM，得到实际回收的大小
	return gc_count;
	//get_delete_message();
}*/


int64_t gc_reference_time_map_patch(int deleteversion)
{
	//int *cbt;
	//cbt=get_container_bit_table(deleteversion);//得到删除版本的container_bit_map

	get_real_reference_time_map();//得到实际的RTM
	
	printf("640\n");
	show_RTM();
	printf("642\n");
	struct RTMdata *htemp;
	htemp = RTMhead;
	while(htemp!=NULL)
	{
		int i=0;
			//printf("gc111\n");
		while(i<htemp->len)
		{
			if (htemp->rtm[i]<=deleteversion)
			{       		
				gc_count=gc_count+1;
			}
			else
			{
				htemp->rtm[i]=0;
			}
			i=i+1;
		}
		htemp = htemp->next;
	}

	Destory_RTM();
	//Destory_gc_list();
	
	return gc_count;
}

