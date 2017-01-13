
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
	printf("init_id_shift successfully!\n");
	
}


void init_id_shift_hash()
{
	id_shift_hash = g_hash_table_new_full(g_int64_hash, g_int64_equal, free,NULL);
	printf("init id_shift_hash successfully!\n");
}

void destory_id_shift_hash()
{
	//g_hash_table_destroy(id_shift_hash);
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
        int ans=htemp->id_shift_data.chunk_shift+htemp->id_shift_data.chunk_num;
        return ans;
    }

    printf("can't find the shift!worry!\n");
    return 0;
}








//============================index_bit======================================


#define SHIFT 5
#define MASK 32


void init_index_bit()
{
	index_bit[index_bit_size]={0};
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


void init_container_bit_table(int backupversion)
{
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

void show_RTM()
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

	struct _id_shift_type *htemp;
	htemp=ishead;
	if(CBT[0]!=backupversion)
	{
		printf("the backupversion doesn't match in the CBT\n");
	}
	while(htemp!=NULL)
    {

    	int64_t id=htemp->id_shift_data.container_id;


        int cbt_shift=id*2+33;
        int attribute=get_attribute(id);
            
        update_container_bit_table_attribute(cbt_shift,attribute);
        
    	if (attribute==1)
    	{
      		update_reference_time_map(id);
    	}
        
        htemp=htemp->next;
       
    }
   

    int num_new_add=container_count_end-container_count_start;


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
}


int get_attribute(int64_t id)
{
	
	int start=get_shift_by_id(id);
 
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
    CBTpath = sdscat(CBTpath, "/container_bit_table.cbt");

    int container_shift=container_size;

    if((fp = fopen(CBTpath, "r"))) 
    {
        
        while(CBT[0]!=backupversion)
        {
        	if (fseek(fp,container_shift,SEEK_SET))
        	{
        		fread(&CBT, sizeof(CBT),1,fp);
        	}

        	container_shift+=container_size;//偏移量往后移动
        }
        
        fclose(fp);
    }

    sdsfree(CBTpath);
    
    NOTICE("get container bit table of %d backupversion successfully",backupversion);

    return CBT;
}



void write_container_bit_table_to_disk() 
{
    sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/container_bit_table.cbt");

    FILE *fp;
    if((fp = fopen(CBTpath, "a+"))) 
    {
        /* Read if exists. */
        fwrite(&CBT, sizeof(CBT), 1, fp);
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

int* get_merge_container_bit_table(int deleteversion)
{

    int* ans;
    if (deleteversion==1)
    {
        ans=get_container_bit_table(deleteversion);  
    }
    else
    {
        ans=get_container_bit_table(1);
        int i;
        for ( i= 1; i < deleteversion; i++)
        {
            int *ans2=get_container_bit_table(i+1);
            ans=merge_container_bit_table(ans,ans2);
            free(ans2);
        }
    }
    return ans;

  
}

int* merge_container_bit_table(int* v1,int* v2)
{
    int n=get_container_bit_end();
    int i;
    for (i = 1; i <= 32; i++)
    {
        set_CBT_array(i,0,v1);
    }
    for (i= 33; i <= n*2+33; i=i+1)
    {
        if ((get_CBT_array(i,v1)==1)||(get_CBT_array(i,v2)==1))
        {
            set_CBT_array(i,1,v1);
        }
        else if ((get_CBT_array(i,v1)==0)||(get_CBT_array(i,v2)==0))
        {
            set_CBT_array(i,0,v1);
        }
    }
    return v1;
}


int* get_first_container_bit_table()
{
    
    sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/container_bit_table.cbt");

    int32_t* new_cbt=(int32_t* )malloc(sizeof(CBT));
    if ((fp = fopen(CBTpath, "r"))) {
        
        fseek(fp,0,SEEK_SET);
        fread(new_cbt, sizeof(CBT),1, fp);
       
        fclose(fp);   
    }
   
    sdsfree(CBTpath);

    return new_cbt;
}


int* get_container_bit_table(int times)
{
    

    sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/container_bit_table.cbt");
    int32_t* new_cbt=(int32_t* )malloc(sizeof(CBT));
 
    if ((fp = fopen(CBTpath, "r"))) {
        
        fseek(fp,(times-1)*sizeof(CBT),SEEK_SET);
        
        fread(new_cbt, sizeof(CBT),1, fp);
       
        fclose(fp);
        
    }
   
    sdsfree(CBTpath);

    return new_cbt;
}


//===============================reference_time_map=====================


struct RTMdata *RTMhead;


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
    printf("Init_RTMlist successfully!\n");             
}

void write_RTM_to_disk() 
{
    sds RTMpath = sdsdup(destor.working_directory);
    RTMpath = sdscat(RTMpath, "/reference_time_map.rtm");

    FILE *fp;
    if((fp = fopen(RTMpath, "w+"))) 
    {
        
    	while(RTMhead!=NULL)
    	{
            
    		fwrite(&RTMhead->id, sizeof(int64_t), 1, fp);
           
    		fwrite(&RTMhead->len, sizeof(int16_t), 1, fp);
           
            
            fwrite(RTMhead->rtm, sizeof(int16_t), RTMhead->len, fp);

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
    	struct RTMdata* tmp;
        struct RTMdata* tmp1;
        int i=0;
    	while((!feof(fp))&&(i<container_count_start))
    	{
        
            tmp = (struct RTMdata*)malloc(sizeof(struct RTMdata));
            fread(&tmp->id, sizeof(tmp->id), 1, fp);
            fread(&tmp->len, sizeof(tmp->len), 1, fp);
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
    NOTICE("read_RTM_from_disk successfully");

}




void RTMlist_AddEnd (struct RTMdata* rtmdata)
{
    struct RTMdata *node,*htemp;
    if (!(node=(struct RTMdata *)malloc(sizeof(struct RTMdata))))
    {
        printf("fail alloc space！\n");
        return NULL;
    }
    else
    {
        node->id=rtmdata->id;
        node->rtm=rtmdata->rtm;
        node->len=rtmdata->len;

        node->next=NULL;
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


int* get_newest_container_bit_table(int times)
{

    sds CBTpath = sdsdup(destor.working_directory);
    CBTpath = sdscat(CBTpath, "/container_bit_table.cbt");
    
    int* cbt=(int32_t *)malloc(sizeof(CBT));
    //int cbt[sizeof(CBT)]={0};
    //times*sizeof(CBT)
    if ((fp = fopen(CBTpath, "r"))) {
        
        fseek(fp,-times*sizeof(CBT),SEEK_END);
        fread(cbt, sizeof(CBT),1, fp);
        

        fclose(fp);
    }

    sdsfree(CBTpath);

    NOTICE("get newest container bit table successfully");
    
    return cbt;
}

void show_cbt()
{
    int i=1;
    int *arr=get_newest_container_bit_table(i);

    printf("show_ccbtshow_cbtshow_cbtshow_cbt\n");

    int k;
    for (k = 1; k <=1000; k++)
    {
        printf(" %d",get_CBT_array(k,arr));
        if (k%32==0)
        {
            printf("\n");
        }
    }

}


int32_t get_CBT_array(int32_t n,int *array)
{
    int32_t index_loc;
    int32_t bit_loc;
    int check=n%MASK;
    int flag;
    if (check==0)
    {
        index_loc=(n>>SHIFT)-1;
        flag=array[index_loc]&(1<<31);
    }
    else
    {
        index_loc=n>>SHIFT;
        bit_loc=n%MASK-1;
        flag=array[index_loc]&(1<<bit_loc);
    }
    if (flag==0)
    {
        return 0;
    }
    else
        return 1;
}

void set_CBT_array(int n,int zero_or_one,int *array)
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
            array[index_loc]|=(1<<31);
        }
        else
        {
            index_loc=n>>SHIFT;
            bit_loc=n%MASK-1;
            array[index_loc]|=(1<<bit_loc);
        }
    }
}


void update_RTM_to_same_backupversion(int n,int backupversion)
{
	
	struct RTMdata *htemp;
	htemp=RTMhead;

	int i=0;

    while(i<n)
	{
		htemp=htemp->next;
        i=i+1;
	}

	int j=0;
	while((htemp->rtm[j]!=backupversion)&&(j<htemp->len))
	{
		htemp->rtm[j]=backupversion;
		j=j+1;
	}
 
}


static int last=1;


static void check_g_sequence(GSequence *seq)
{
    GSequenceIter* new_iter=g_sequence_get_begin_iter(seq);
    GSequenceIter* new_end = g_sequence_get_end_iter(seq);

    while(new_iter!=new_end)
    {
        int *pos=g_sequence_get(new_iter);
        
        new_iter=g_sequence_iter_next(new_iter);
        printf(" %d ",*pos);
    }
}
//static int container_count_gc=0;//用来记录访问的container的数目；

//数组用来存放上次检测为00的位置；

void check_last_container_bit_table(GSequence *seq,int quit)
{
    //int check_number=g_sequence_get_length(seq);

    last=last+1;

    //printf("last now is %d\n",last);

    int* check_arr=get_newest_container_bit_table(last);
    
    int check_bv=check_arr[0];
    int j=0;
    //int *zero_arr;

    GSequence *zero_new_gsequence=g_sequence_new(free);
    //int uncheck_container=0;

    GSequenceIter* iter=g_sequence_get_begin_iter(seq);
    GSequenceIter* end = g_sequence_get_end_iter(seq);

    while(iter!=end)
    {
        int *pos=g_sequence_get(iter);

        ///printf("aaaa%d\n", *pos);
        int m,n;
        m=get_CBT_array(*pos,check_arr);
        n=get_CBT_array(*pos+1,check_arr);

        if ((m==0&&n==1)||(m==1&&n==0))
        {
            
            iter=g_sequence_iter_next(iter);
        }
        else if(m==1&&n==1)
        {
            update_RTM_to_same_backupversion(*pos,check_bv);
            iter=g_sequence_iter_next(iter);
        }
        else if (m==0&&n==0)
        {
            g_sequence_append(zero_new_gsequence,pos);
            iter=g_sequence_iter_next(iter);
        }
    }
    //check_g_sequence(zero_new_gsequence);
    //printf("g_sequence_get_length is %d\n",g_sequence_get_length(zero_new_gsequence));
    if(g_sequence_get_length(zero_new_gsequence)!=0)
    {
        
        check_last_container_bit_table(zero_new_gsequence,quit);
    }

    //g_sequence_free(zero_new_gsequence);

}




void get_real_reference_time_map()
{
    int cce=get_container_bit_end();
    

    read_RTM_from_disk_in_gc(cce);

    //show_RTM();
	int32_t *arr=get_newest_container_bit_table(last);
   

	int cu_bv=arr[0];



    GSequence *zero_gsequence=g_sequence_new(free);

    int border=cce*2+33;

    int i=33;
    int j=0;
	while(i<border)
	{

        int m,n;
        m=get_CBT_array(i,arr);
        n=get_CBT_array(i+1,arr);
        
        if ((m==0&&n==1)||(m==1&&n==0))
        {
            i=i+2;
        }
        else if (m==0&&n==0)
        {
    
            int* s=(int32_t*)malloc(sizeof(int32_t));
            *s=j;
            g_sequence_append(zero_gsequence,s);
            i=i+2;
        }
        else if (m==1&&n==1)
        {
            update_RTM_to_same_backupversion(j,cu_bv);
            i=i+2;
        }
        j=j+1;
	}

    //check_g_sequence(zero_gsequence);
	check_last_container_bit_table(zero_gsequence,cu_bv+1);
    
    g_sequence_free(zero_gsequence);

}







void check_write_cce()
{
    int n=get_container_bit_end();
    printf("the cce is %d\n",n );
}



int get_container_bit_end()
{
    sds ccepath = sdsdup(destor.working_directory);
    ccepath = sdscat(ccepath, "/container_count_end.cce");
    int64_t* cce=(int64_t*)malloc(sizeof(int64_t));

    if((fp = fopen(ccepath, "r"))) 
    {
    
        fread(&cce, sizeof(int64_t),1,fp);
        fclose(fp);
    }

    sdsfree(ccepath);
    
    NOTICE("get container_count_end successfully");

    return cce;
}



void write_container_count_end_to_disk() 
{
    sds ccepath = sdsdup(destor.working_directory);
    ccepath = sdscat(ccepath, "/container_count_end.cce");

    FILE *fp;
    if((fp = fopen(ccepath, "w"))) 
    {
    
        fwrite(&container_count_end, sizeof(int64_t), 1, fp);
        fclose(fp);
    }
    sdsfree(ccepath);
    NOTICE("write container_count_end to disk successfully");
     
}


void  read_RTM_from_disk_in_gc(int broad) 
{
 
    sds RTMpath = sdsdup(destor.working_directory);
    RTMpath = sdscat(RTMpath, "/reference_time_map.rtm");
    FILE *fp;
    RTMhead = NULL;
    if((fp = fopen(RTMpath, "r+"))) 
    {
        
        struct RTMdata* tmp;
        struct RTMdata* tmp1;
        int i=0;
        while((!feof(fp))&&(i<broad))
        {
        
            tmp = (struct RTMdata*)malloc(sizeof(struct RTMdata));
            fread(&tmp->id, sizeof(tmp->id), 1, fp);
            fread(&tmp->len, sizeof(tmp->len), 1, fp);
          
            
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

    NOTICE("read_RTM_from_disk in gc successfully");

    //return RTMhead;
}


void Destory_RTM()
{
    struct RTMdata*p = RTMhead;
    while(RTMhead!=NULL)
    {
         p = RTMhead;
         free(p->rtm);
         free(p);
         RTMhead = RTMhead->next;
    }
}
