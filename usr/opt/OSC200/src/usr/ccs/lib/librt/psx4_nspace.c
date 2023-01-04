/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
 * psx4_nspace.c
 *
 * Operations to manage POSIX 1003.4/D11 Binary Semaphores namespaces.  
 *
 */
#include <sys/psx4_nspace.h>
#include <errno.h>


p4_key_table_t  *p4_create_key_table();
void            *p4_find_object_by_key();
p4_key_entry_t  *p4_find_entry_by_key();
int             p4_bind_key_to_name_entry();
int             p4_get_key_by_name();
queue_entry_t   dequeue_head();
void            remqueue();
void            enqueue_tail();


p4_key_t p4_invalid_key = {0, 0};

#define  qelt (queue_chain_t *)ke 

/*
 * Create an empty key table.  The key table will hold the number of key
 * entries indicated by cnt.  The size of key entry is indicated by size.
 */

p4_key_table_t *p4_create_key_table(cnt, size)
     register int cnt;
     register int size;

{
  register p4_key_table_t *kt;
  register p4_key_entry_t *ke;
  register int i;
  
  /* Catch obvious errors. */
  if (size < 0) {
    return P4_KEY_TABLE_NULL;
  }


  /* Create the key table header. */
  kt = (p4_key_table_t *)malloc(sizeof(p4_key_table_t));
  if (kt == P4_KEY_TABLE_NULL) {
    return P4_KEY_TABLE_NULL;
  }

  /* Create the descriptor array. */
  ke = (p4_key_entry_t *)malloc(cnt * size);
  if (ke == P4_KEY_ENTRY_NULL) {
    free(kt, sizeof(p4_key_table_t));
    return P4_KEY_TABLE_NULL;
  }

  /* 
   * Initialize everything.  Start version numbers at 1 so that the key
   * {0,0} is never valid.  We use this for P4_INVALID_KEY.  Put all keys
   * on the free list to start.
   */
 
  kt->size = cnt;
  kt->e_size = size;
  queue_init(&kt->free_q);
  queue_init(&kt->used_q);
  kt->entry = ke;
  
  memset(ke, 0, cnt*size);
 
  for(i=0; i < cnt; i++, ke = (p4_key_entry_t *)(((char *)ke)+size)){
       enqueue_tail(&kt->free_q, qelt);
        
       P4_SET_KEY_INDEX(ke->key, i);
       P4_SET_KEY_VERSION(ke->key,1);
     }

  /* Last entry on freelist points to invalid key marker. */
 
  return kt;
}
      
/*
 * Find an object by key.  
 */
void *p4_find_object_by_key(kt, key)
     register p4_key_table_t *kt;
     register p4_key_t key;
{
  register p4_key_entry_t *ke;

  if((kt == P4_KEY_TABLE_NULL) || ((p4_key_invalid(kt,key)) ==  EINVAL))
        return (void *)EINVAL;
 
  ke = P4_GET_ENTRY(kt, P4_KEY_INDEX(key));

  return ke->object;

}

/*
 * Find an entry by key.  
 */

p4_key_entry_t *p4_find_entry_by_key(kt, key)
     register struct p4_key_table *kt;
     register struct p4_key key;
{

  if((kt == P4_KEY_TABLE_NULL) || ((p4_key_invalid(kt,key)) == EINVAL))
        return (void *)EINVAL;
   return P4_GET_ENTRY(kt, P4_KEY_INDEX(key));
}



/* 
 * Create a binding between a key and a name entry.
 */
int p4_bind_key_to_name_entry(kt, object, name, mykey)
     register p4_key_table_t *kt;
     void     *object;
     char     *name;
     p4_key_t *mykey;
{

   register p4_key_entry_t *ke;

   if(kt == P4_KEY_TABLE_NULL)
       return FAILURE;
 
   if(queue_empty(&kt->free_q))
       return FAILURE; 

   qelt = dequeue_head(&kt->free_q);
   if(ke == 0)
       return FAILURE;
    

    /*
     * Set up the key entry.
     */
    ke->name = (char *)malloc(strlen(name));
    if(!ke->name){
      enqueue_tail(&kt->free_q, qelt);
      return FAILURE;
    }
    strcpy(ke->name, name);
    ke->object = object;
    ke->state = P4_KEY_STATE_INUSE;
    ke->pid = getpid();
    enqueue_tail(&kt->used_q, qelt);
    mykey->index = ke->key.index;
    mykey->version = ke->key.version;

    return SUCCESS;
}

/*
 * Return a key entry to the freelist.  The key version is incremented to
 * prevent reuse of the key.
 *
 */
p4_return_key_entry(kt,ke)
     register struct p4_key_table *kt;
     register struct p4_key_entry *ke;
{
  register p4_key_version_t version;


  /* Increment the version so that this key will be stale. */

  version = P4_KEY_VERSION(ke->key);
  if (version >= P4_KEY_VERSION_MAX) version = 0;
  version++;
  P4_SET_KEY_VERSION(ke->key, version);

  /* 
   * Clear out the key entry.  Save values to pass to the last close routine.
   */
  ke->state = P4_KEY_STATE_FREE;
  ke->object = NULL;
  ke->pid = NULL;
  ke->lock = P4_MEM_UNLOCK;
  /* 
   * Put the key entry back on the free list.
   */

  queue_move(&kt->used_q, &kt->free_q, qelt);

  return SUCCESS;
}

/* 
 * Get key by name.
 */
int p4_get_key_by_name(kt,name, mykey)
     register p4_key_table_t *kt;
     char     *name;
     p4_key_t *mykey;
{
   struct obj_header *obj_h;
   register p4_key_entry_t *ke;
  
   if((kt == P4_KEY_TABLE_NULL) ||(queue_empty(&kt->used_q)))
       return FAILURE; 

  
   for((queue_chain_t *)ke = queue_first(&kt->used_q); 
       !(queue_end(&kt->used_q, qelt)); 
       (queue_chain_t *)ke = queue_next(qelt)){
       
         
       if(!strncmp(ke->name, name, strlen(name))){                        /* name match */
            obj_h = (struct obj_header *)ke->object;
            if(obj_h->state == P4_KEY_STATE_DELETE)
                  continue;
            break;
	  }
       else
          continue;
     }
  
  if(queue_end(&kt->used_q, qelt))
      return FAILURE;

  *mykey = ke->key;

  return SUCCESS;

}


/*
 * Check key for validity.  
*/

p4_key_invalid(kt,key)
     register struct p4_key_table *kt;
     register struct p4_key key;
{
  register struct p4_key_entry *ke;

  if (P4_KEY_INDEX_OUTRANGE(key,kt)) 
    return EINVAL;
  
  ke = P4_GET_ENTRY(kt, P4_KEY_INDEX(key));

  if (!(P4_KEYS_EQUAL(key, ke->key)) || (ke->state != P4_KEY_STATE_INUSE)) 
    return EINVAL;
    
  return 0;
}


/*
 *      Insert element at tail of queue.
 */

void enqueue_tail(que,elt)
        register queue_t        que;
        register queue_entry_t  elt;
{
        elt->next = que;
        elt->prev = que->prev;
        elt->prev->next = elt;
        que->prev = elt;
}

/*
 *      Remove and return element at head of queue.
 */

queue_entry_t dequeue_head(que)
        register queue_t        que;
{
        register queue_entry_t  elt;

        if (que->next == que)
                return((queue_entry_t)0);

        elt = que->next;
        elt->next->prev = que;
        que->next = elt->next;
        return(elt);
}

/*
 *      Remove arbitrary element from queue.
 */


void remqueue(que, elt)
        queue_t                 que;
        register queue_entry_t  elt;
{
        elt->next->prev = elt->prev;
        elt->prev->next = elt->next;
}







