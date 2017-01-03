
#include <stdio.h>
#include <stdlib.h>
#include "gc_rtm.h"
#include "../jcr.h"
#include "../utils/sds.h"
#include <glib.h>
#include "../storage/containerstore.h"



#define SHIFT 5
#define MASK 0x1F
#define int16_t short int

static FILE* fp;




//=================================_id_shift============================================



void init_id_shift()
{
    globle_shift=1;
    ishead=NULL;
	/*struct _id_shift_type* head=NULL;
	head=(struct _id_shift_type *)malloc(sizeof(struct _id_shift_type));
	head->id_shift_data.container_id=0;
	head->id_shift_data.chunk_shift=0;
	head->next=NULL;*/
	printf("init_id_shift successfully!\n");
	
}


/*static inline void free_id_hash(int64_t* key){
    free(key);
}*/


void init_id_shift_hash()
{
	id_shift_hash = g_hash_table_new_full(g_int64_hash, g_int64_equal, free,NULL);
	printf("init id_shift_hash successfully!\n");
	
}

void destory_id_shift_hash()
{
    //g_hash_table_remove_all(id_shift_hash);
	//g_hash_table_destroy(id_shift_hash);id_shift_hash = NULL;
}



void Destory_id_shift()
{
    struct _id_shift_type *p = ishead;
    while(ishead!=NULL)
    {
         p = ishead;
         free(p);
         ishead = ishead->next;
    }
}

void close_id_shift_and_hashtable()
{
   
	Destory_id_shift();
 
	destory_id_shift_hash();
   
	printf("close_id_shift_and_hashtable successfully!\n");
}



/*=======================================*/
//在id_shift的链表中追加结点；
struct _id_shift_type * _id_shift_AddEnd (struct _id_shift id_shift_add_data)
{
    struct _id_shift_type *node,*htemp;
    if (!(node=(struct _id_shift_type *)malloc(sizeof(struct _id_shift_type))))
    {
        printf("fail alloc space！\n");
        return NULL;
    }
    else
    {
        node->id_shift_data.container_id=id_shift_add_data.container_id;
        node->id_shift_data.chunk_shift=id_shift_add_data.chunk_shift;
        node->next=NULL;
        if (ishead==NULL)
        {
            ishead=node;
            return ishead;
           
        }
        htemp=ishead;
        while(htemp->next!=NULL)
        {
            htemp=htemp->next;
        }
        htemp->next=node;
        return ishead;
    }
}



//通过id号找到对应的偏移量；

int32_t get_shift_by_id(int64_t id)
{
	struct _id_shift_type *htemp;
    htemp=ishead;
    while(htemp!=NULL)
    {
        if (htemp->id_shift_data.container_id==id)
        {
            return htemp->id_shift_data.chunk_shift;
        }
        htemp=htemp->next;
    }
    printf("can't find the shift!\n");
    return 0;
}

int get_next_shift_by_id(int64_t id)
{
	struct _id_shift_type *htemp;
    htemp=ishead;
    while((htemp!=NULL)&&(htemp->next!=NULL))
    {
        if (htemp->id_shift_data.container_id==id)
        {
            return htemp->next->id_shift_data.chunk_shift;
        }
        htemp=htemp->next;
    }
    if(htemp->next==NULL)
    {
       /* if (htemp->id_shift_data.container_id==id)
        {
            //struct containerMeta* cm =retrieve_container_meta_by_id(id);
            int32_t chunk_len=htemp->id_shift_data.chunk_num;
            return (htemp->id_shift_data.chunk_shift)+chunk_len;
        }*/  
        int ans=htemp->id_shift_data.chunk_shift+htemp->id_shift_data.chunk_num;
        return ans;

    }

    printf("can't find the shift!worry!\n");
    return 0;
}








//============================index_bit======================================


#define SHIFT 5
#define MASK 32

//#define index_bit_size  1<<20

//int32_t index_bit[index_bit_size];
void init_index_bit()
{
	//index_bit[index_bit_size]={0};
	printf("index_bit init successfully!\n");
}


void set_index_bit(int32_t n)
{
    int32_t index_loc;
    int32_t bit_loc=0;
    int check=n%MASK;
    if (check==0)
    {
        index_loc=(n>>SHIFT)-1;
        index_bit[index_loc]|=(1<<31);
    }
    else
    {
        index_loc=n>>SHIFT;
        bit_loc=n%MASK-1;
        index_bit[index_loc]|=(1<<bit_loc);
    }
}



int32_t get_index_bit(int32_t n)
{
    int32_t index_loc;
    int32_t bit_loc;
    int check=n%MASK;
    int flag;
    if (check==0)
    {
        index_loc=(n>>SHIFT)-1;
        //index_bit[index_loc]|=(1<<31);
        flag=index_bit[index_loc]&(1<<31);
    }
    else
    {
        index_loc=n>>SHIFT;
        bit_loc=n%MASK-1;
        flag=index_bit[index_loc]&(1<<bit_loc);
    }
    if (flag==0)
    {
        return 0;
    }
    else
        return 1;
}


int check_index_bit_equal_one(int start,int end)
{
	if (start==end)
	{
		printf("start is equil to end! \n");
	}

 	int i=start;
	int j=start+1;

 /*   int k;
    for (k = start; k <= end; k++)
    {
        printf("%d",get_index_bit(k));
    }
    printf("\n");*/


	while(i<end) 
	{
		if (get_index_bit(i)==1)
		{
			i++;
		}
		else
		{
			break; 
		}
	}

	if (i==end)
	{
		return 2; 
	}
	else if (i==start) 
	{
		while(j<end)
		{
			if(get_index_bit(j)==0) 
			{
				j++;
			}
			else
				break;
		}
		
	}

	if (j==end)
	{
		return 0;
	}
	else
		return 1;

}



//=============================container_bit_table==========================


#define container_size 1<<16
int32_t CBT[container_size];
void init_container_bit_table(int backupversion)
{
	
	//CBT[container_size]={0};
	CBT[0]=backupversion;
	printf("init container_bit_table successfully\n");
}

int32_t get_cbt_bit(int32_t n)
{
    int32_t index_loc;
    int32_t bit_loc;
    int check=n%MASK;
    int flag;
    if (check==0)
    {
        index_loc=(n>>SHIFT)-1;
        //index_bit[index_loc]|=(1<<31);
        flag=CBT[index_loc]&(1<<31);
    }
    else
    {
        index_loc=n>>SHIFT;
        bit_loc=n%MASK-1;
        flag=CBT[index_loc]&(1<<bit_loc);
    }
    if (flag==0)
    {
        return 0;
    }
    else
        return 1;
}






int64_t get_container_count_add()
{
	return container_count_end-container_count_start;
}


int64_t get_storage_container_number()
{
    
    return container_count_end+1;
}



void set_container_bit_table(int n,int zero_or_one)
{
	if (zero_or_one==0)
	{
		return;
	}
	else if (zero_or_one==1)
	{
		int32_t index_loc;
    	int32_t bit_loc=0;
    	int check=n%MASK;
    	if (check==0)
    	{
        	index_loc=(n>>SHIFT)-1;
        	CBT[index_loc]|=(1<<31);
    	}
    	else
    	{
        	index_loc=n>>SHIFT;
        	bit_loc=n%MASK-1;
        	CBT[index_loc]|=(1<<bit_loc);
    	}
	}

}

static void show_RTM()
{
    printf("update reference time map as follow!\n");
    
    struct RTMdata *Rtemp=RTMhead;
    int i=0;
    while(Rtemp!=NULL)
    {
        int16_t k=Rtemp->len;
        int16_t s;
        for (s = 0; s < 20; s++)
        {
            printf("%d",Rtemp->rtm[s]);
        }
        printf("    %d",i);
        printf("ssssssssss\n");
        Rtemp=Rtemp->next;
        i=i+1;
    }
}

//int current_bv;

void update_container_bit_table_and_RTM(int backupversion)
{
    //printf("start update_CBT RTM!\n");
	struct _id_shift_type *htemp;
	htemp=ishead;
	if(CBT[0]!=backupversion)
	{
		printf("the backupversion doesn't match in the CBT\n");
	}
	while(htemp!=NULL)
    {

    	int64_t id=htemp->id_shift_data.container_id;
        //printf("container_count_start is %ld\n", container_count_start);
        //printf("the id is %ld\n",id );
        //printf("the times of first occur!!!!!!\n");

        int cbt_shift=id*2+33;
        int attribute=get_attribute(id);
            
        //printf("attribute is :%d\n",attribute);
        update_container_bit_table_attribute(cbt_shift,attribute);
        
    	if (attribute==1)
    	{
      		update_reference_time_map(id);
    	}
        
        htemp=htemp->next;
       
    }
    //int64_t num_new_add=get_container_count_add();

    int num_new_add=container_count_end-container_count_start;

    //printf(" num_new_add%lld\n", num_new_add);

    //printf("container_count_start is %ld\n", container_count_start);
    //printf("container_count_end is %ld\n", container_count_end);

  	if (num_new_add>0)
    {
    	int i=container_count_start;
    	int add_attribute=3;
        for (; i <container_count_end; i=i+1)
        {
        	int j=i*2+33;
            update_container_bit_table_attribute(j,add_attribute);
        }  
    }
    printf("update container_bit_table successful!\n");
/*
    printf("update container_bit_table is as follow!\n");
    int k;
    for (k = 1; k <=1000; k++)
	{
		printf(" %d",get_cbt_bit(k));
		if (k%32==0)
		{
			printf("\n");
		}
	}
*/
/*	printf("update reference time map as follow!\n");
	//struct RTMdate *Rtemp;
	struct RTMdata *Rtemp=RTMhead;
	int i=0;
	while(Rtemp!=NULL)
	{
		int16_t k=Rtemp->len;
		int16_t s;
		for (s = 0; s < 20; s++)
		{
			printf("%d",Rtemp->rtm[s]);
		}
        printf("    %d",i);
		printf("sssssssssssssss\n");
		Rtemp=Rtemp->next;
        i=i+1;
	}*/
    //show_RTM();
}



//根据container的id号，得到container相对于上一个版本的属性
int get_attribute(int64_t id)
{
	
	int start=get_shift_by_id(id);
    //printf("the star index_bit is %d\n",get_index_bit(start));

    //printf("the start is %d\n", start);
	int end=get_next_shift_by_id(id)-1;
    if ((start==0)||(end==0))
    {
        WARNING("start or end equal to 0,worry!\n");
        assert((start==0)||(end==0));  
    }
	
    //printf("the end is %d\n",end );
	int flag=check_index_bit_equal_one(start,end);
	
    //printf("the flag is %d\n",flag );
	if (flag==0) //全部为0
	{
		return 0;
	}
	if(flag==1)//部分为0
	{
		return 1;
	}
	if(flag==2)//全部为1
	{
		return 2;
	}
}



void update_container_bit_table_attribute(int shift,int attribute)
{
	
	if (attribute==1) //表示部分引用
	{
		set_container_bit_table(shift,1);
		set_container_bit_table(shift+1,0);
	}
	if (attribute==2) //表示完全引用
	{
		set_container_bit_table(shift,1);
		set_container_bit_table(shift+1,1);
	}
	if (attribute==3) //表示新增属性
	{
		set_container_bit_table(shift,0);
		set_container_bit_table(shift+1,1);
	}
}

int* get_container_bit_map(int backupversion)
{
	sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/containerbittable");

    sds cbt_fname = sdsdup(CBTpath);
    cbt_fname = sdscat(cbt_fname, "container_bit_table");

    int container_shift=container_size;

    if((fp = fopen(cbt_fname, "r"))) 
    {
        
        while(CBT[0]!=backupversion)
        {
        	if (fseek(fp,container_shift,SEEK_SET))
        	{
        		fread(&CBT, container_size,1,fp);
        	}

        	container_shift+=container_size;//偏移量往后移动
        }
        
        fclose(fp);
    }

    sdsfree(cbt_fname);
    
    NOTICE("get container bit table of %d backupversion successfully",backupversion);

    return CBT;
}



void write_container_bit_table_to_disk() 
{
    sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/container_bit_table.cbt");

    //sds cbt_fname = sdsdup(CBTpath);
    //cbt_fname = sdscat(cbt_fname, "container_bit_table");

    FILE *fp;
    if((fp = fopen(CBTpath, "w"))) 
    {
        /* Read if exists. */
        fwrite(&CBT, container_size, 1, fp);
        fclose(fp);
    }
    sdsfree(CBTpath);
    NOTICE("write container bit table to disk successfully");
}




int* merge_or_container_bit_table(int backupversion1,int backupversion2)
{
	int *new_container_bit_table;
	
	int *point_container_bit_table1;
	point_container_bit_table1=get_container_bit_map(backupversion1);
	
	int *point_container_bit_table2;
 	int i;
	for (i = 0; i < container_size; i++)
	{
		new_container_bit_table[i]=((point_container_bit_table1[i])|(point_container_bit_table2[i]));
	}
	return new_container_bit_table;
}




int* get_newest_container_bit_map(int backupversion)
{
	sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/containerbittable/");

    sds cbt_fname = sdsdup(CBTpath);
    cbt_fname = sdscat(cbt_fname, "container_bit_table");

    if ((fp = fopen(cbt_fname, "r"))) {
        if (fseek(fp,-container_size,SEEK_END))
        {
        	fread(&CBT, container_size,1, fp);
        }
        
        fclose(fp);
    }

    sdsfree(cbt_fname);

    
    NOTICE("write container bit table successfully");

    return CBT;

}
//===============================reference_time_map=====================

/*struct RTMdata
{
	int64_t id;
	int16_t len;
	int16_t *rtm;
	struct RTMdata* next;
};*/

struct RTMdata *RTMhead;

/*struct RTMlist
{
	RTMdata rtmdata;
	struct RTMlist *next;
};*/

void Init_RTM()
{

	if (CBT[0]==0)
	{
		Init_RTMlist();
	}
	else if(CBT[0]>0)
	{
		read_RTM_from_disk() ;
	}
}

void Init_RTMlist()
{
    RTMhead=NULL;
/*    RTMhead = (struct RTMdata *)malloc(sizeof(struct RTMdata));   
    if(RTMhead== NULL)                      
        printf("FAIL ALLOC SPACE!\n");
    RTMhead->id=0;
    RTMhead->len=0;
    RTMhead->rtm=0;
    RTMhead->next=NULL;
*/
    printf("Init_RTMlist successfully!\n");             
}

void write_RTM_to_disk() 
{
    sds RTMpath = sdsdup(destor.working_directory);
    RTMpath = sdscat(RTMpath, "/reference_time_map.rtm");

    FILE *fp;
    //printf("w1w1w1wsw1w1w1w1w1w\n");
    if((fp = fopen(RTMpath, "w+"))) 
    {
        
    	while(RTMhead!=NULL)
    	{
            //printf("w");
    		fwrite(&RTMhead->id, sizeof(int64_t), 1, fp);
            //printf("RTMid %d ",RTMhead->id);
    		fwrite(&RTMhead->len, sizeof(int16_t), 1, fp);
            //printf("RTMlen %d ",RTMhead->len);
            
            fwrite(RTMhead->rtm, sizeof(int16_t), RTMhead->len, fp);
            
          /*  int i=0;
            while(i<RTMhead->len)
            {
                fwrite(&RTMhead->rtm[i], sizeof(int16_t), 1, fp);
                i++;
            }*/

       /*  上面两种写入数据的方式一样的！
           int j;
            for (j = 0; j < 10; j++)
            {
                printf("RTM %d",RTMhead->rtm[j]);
            }
            printf("\n");*/

    		struct RTMdata* tmp = RTMhead;
    		free(tmp->rtm);
    		free(tmp);
    		RTMhead = RTMhead->next;
    	}
        
        fclose(fp);
    }
    sdsfree(RTMpath);
    NOTICE("write write_RTM_to_disk successfully");
}


void read_RTM_from_disk() 
{
 
    sds RTMpath = sdsdup(destor.working_directory);
    RTMpath = sdscat(RTMpath, "/reference_time_map.rtm");
    FILE *fp;
    RTMhead = NULL;
    if((fp = fopen(RTMpath, "r+"))) 
    {
        //printf("read2read2read2read2read2\n");
    	struct RTMdata* tmp;
        struct RTMdata* tmp1;
        int i=0;
    	while((!feof(fp))&&(i<container_count_start))
    	{
        
            tmp = (struct RTMdata*)malloc(sizeof(struct RTMdata));
            fread(&tmp->id, sizeof(tmp->id), 1, fp);
            fread(&tmp->len, sizeof(tmp->len), 1, fp);
           /* int16_t *array= (int16_t*)malloc(sizeof(int16_t)*(tmp->len));
            fread(array, sizeof(int16_t), tmp->len, fp);
            tmp->rtm=array;*/
            
            tmp->rtm=(int16_t*)malloc(sizeof(int16_t)*(tmp->len));
            
            fread(tmp->rtm, sizeof(int16_t), tmp->len, fp);
           
    		if(RTMhead == NULL){
                tmp1=tmp;	
                RTMhead=tmp1;
    		}else{
                tmp1->next=tmp;
    			tmp1=tmp1->next;
    		}
            tmp1->next=NULL;
            i=i+1;
    	}    
        fclose(fp);
    }
    sdsfree(RTMpath);
/*
    struct RTMdata *Rtemp=RTMhead;
    //int i=0;
    while(Rtemp!=NULL)
    {
        int16_t k=Rtemp->len;
        int16_t s;
        for (s = 0; s < 10; s++)
        {
            printf("%d",Rtemp->rtm[s]);
        }
        //printf("    %d",i);
        printf("rrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");
        Rtemp=Rtemp->next;
        //i=i+1;
    }*/
    NOTICE("read_RTM_from_disk successfully");

    //show_RTM();
}


//单链表的初始化
 



void RTMlist_AddEnd (struct RTMdata* rtmdata)
{
    struct RTMdata *node,*htemp;
    if (!(node=(struct RTMdata *)malloc(sizeof(struct RTMdata))))//分配空间
    {
        printf("内存分配失败！\n");
        return NULL;
    }
    else
    {
        node->id=rtmdata->id;//保存数据
        node->rtm=rtmdata->rtm;//保存数据
        node->len=rtmdata->len;//保存数据

        node->next=NULL;//设置结点指针为空，即为表尾；
        if (RTMhead==NULL)
        {
            RTMhead=node;
          
        }
        else
        {
        	htemp=RTMhead;
        
        	while(htemp->next!=NULL)
        	{
            	htemp=htemp->next;
        	}
        	htemp->next=node;

        }


    }
}

void update_reference_time_map(int64_t id)
{
	int start=get_shift_by_id(id);
	int end=get_next_shift_by_id(id)-1;
	struct RTMdata *tmphead=RTMhead;
	while(tmphead!=NULL)
	{
		if (tmphead->id==id)
		{
			int i;
			for ( i = 0; i < end-start+1; i++)
			{
				if (get_index_bit(start+i))
				{                    
					tmphead->rtm[i]=(int16_t)current_bv;
				}
			}
			
		}
		tmphead=tmphead->next;
	}
}


/*void change_RTM_backupversition(RTMlist *head,int backupversion,int n,containerid id)
{
	int* arr=get_RTMarray_accord_containerid(id,head);
	arr[n]=backupversion;
}*/


/*int *get_RTMarray_accord_containerid(containerid id,RTMlist *head)
{
	RTMlist *htemp;
	htemp=head;
	while(htemp)
	{
		if (htemp->rtmdata->id==id)
		{
			return htemp->rtmdata.rtm;
		}
	}
	return NULL;
}
*/



//得到实际的reference_time_map

//一定要注意，container bit table的顺序和RTM的顺序是一致的。


int* get_newest_container_bit_table(int last)
{
	sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/containerbittable/");

    sds cbt_fname = sdsdup(CBTpath);
    cbt_fname = sdscat(cbt_fname, "container_bit_table");


    if ((fp = fopen(cbt_fname, "r"))) {
        
        if (fseek(fp,-last*(container_size),SEEK_END))
        {
        	fread(&CBT, container_size,1, fp);
        }

        
        fclose(fp);
    }

    sdsfree(cbt_fname);

    
    //NOTICE("write container bit table successfully");
    return CBT;

}


int get_CBT(int n)
{
	int index_loc;
	int bit_loc;
	index_loc=n>>SHIFT;//等价于n/32。
	bit_loc=n&MASK;//等价于n%32。
	return CBT[index_loc]<<bit_loc;
}

int get_CBT_array(int n,int *array)
{
	int index_loc;
	int bit_loc;
	index_loc=n>>SHIFT;//等价于n/32。
	bit_loc=n&MASK;//等价于n%32。
	return array[index_loc]<<bit_loc;
}


//n means the number of n-th container 
void update_RTM_to_same_backupversion(int n,int backupversion,struct RTMdata *RTMhead)
{
	
	struct RTMdata *htemp;
	htemp=RTMhead;
	int i=0;
	while(i<n)
	{
		htemp=htemp->next;
	}
	int j=0;
	while((htemp->rtm[j]!=backupversion)&&(j<htemp->len))
	{
		htemp->rtm[j]=backupversion;
		j++;
	}
}


static int last=1;

static int container_count_gc=0;//用来记录访问的container的数目；

//数组用来存放上次检测为00的位置；

void check_last_container_bit_table(int* a,int n, struct RTMdata *RTMhead)
{
    int check_number=n;
    int* check_arr=get_newest_container_bit_table(last+1);
    int bv=check_arr[0];
    int j=0;
    int *zero_arr;

    int uncheck_container=0;
    while(j<n)
    {

        if ((get_CBT_array(a[j],check_arr)==0)&&(get_CBT_array(a[j]+1,check_arr)==1)||(get_CBT_array(a[j],check_arr)==1)&&(get_CBT_array(a[j]+1,check_arr)==0))
        {
            check_number--;
            j++;
            continue;
        }

        if ((get_CBT_array(a[j],check_arr)==1)&&(get_CBT_array(a[j],check_arr)==1))
        {
            j++;
            update_RTM_to_same_backupversion(container_count_gc,bv,RTMhead);
        }

        //属性为00
        if ((get_CBT_array(a[j],check_arr)==0)&&(get_CBT_array(a[j],check_arr)==0))
        {
        	j++;
            uncheck_container++;
            zero_arr[uncheck_container]=zero_arr[j];
        }
    }

    while(uncheck_container!=0)
    {
        check_last_container_bit_table(zero_arr,uncheck_container,RTMhead);
    }
}



struct RTMdata* get_real_reference_time_map(struct RTMdata *RTMhead)
{
	
	//首先得到最新版本的container_bit_table
	
	int uncheck_container=0;//用来记录属性为00的container的数目。
	int* arr;
	arr=get_newest_container_bit_table(last);
	
	int *zero_arr;
	int i;
	for ( i= 32; i < contaienr_count_end*2; i+2)
	{
		container_count_gc++;

		
		//属性为01或者10
		if((get_CBT_array(i,arr)==0)&&(get_CBT_array(i+1,arr)==1)||(get_CBT_array(i,arr)==1)&&(get_CBT_array(i+1,arr)==0))
		{
			continue;
		}

		//属性为00
		if ((get_CBT_array(i,arr)==0)&&(get_CBT_array(i+1,arr)==0))
		{
			uncheck_container++;
			zero_arr[uncheck_container]=container_count_gc;
		}

		//属性为11
		if ((get_CBT_array(i,arr)==1)&&(get_CBT_array(i+1,arr)==1))
		{
			update_RTM_to_same_backupversion(container_count_gc,current_bv,RTMhead);
		}
	}

	check_last_container_bit_table(zero_arr,uncheck_container, RTMhead);

	return RTMhead;

}










