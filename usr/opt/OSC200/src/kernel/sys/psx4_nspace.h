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
 * psx4_nspace.h
 *
 * This file contains the definitions for name space handler for the POSIX 1003.4/D11
 * Binary Semaphores.
 */

#include <sys/types.h>
#ifndef _PSX4_NSPACE_H
#define _PSX4_NSPACE_H 1

#define SUCCESS  0
#define FAILURE -1
#define TRUE  1
#define FALSE 0
#ifndef NULL
#define NULL  0L
#endif

/* 
 * object header 
 */
struct obj_header{
      ushort     cnt;
      ushort     state;
#define P4_STATE_INUSE      0 
#define P4_KEY_STATE_DELETE 1
    };


typedef unsigned short p4_key_version_t;
typedef unsigned short p4_key_index_t;

typedef struct p4_key {
  p4_key_index_t index;		/* Index of entry */
  p4_key_version_t version;	/* Version number of key */
} p4_key_t;

/*
 * These macros isolate knowledge of the format of the key.  They would need
 * to be changed to use masks and shifts if the two parts of the key are
 * changed to be asymmetric.  If that is done, make the index access faster,
 * and test the index first when comparing keys (the test will fail faster
 * if a key is on the free list).
 */

#define P4_KEY_INDEX(k) (k).index
#define P4_SET_KEY_INDEX(k,v) (k).index = v
#define P4_KEY_INDEX_OUTRANGE(key,kt) ((key).index >= (kt)->size)
#define P4_KEY_VERSION(k) (k).version
#define P4_SET_KEY_VERSION(k,v) (k).version = v
#define P4_KEY_VERSION_MAX 65535
#define P4_KEYS_EQUAL(k1,k2) \
  (((k1).index == (k2).index) && ((k1).version == (k2).version))

#define P4_GET_ENTRY(kt, index)  (void *)((char *)kt->entry + (kt->e_size * index))      

/*
 * Special invalid key marker (for error return from p4_create_key).
 */

#define P4_INVALID_KEY p4_invalid_key


/* 
 * A generic doubly-linked list (queue)
 */

struct p4queue_entry {
	struct p4queue_entry	*next;		                  /* next element */
	struct p4queue_entry	*prev;		                  /* previous element */
};


typedef struct p4_key_entry{
       struct p4queue_entry *next;                                   
       struct p4queue_entry *prev;                                 /* free/use queue                */
       char                 *name;                                 /* semaphore set name            */
       void                 *object;                               /* virtual address of object     */
       p4_key_t              key;                                  /* valid key                     */
       ushort                flag;                                 /* deleted flag                  */
       pid_t                 pid;                                  /* process id                    */
       int                   lock;                                 /* memory lock                   */
#define P4_MEM_LOCK     1
#define P4_MEM_UNLOCK   0
       int                   state;                                /* Is this entry available ?     */
#define P4_KEY_STATE_FREE 0
#define P4_KEY_STATE_INUSE 1
     }p4_key_entry_t;

/*  The purpose of p4_key_table is to provide a process level binding to  the object */

typedef struct p4_key_table{
       struct p4queue_entry        free_q;
       struct p4queue_entry        used_q;
       ushort                    cnt;
       ushort                    size;                                   /* size of this table */
       ushort                    e_size;                                 /* entry size                          */
       void                      *entry;                                 /* array of descriptor entries */
     }p4_key_table_t;

#define P4_KEY_ENTRY_NULL ((p4_key_entry_t *)0)
#define P4_KEY_TABLE_NULL ((void *)0)


/*
 * Validity check for key.  This is a macro for speed.
 */
/*
#define p4_key_invalid(key, kt) \
   (P4_KEY_INDEX_OUTRANGE((key),(kt)) ? \
   EACCES : \
   (P4_KEYS_EQUAL((key), \
		  ((struct p4_key_entry *) \
		   (&(kt)->entry[P4_KEY_INDEX(key)]))->key) ? \
    0 : EACCES))
*/

#ifndef _KERNEL
typedef struct p4queue_entry	*queue_t;
typedef	struct p4queue_entry	queue_head_t;
typedef	struct p4queue_entry	queue_chain_t;
typedef	struct p4queue_entry	*queue_entry_t;


/*
 *	Macro:		queue_init
 *	Function:
 *		Initialize the given queue.
 *	Header:
 *		void queue_init(q)
 *			queue_t		q;	
 */
#define queue_init(q)	((q)->next = (q)->prev = q)

/*
 *	Macro:		queue_first
 *	Function:
 *		Returns the first entry in the queue,
 *	Header:
 *		queue_entry_t queue_first(q)
 *			queue_t	q;		/* IN *\
 */
#define queue_first(q)	((q)->next)

/*
 *	Macro:		queue_next
 *	Header:
 *		queue_entry_t queue_next(qc)
 *			queue_t qc;
 */
#define queue_next(qc)	((qc)->next)

/*
 *	Macro:		queue_end
 *	Header:
 *		boolean_t queue_end(q, qe)
 *			queue_t q;
 *			queue_entry_t qe;
 */
#define queue_end(q, qe)	((q) == (qe))

#define queue_empty(q)		queue_end((q), queue_first(q))


/*
 * move from one queue to another queue 
 */

#define queue_move(from, to , elt)             \
   remqueue(from, elt);                        \
   enqueue_tail(to, elt)                      

/*
 *	Remove arbitrary element from queue.
 */

#endif /* NDEF_KERNEL */
#endif /* P4_NSPACE_H */

















