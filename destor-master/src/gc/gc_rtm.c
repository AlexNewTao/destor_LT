
#include <stdio.h>
#include <stdlib.h>
#include "gc_rtm.h"
#include "../jcr.h"
#include "../utils/sds.h"

#define SHIFT 5
#define MASK 0x1F
GHashTable* id_shift_hash;

struct _id_shift_type *head;
static FILE* fp;


int64_t container_count_start;
int64_t container_count_end;
//=================================_id_shift============================================
/*
初始化index_bit
首先有关container_bit_table的更新问题，我们就必须要涉及到index_bit这个结构；
	在索引查找的时候，找到了container的id，container meta中存储的map可以得到偏移量，找到chunk的位置
	container meta中chunknum就可知道chunk的数目，每一个chunk用一位bit来表示。初始化为0
	index_bit结构：containerid+chunknum的bit位
*/


/*由于在索引中key-value在进行分配的时候，并不是以物理上container的方式进行添加的，而是以segment为单位进行
替换，步骤如下。
（1）在索引初始化的时候，根据索引的值，我们找到container的id号，根据id号访问container meta数据，得到
container对应的chunk数目，存储在id_shift结构体中；注意，id_shift中的偏移量是chunk初始的偏移量。
（2）对id_shift中的container id建立一个hash表，方便在O(1)的时间复杂度内招待对应的id号是否已经存在于id_shiftzhong。
（3）有新的索引来的时候，访问hash表，判断是否已经存在id_shift中了，以及保存，就不管，否则就添加在id_shift中。
（4）id_shift对于一个版本来说是常驻内存的，备份结束之后，会进行container_bit_table的进一步判断。

*/

//_id_shift用来存放conatiner的id号以及对应container中chunk在index_bit中的初始偏移量；

/*
struct _id_shift
{
	int64_t container_id;
	int32_t chunk_shift;

};

struct _id_shift_type
{
	struct _id_shift id_shift_data;
	struct _id_shift_type *next;
};*/


void init_id_shift()
{
	struct _id_shift_type* head=NULL;
	head=(struct _id_shift_type *)malloc(sizeof(struct _id_shift_type));
	head->id_shift_data.container_id=0;
	head->id_shift_data.chunk_shift=0;
	head->next=NULL;
	printf("init_id_shift successfully!\n");
	
}





void init_id_shift_hash()
{
	id_shift_hash = g_hash_table_new(g_int64_hash, g_int_equal);
	printf("init id_shift_hash successfully!\n");
	
}

void destory_id_shift_hash()
{
	g_hash_table_destroy(id_shift_hash);
}

void Destory_id_shift()
{
    struct _id_shift_type *p = head;
    while(head!=NULL)
    {
         head = head->next;
         free(p);
         p = head;
    }
}
//20161128,在备份结束后，我们要释放id_shift和id_shift_hash；
void close_id_shift_and_hashtable()
{
	Destory_id_shift();
	destory_id_shift_hash();
}



/*=======================================*/
//在id_shift的链表中追加结点；
struct _id_shift_type *_id_shift_AddEnd (struct _id_shift_type *head,struct _id_shift id_shift_data)
{
    struct _id_shift_type *node,*htemp;
    if (!(node=(struct _id_shift_type *)malloc(sizeof(struct _id_shift_type))))//分配空间
    {
        printf("内存分配失败！\n");
        return NULL;
    }
    else
    {
        node->id_shift_data.container_id=id_shift_data.container_id;
        node->id_shift_data.chunk_shift=id_shift_data.chunk_shift;//保存数据
        node->next=NULL;//设置结点指针为空，即为表尾；
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

//通过id号找到对应的偏移量；

int get_shift_by_id(struct _id_shift_type *head,int64_t id)
{
	struct _id_shift_type *htemp;
    htemp=head;
    while(htemp)
    {
        if (htemp->id_shift_data.container_id==id)
        {
            return htemp->id_shift_data.chunk_shift;
        }
        htemp=htemp->next;
    }
    return 0;
}

int get_next_shift_by_id(struct _id_shift_type *head,int64_t id)
{
	struct _id_shift_type *htemp;
    htemp=head;
    while(htemp)
    {
        if (htemp->id_shift_data.container_id==id)
        {
            return htemp->next->id_shift_data.chunk_shift;
        }
        htemp=htemp->next;
    }
    return 0;
}

//释放空间






//============================index_bit======================================
//初始化一个数组的n位的所有位为0；
//size为 index中chunk的数目/32+1，
//index_bit_size的大小表示chunk的数目，采用4TB的数据为准

#define SHIFT 5
#define MASK 0x1F

#define index_bit_size  1<<26

int index_bit[index_bit_size];
void init_index_bit()
{
	index_bit[index_bit_size]&=0;
	printf("index_bit init successfully!\n");
}


//把某一位设置为1

void set_index_bit(int n)
{
	int index_loc;
	int bit_loc;
	index_loc=n>>SHIFT;//等价于n/32。
	bit_loc=n&MASK;//等价于n%32。
	index_bit[index_loc]|=1<<bit_loc;
}


//得到index_bit中的某一位是1还是0；
int get_index_bit(int n)
{
	int index_loc;
	int bit_loc;
	index_loc=n>>SHIFT;//等价于n/32。
	bit_loc=n&MASK;//等价于n%32。
	return index_bit[index_loc]<<bit_loc;
}

//在某个范围内检测bit位是否全为0，部分为0，全为1，分别用0,1,2来表示
int check_index_bit_equal_one(int start,int end)
{
	int i=start;
	int j=start+1;
	while(i<end) //先遍历一遍start到end所指向的元素
	{
		if (get_index_bit(i))
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
		return 2; //i为end表示遍历到了末尾，全部为1，return 2；
	}
	else if (i==start) //有可能第一个就是0，已经break了，但是不能判断是全0还是部分为0，所以需要下一步判断
	{
		while(j<end)
		{
			if(!get_index_bit(j+1)) //如果为0，就接着遍历，否则退出。
			{
				j++;
			}
			else
				break;
		}
		
	}

	if (j==end) //如果最后遍历结束，到end，说明全部为0，否则部分为0.
	{
		return 0;
	}
	else
		return 1;

}



//=============================container_bit_table==========================

//container的个数，为了简单起见，我们直接固定化大小，为4T大小
#define container_size 1<<16
int CBT[container_size];
void init_container_bit_table(int backupversion)
{
	//因为数组大小为32位，我们对每个container采用两位表示；
	CBT[container_size]&=0;
	CBT[0]=backupversion;
	printf("init container_bit_table successfully\n");
}



//得到版本新增加的container的数目；
int64_t get_container_count_add()
{
	return (container_count_end-container_count_start);
}

//得到存储的container的数目
//由于空间的分配，我需要指定空间的数目，所以，我需要知道存储的container的数目，但是container在备份中
//的大小是变化的，那么首先我就先分配一定量的空间，然后每过一段时间，检测一下container的数目是否大于已
//经分配的空间数目，如果大于，就多分配空间，否则，就不用管。
//在这里，我就简单化处理，假设存储的数目达到10T，container的大小为4M，那么container的数目为10T/4M=2.5*2^20

//这一个是实际得到的存储的container的数目
int64_t get_storage_container_number()
{
    //我再这里用M为单位,那么存储的container数目为 
    return contaienr_count_end;
}


//2016.11.18.设置对应的位
void set_container_bit_table(int n,int zero_or_one)
{
	int index_loc;
	int bit_loc;
	index_loc=(n>>SHIFT)+1;//等价于n/32。第一个用来存储备份版本数
	bit_loc=n&MASK;//等价于n%32。
	if (zero_or_one==1)
	{
		CBT[index_loc]|=1<<bit_loc;
	}
	else
	{
		CBT[index_loc]&=0<<bit_loc;
	}

}


//更新container_bit_table
//新增引用为01
//部分引用为10
//完全引用为11
int current_bv;

void update_container_bit_table(int backupversion)
{
	struct _id_shift_type *htemp;
	htemp=head;
	if(CBT[0]!=backupversion)
	{
		printf("the backupversion doesn't match in the CBT\n");
	}
	while(htemp!=NULL)
    {
    	int64_t id=htemp->id_shift_data.container_id;

    	//int shift=htemp->id_shift.chunk_shift; 这个属性的偏移量和CBT中的是不一样的。

    	int64_t shift=id-32;//有32位是备份版本数占据了，所以偏移量需要减去32.

    	int attribute=get_attribute(id);

    	update_container_bit_table_attribute(shift,attribute);


    	/*if (attribute==1)
    	{
    		update_reference_time_map(head,id);
    	}*/
    	

        htemp=htemp->next;
    }

    int64_t num_new_add=get_container_count_add();
    
    int add_attribute=3;
    int64_t i;
    for (i = 0; i < num_new_add; i=i+2)
    {
    	update_container_bit_table_attribute(i,add_attribute);
    }

    printf("update container_bit_table successful!\n");

}



//根据container的id号，得到container相对于上一个版本的属性
int get_attribute(int64_t id)
{
	int start=get_shift_by_id(head,id);
	int end=get_next_shift_by_id(head,id)-1;
	int flag=check_index_bit_equal_one(start,end);
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
    CBTpath = sdscat(CBTpath, "/containerbittable/");

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






//2016.11.18把container bit table以文件的形式写到磁盘上，每个版本都可以后缀添加


void write_container_bit_table_to_disk() 
{
    sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/containerbittable/");

    sds cbt_fname = sdsdup(CBTpath);
    cbt_fname = sdscat(cbt_fname, "container_bit_table");

    FILE *fp;
    if((fp = fopen(cbt_fname, "r"))) 
    {
        /* Read if exists. */
        fwrite(&CBT, container_size, 1, fp);
        fclose(fp);
    }
    sdsfree(cbt_fname);
    NOTICE("write container bit table successfully");
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

/*typedef struct RTMdata
{
	int64_t id;
	int16_t *rtm;
	
};

typedef struct RTMlist
{
	RTMdata rtmdata;
	struct RTMlist *next;
};

*/


//单链表的初始化
 
/*RTMlist* Init_RTMlist()
{
    RTMlist *L;
    L = (RTMlist *)malloc(sizeof(RTMlist));   //申请结点空间 
    if(L == NULL)                       //判断是否有足够的内存空间 
        printf("申请内存空间失败\n");
    L->next=NULL;
    printf("Init_RTMlist successfully!\n");             
 	return L;
}

*/
/*RTMlist *RTMlist_AddEnd (RTMlist *head,RTMdata rtmdata)
{
    RTMlist *node,*htemp;
    if (!(node=(RTMlist *)malloc(sizeof(RTMlist))))//分配空间
    {
        printf("内存分配失败！\n");
        return NULL;
    }
    else
    {
        node->rtmdata.id=rtmdata.id;//保存数据
        node->rtmdata.rtm=rtmdata.rtm;//保存数据
        node->next=NULL;//设置结点指针为空，即为表尾；
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
        return htemp;
    }
}*/

/*void update_reference_time_map(RTMlist *head,int64_t id)
{
	int start=get_shift_by_id(head,id);
	int end=get_next_shift_by_id(head,id)-1;

	RTMlist *tmphead=head->next;
	while(tmphead!=NULL)
	{
		if (tmphead->rtmdata.id==id)
		{
			for (int i = 0; i < end-start; i++)
			{
				if (get_index_bit(start+i))
				{
					tmphead->rtmdata.rtm[i]=current_bv;
				}
			}
			
		}
		tmphead=tmphead->next;
	}
}*/


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


/*int* get_newest_container_bit_table(int last)
{
	CBTpath = sdsdup(destor.working_directory);
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

}*/


/*int get_CBT(int n)
{
	int index_loc;
	int bit_loc;
	index_loc=n>>SHIFT;//等价于n/32。
	bit_loc=n&MASK;//等价于n%32。
	return CBT[index_loc]<<bit_loc;
}
*/


//n means the number of n-th container 
/*void update_RTM_to_same_backupversion(int n,int backupversion,RTMlist *head)
{
	
	RTMlist *htemp;
	htemp=head;
	int i=0;
	while(i<n)
	{
		htemp=htmp->next;
	}
	int j=0;
	while(htemp->rtmdata.rtm[j]!=backupversion)
	{
		htemp->rtmdata.rtm[j]=backupversion;
		j++;
	}
}*/




//数组用来存放上次检测为00的位置；

/*void check_last_container_bit_table(int* a,int n, RTMlist *head)
{
    int check_number=n;
    int* check_arr=get_newest_container_bit_table(last+1);
    int bv=check_arr[0];
    int j=0;
    int *zero_arr;

    int uncheck_container=0;
    while(j<n)
    {

        if ((get_CBT(a[j],check_arr)==0)&&(get_CBT(a[j]+1,check_arr)==1)||(get_CBT(a[j],check_arr)==1)&&(get_CBT(a[j]+1,check_arr)==0))
        {
            check_number--;
            j++;
            continue;
        }

        if ((get_CBT(a[j],check_arr)==1)&&(get_CBT(a[j],check_arr)==1))
        {
            j++;
            update_RTM_to_same_backupversion(container_count,bv,head);
        }

        //属性为00
        if ((get_CBT(a[j],check_arr)==0)&&(get_CBT(a[j],check_arr)==0))
        {
        	j++;
            uncheck_container++;
            zero_arr[uncheck_container]=zero_arr[j];
        }
    }

    while(uncheck_container!=0)
    {
        check_last_container_bit_table(zero_arr,uncheck_container,head)
    }
}*/


/*RTMlist* get_real_reference_time_map(RTMlist *head)
{
	int last=1;
	//首先得到最新版本的container_bit_table
	int container_count=0;//用来记录访问的container的数目；

	int uncheck_container=0;//用来记录属性为00的container的数目。
	int* arr;
	arr=get_newest_container_bit_table(last);
	
	int *zero_arr;
	
	for (int i = 32; i < contaienr_count_end*2; i+2)
	{
		container_count++;

		
		//属性为01或者10
		if((get_CBT(i,arr)==0)&&(get_CBT(i+1,arr)==1)||(get_CBT(i,arr)==1)&&(get_CBT(i+!,arr)==0))
		{
			continue;
		}

		//属性为00
		if ((get_CBT(i,arr)==0)&&(get_CBT(i+1,arr)==0))
		{
			uncheck_container++;
			zero_arr[uncheck_container]=container_count;
		}

		//属性为11
		if ((get_CBT(i,arr)==1)&&(get_CBT(i+1,arr)==1))
		{
			update_RTM_to_same_backupversion(container_count,current_bv,head);
		}
	}

	check_last_container_bit_table(zero_arr,uncheck_container, head);

	return head;

}*/










