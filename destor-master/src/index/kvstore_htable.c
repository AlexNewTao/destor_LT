/*
 * kvstore_htable.c
 *
 *  Created on: Mar 23, 2014
 *      Author: fumin
 */

#include "../destor.h"
#include "index.h"
#include "../gc/gc_rtm.h"
#include "../storage/containerstore.h"

#include "../jcr.h"

typedef char* kvpair;

#define get_key(kv) (kv)
#define get_value(kv) ((int64_t*)(kv+destor.index_key_size))

static GHashTable *htable;



extern GHashTable* id_shift_hash;
extern struct _id_shift_type *head;



static int32_t kvpair_size;

static int globle_shift=0;


static struct metaEntry* get_metaentry_in_container_meta(
		struct containerMeta* cm, fingerprint *fp) {
	return g_hash_table_lookup(cm->map, fp);
}



/*
 * Create a new kv pair.
 */



static kvpair new_kvpair_full(char* key){
    kvpair kvp = malloc(kvpair_size);
    memcpy(get_key(kvp), key, destor.index_key_size);
    int64_t* values = get_value(kvp);
    int i;
    for(i = 0; i<destor.index_value_length; i++){
    	values[i] = TEMPORARY_ID;
    }
    return kvp;
}

static kvpair new_kvpair(){
	 kvpair kvp = malloc(kvpair_size);
	 int64_t* values = get_value(kvp);
	 int i;
	 for(i = 0; i<destor.index_value_length; i++){
		 values[i] = TEMPORARY_ID;
	 }
	 return kvp;
}

/*
 * IDs in value are in FIFO order.
 * value[0] keeps the latest ID.
 */
static void kv_update(kvpair kv, int64_t id){
    int64_t* value = get_value(kv);
	memmove(&value[1], value,
			(destor.index_value_length - 1) * sizeof(int64_t));
	value[0] = id;
}

static inline void free_kvpair(kvpair kvp){
	free(kvp);
}


void init_kvstore_htable()
{
    kvpair_size = destor.index_key_size + destor.index_value_length * 8;

    if(destor.index_key_size >=4)
    	htable = g_hash_table_new_full(g_int_hash, g_feature_equal,
			free_kvpair, NULL);
    else
    	htable = g_hash_table_new_full(g_feature_hash, g_feature_equal,
			free_kvpair, NULL);

	sds indexpath = sdsdup(destor.working_directory);
	indexpath = sdscat(indexpath, "index/htable");


	
 
    init_id_shift();
    init_id_shift_hash();
    init_index_bit();


	/* Initialize the feature index from the dump file. */
	FILE *fp;
	if ((fp = fopen(indexpath, "r"))) 
	{
		/* The number of features */
		int key_num;
		fread(&key_num, sizeof(int), 1, fp);
		for (; key_num > 0; key_num--) 
		{
			/* Read a feature */
			kvpair kv = new_kvpair();
			fread(get_key(kv), destor.index_key_size, 1, fp);


			/* The number of segments/containers the feature refers to. */
			int id_num, i;
			fread(&id_num, sizeof(int), 1, fp);
			assert(id_num <= destor.index_value_length);

			for (i = 0; i < id_num; i++)
			{
				/* Read an ID */
				fread(&get_value(kv)[i], sizeof(int64_t), 1, fp);

				int64_t f_id=get_value(kv);

                //2016.11.16.在container store中，根据id号访问container元数据
                struct containerMeta* cm=retrieve_container_meta_by_id(f_id);

                //2016.11.17.首先要判断这个container id是不是已经存放在id_shift结构体中了。
                //我们采用hash表来判断container id是不是已经存在了。
                //直接调用glib中的hash库来操作。
               
                if (!g_hash_table_lookup(id_shift_hash,&f_id))
                {
                    //添加到hash表中
                    g_hash_table_add(id_shift_hash,&f_id);

                    int container_chunk_number=cm->chunk_num;

                    //2016.11.16.追加到id_shift结构体的尾部
                    struct _id_shift_type *is = (struct _id_shift_type*)malloc(sizeof(struct _id_shift_type));
                    is->id_shift_data.container_id=f_id;
                    is->id_shift_data.chunk_shift=globle_shift;

                    struct _id_shift_type *head=_id_shift_AddEnd(head,is->id_shift_data);

                    //把初始的chunk偏移量往后移动
                    globle_shift=globle_shift+container_chunk_number;
                }
			}
			g_hash_table_insert(htable, get_key(kv), kv);
		}
		fclose(fp);
	}

	sdsfree(indexpath);
}











void close_kvstore_htable() {
	sds indexpath = sdsdup(destor.working_directory);
	indexpath = sdscat(indexpath, "index/htable");

	FILE *fp;
	if ((fp = fopen(indexpath, "w")) == NULL) {
		perror("Can not open index/htable for write because:");
		exit(1);
	}

	NOTICE("flushing hash table!");
	int key_num = g_hash_table_size(htable);
	fwrite(&key_num, sizeof(int), 1, fp);

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, htable);
	while (g_hash_table_iter_next(&iter, &key, &value)) {

		/* Write a feature. */
		kvpair kv = value;
		if(fwrite(get_key(kv), destor.index_key_size, 1, fp) != 1){
			perror("Fail to write a key!");
			exit(1);
		}

		/* Write the number of segments/containers */
		if(fwrite(&destor.index_value_length, sizeof(int), 1, fp) != 1){
			perror("Fail to write a length!");
			exit(1);
		}
		int i;
		for (i = 0; i < destor.index_value_length; i++)
			if(fwrite(&get_value(kv)[i], sizeof(int64_t), 1, fp) != 1){
				perror("Fail to write a value!");
				exit(1);
			}

	}

	/* It is a rough estimation */
	destor.index_memory_footprint = g_hash_table_size(htable)
			* (destor.index_key_size + sizeof(int64_t) * destor.index_value_length + 4);

	fclose(fp);

	NOTICE("flushing hash table successfully!");

	sdsfree(indexpath);

	g_hash_table_destroy(htable);
}

/*
 * For top-k selection method.
 */
int64_t* kvstore_htable_lookup(char* key) {
	kvpair kv = g_hash_table_lookup(htable, key);
	return kv ? get_value(kv) : NULL;
}

void kvstore_htable_update(char* key, int64_t id) {
	kvpair kv = g_hash_table_lookup(htable, key);
	if (!kv) {
		kv = new_kvpair_full(key);
		g_hash_table_replace(htable, get_key(kv), kv);

		int64_t f_id=get_value(kv);

		//在container store中，根据id号访问container元数据
        struct containerMeta* cm=retrieve_container_meta_by_id(f_id);

        //首先要判断这个container id是不是已经存放在id_shift结构体中了。
        //我们采用hash表来判断container id是不是已经存在了。
        //直接调用glib中的hash库来操作。
                

        if (!g_hash_table_lookup(id_shift_hash,&f_id))
        {
            //添加到hash表中
            g_hash_table_add(id_shift_hash,&f_id);

            int container_chunk_number=cm->chunk_num;

            //追加到id_shift结构体的尾部
            struct _id_shift_type *is = (struct _id_shift_type*)malloc(sizeof(struct _id_shift_type));
            is->id_shift_data.container_id=f_id;
            is->id_shift_data.chunk_shift=globle_shift;

            struct _id_shift_type *head=_id_shift_AddEnd(head,is->id_shift_data);

            //把初始的chunk偏移量往后移动
            globle_shift=globle_shift+container_chunk_number;
		}  
	}
	kv_update(kv, id);
}


/* Remove the 'id' from the kvpair identified by 'key' */
void kvstore_htable_delete(char* key, int64_t id)
{
	kvpair kv = g_hash_table_lookup(htable, key);
	if(!kv)
		return;

	int64_t *value = get_value(kv);
	int i;
	for(i=0; i<destor.index_value_length; i++){
		if(value[i] == id){
			value[i] = TEMPORARY_ID;
			/*
			 * If index exploits physical locality,
			 * the value length is 1. (correct)
			 * If index exploits logical locality,
			 * the deleted one should be in the end. (correct)
			 */
			/* NOTICE: If the backups are not deleted in FIFO order, this assert should be commented */
			assert((i == destor.index_value_length - 1)
					|| value[i+1] == TEMPORARY_ID);
			if(i < destor.index_value_length - 1 && value[i+1] != TEMPORARY_ID){
				/* If the next ID is not TEMPORARY_ID */
				memmove(&value[i], &value[i+1], (destor.index_value_length - i - 1) * sizeof(int64_t));
			}
			break;
		}
	}

	/*
	 * If all IDs are deleted, the kvpair is removed.
	 */
	if(value[0] == TEMPORARY_ID)
	{
		/* This kvpair can be removed. */
		g_hash_table_remove(htable, key);
	}
}
