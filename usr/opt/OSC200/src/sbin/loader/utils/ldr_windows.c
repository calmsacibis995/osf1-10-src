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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ldr_windows.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/07 16:20:41 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *	loader windows and loader mappings
 *
 * OSF/1 Release 1.0
 */

/* #define DEBUG */

#include <sys/types.h>
#include <sys/param.h>
#include <loader.h>

#include "ldr_errno.h"

#include "ldr_types.h"
#include "ldr_malloc.h"
#include "ldr_sys_int.h"
#include "ldr_windows.h"
#include "squeue.h"


#ifndef FALSE
#define	FALSE		0
#define	TRUE		1
#endif

#define	INIT_RC		1	/* reference counts start at 1 */
#define	ZERO_RC		0	/* reference count of 0 */

#define	FAILURE		0
#define	SUCCESS		1
#define	FILE_UNMAPPED	2	/* file has been unmapped */

#define MAP_HIGHWATER 	1024000	/* high water mark for addr space usage */

/*
 *	macro for dealing with whole VM pages
 */
#define rounddown(x, y) (((x)/(y))*(y))

struct ldr_mapping_t	{
	struct ldr_mapping_t	*LRU;	/* linked list of mappings arranged in
					   least recently used order - also 
					   used as Next pointer */
	ldr_file_t	fd;		/* file handle for file mapped */
	int		refcount;	/* no of windows associated with map */
	off_t		start;		/* starting pt. of mapping in file */
	size_t		length;		/* no of bytes of file mapped in */
	mode_t		mode;		/* ? ? */
	char		*data;		/* mapped file bytes */
};

typedef struct ldr_mapping_t ldr_mapping_t;


static int map_file(ldr_file_t, off_t, size_t, ldr_mapping_t**);
static int file_mapped(ldr_file_t, off_t, size_t, ldr_mapping_t**);

/*
 *	loader map queue keeps a queue of all file segments mapped
 *	Note : ldr_map_q  and ldr_lru_q should be static.
 *	       Are currently this way for debugging.
 */

struct	ldr_hd_tail_queue_t {
	ldr_mapping_t	*sq_head;
	ldr_mapping_t	*sq_tail;
};

typedef struct ldr_hd_tail_queue_t ldr_hd_tail_queue_t;

ldr_hd_tail_queue_t ldr_map_q_data = { NULL, NULL };
ldr_hd_tail_queue_t *ldr_map_q = &ldr_map_q_data;

/*
 *	loader LRU queue is used to keep files mapped read only
 *	long after no windows to them exist.  These mappings are
 *	destroyed when the address space usage goes over a high
 *	water mark.
 */

ldr_hd_tail_queue_t ldr_lru_q_data = { NULL, NULL };
ldr_hd_tail_queue_t *ldr_lru_q = &ldr_lru_q_data;

int	ldr_map_address_sp_usage = 0;

/*
 *	cache the Virtual Memory Pagesize for this machine
 */

int	ldr_vm_pagesize = 0;	/* will cache right value when first used */


/* Initializer for ldr_window_t's */

static const ldr_window_t initial_wp = {
	NULL,				/* map */
	LDR_FILE_NONE,			/* fd */
	(off_t)0,			/* start offset */
	(size_t)0			/* size */
	};

/*
 *	mappings 
 *	Caveat : Files thru this interface are only mapped 
 *	MAP_PRIVATE | PROT_READ.
 *	NOTE: many of these routines assume locking out other 
 *	processes has been done apriori.
 */

/*
 *	map_file: map a file shared, read only and add it to list of
 *	mapped files.
 *	NOTE: return arg mpp must be allocated by caller.
 */

static int 
map_file(ldr_file_t fd, off_t start, size_t len,
		     ldr_mapping_t **mpp)
{
	caddr_t ma;			/* address where mapped */
	int  pagesize;			/* system page size in bytes */
	off_t newstart;			/* page aligned start */
	size_t newlen;			/* len rounded upto next page end */
	int rc;				/* return code */
	ldr_mapping_t *temp;
	struct addressconf *addr_conf;	/* address config info */

#ifdef DEBUG
	extern int cmd_ldr_walk_map_q();
#endif


	/* 
	 * map file into address space 
	 */

	pagesize = ldr_getpagesize();	/* VM page size */
	if (pagesize <= 0) {
#ifdef DEBUG
		ldr_msg("map_file: invalid page size = %d\n",pagesize);
#endif
		return FAILURE;
	}

	/* align start to page boundary */
	newstart = rounddown(start, pagesize);

	/* 
	 * The length needs adjusment as start has been pulled back
	 * to a page boundary and mapping of whole pages is desired. 
	 */

	newlen = roundup((start - newstart + len), pagesize);

	/* Get address configuration info.  We always map in MMAP area */

	if (ldr_getaddressconf(&addr_conf) != LDR_SUCCESS) /* shouldn't fail */
		return FAILURE;

	rc = ldr_mmap(addr_conf[AC_MMAP_DATA].ac_base, newlen, LDR_PROT_READ,
		      LDR_MAP_PRIVATE| LDR_MAP_FILE, fd, newstart, (univ_t *)&ma);
#ifdef undef
	ldr_msg("map_file: ldr_mmap args : base : 0x%x, len : 0x%x, prot : LDR_ROT_READ, LDR_PRIVATE | LDR_MAP_FILE\n",
	       addr_conf[AC_MMAP_DATA].ac_base, newlen);
	ldr_msg("map_file: ldr_mmap args : fd : 0x%x, start : 0x%x, ma : 0x%x\n",
	       fd, newstart, ma);
#endif
	if ((int) rc != 0) {
#ifdef DEBUG
		ldr_msg("map_file: ldr_mmap error rc = %d\n",rc);
#endif
		return rc;
	}


	if ((rc = ldr_malloc(sizeof(ldr_mapping_t), LDR_MAPPING_T, 
			     (univ_t *)mpp)) != LDR_SUCCESS) {
#ifdef DEBUG
		ldr_msg("map_file: allocation of ldr_mapping_t failed\n");
#endif
		rc = ldr_munmap(ma, newlen);
#ifdef DEBUG
		if (rc != LDR_SUCCESS) {
			ldr_msg("map_file: ldr_munmap failed rc = %d\n",rc);
		}
#endif
		return FAILURE;
	}


	/* 
	 * safely add *mpp to the tail of ldr_map_q. This code assumes
	 * that this is the only process that has access to the queue. 
	 * i.e. it is LOCKED.	
	 */
	sq2_ins_tail((struct squeue2 *)ldr_map_q, (struct sq_elem *)(*mpp));

	(*mpp)->fd = fd;
	(*mpp)->refcount = INIT_RC;
	(*mpp)->start = newstart;	/* remember aligned to page boundary */
	(*mpp)->length = newlen;	/* remember made a multiple of page size */
	(*mpp)->data = (void *) ma;

	/* 
	 * increment amount of address space allocated 
	 */
	ldr_map_address_sp_usage += newlen;

	return SUCCESS;
}

/*
 *	file_mapped : returns TRUE if the specified portion
 *	of the file is mapped together with a ptr to its
 *	mapping structure. Else returns FALSE.
 *
 *	This routine ends up searching both the mapped queue
 *	and the LRU queue in that order.  If the required region
 *	is found on the LRU queue the mapping is moved to the
 *	mapped queue.
 *
 *	NOTE: assumes return argument mpp allocated by caller.
 */

static int 
file_mapped(ldr_file_t fd, off_t off, size_t len, 
			ldr_mapping_t **mpp)
{
	ldr_mapping_t *p, *prev;
	
	/* 
	 * check every mapped file to see if this file is mapped
	 * and if so is it the correct region of the file.
	 * Remember that the region specified could be a sub-region
	 * of a previously mapped region.
	 */
	p = ldr_map_q->sq_head;
	while(p) {
		if (p->fd == fd) 
			if ((off >= p->start) && 
			    ((off + len) <= (p->start + p->length))) {
				*mpp = p;
				return TRUE;
			}
		p = p->LRU;
	}

	/* 
	 * check the lru (least recently used queue) to see if the
	 * required region of the file is already mapped.  If it is
	 * mapped we just move the mapping from the lru queue to the
	 * mapped queue and set its reference count to 0.
	 */
	p = ldr_lru_q->sq_head;
	prev = NULL;

	while(p) {
		if (p->fd == fd) 
			if ((off >= p->start) && 
			    ((off + len) <= (p->start + p->length))) {
				*mpp = p;

				/* remove p from LRU queue */
				if (prev == NULL) prev = (ldr_mapping_t *)
					&ldr_lru_q->sq_head;
				sq2_rem_after((struct squeue2 *)ldr_lru_q, 
					      (struct sq_elem *)p, 
					      (struct sq_elem *)prev);
				/* set p's reference count to zero */
				p->refcount = 0;
				/* add p to head of the mapped queue */
				sq2_ins_head((struct squeue2 *)ldr_map_q, 
					     (struct sq_elem *)p);
#ifdef DEBUG
				ldr_msg("file_mapped: moved map from lru to map q\n");
#endif
				return TRUE;
			}
		prev = p;
		p = p->LRU;
	}

	return FALSE;
}

/*
 *	inc_map_refcount : a macro
 */

#define inc_map_refcount(mp) (mp)->refcount++


/*
 *	dec_map_refcount : decrements the reference count
 *	on the mapping and moves the mapping to the LRU queue  
 *	(ldr_lru_q) if the reference count drops to zero.  
 *	If however the file has been closed (i.e. the fd is
 *	marked invalid) the mapping is unmapped and NOT moved
 *	to the LRU queue.
 *
 *	This returns SUCCESS on decrementing the reference count.
 *	It also returns SUCCESS when the reference count drops
 *	to zero but in this case the mapping is removed from the
 *	map queue and placed on the LRU queue. It returns FAILURE
 *	on error.
 */

int 
dec_map_refcount(univ_t map)
{
	ldr_mapping_t	*mp = map;
	int rc;				/* return code */


	if (mp == NULL)
		return(SUCCESS);

	if (mp->refcount <= ZERO_RC) {	/* invalid reference counts */
#ifdef DEBUG
		ldr_msg("dec_map_refcount: error refcount was = %d\n",
		       mp->refcount);
#endif
		return FAILURE;
	}
	mp->refcount--;
	
	/*
	 * when the reference count on the mapping drops to zero
	 * the mapping is put on the LRU list.
	 */

	if (mp->refcount == ZERO_RC) {
		/* remove mp from ldr_map_q */
		rc = sq2_rem_elem((struct squeue2 *)ldr_map_q, 
				  (struct sq_elem *)mp);

		if (rc == 0) {	/* element not on queue */
#ifdef DEBUG
			ldr_msg("dec_map_refcount: map not on ldr_map_q\n");
#endif
			return FAILURE;
		}

		/* 
		 * Add mp to head of LRU queue if the file handle is valid.
		 * If invalid fd, unmap the file and delete mp.
		 */
		if (mp->fd == LDR_FILE_NONE) {

			rc = ldr_munmap(mp->data, mp->length);
			ldr_map_address_sp_usage -= mp->length;
#ifdef DEBUG
			ldr_msg("dec_map_refcount: mapping unmapped, fd was invalid\n");
#endif
			ldr_free((univ_t)mp);
		}
		else {
			sq2_ins_head((struct squeue2 *)ldr_lru_q, 
				     (struct sq_elem *)mp);

#ifdef DEBUG
			ldr_msg("dec_map_refcount: refcount = 0 moved to LRU queue\n");
#endif
		}
	}
#ifdef DEBUG
	ldr_msg("dec_map_refcount: refcount decrm refcount = %d fd = %d\n",
	       mp->refcount, mp->fd);
#endif
	return SUCCESS;	
}


/*
 *	windowing routines :
 *		ldr_unwindow is a macro see file ldr_windows.h
 */

/*
 *	ldr_init_window: allocate window and initialize its file handle
 */

ldr_window_t* 
ldr_init_window(ldr_file_t fd)
{
	ldr_window_t *wp;
	
	if (ldr_malloc(sizeof(ldr_window_t), LDR_WINDOW_T, (univ_t *)&wp) != LDR_SUCCESS)
		return(NULL);
	*wp = initial_wp;
	wp->fd = fd;
	return wp;
}

/*
 *	ldr_file_window: window in the specified region of the file
 *	returning a pointer to the start of the windowed data or NULL
 *	on error.
 */
univ_t 
ldr_file_window(int start, size_t len, ldr_window_t *wp)
{
	int rc;				/* return code */
	ldr_mapping_t **mpp, *mp, *t;	/* ptr to ptr to mapping,
					   ptr to mapping */

	/*
	 * rewindowing : i.e. using the window to map another region
	 * of the same file. Decrement reference count on old map if
	 * it does not serve the needs for the new window.
	 */

	mp = wp->map;
	if (mp != NULL) {

		/*
		 * if the current mapping is large enough to meet the
		 * needs of the new window just change window attributes
		 * otherwise decrement reference count on mapping and
		 * map in the required region of the file.
		 */

		if ((start >= mp->start) &&
		    ((start + len) <= (mp->start + mp->length))) {
			wp->start = start;
			wp->length = len;
			return (univ_t) (mp->data + (wp->start - mp->start));
		}
		else {
			rc = dec_map_refcount(mp);
			if (rc != SUCCESS) {
#ifdef DEBUG
				ldr_msg("ldr_file_window: error decrem map ref count\n");
#endif
				return NULL;
			}
			wp->map = mp = NULL;
		}
	}

	mp = NULL;
	mpp = &mp;

	/*
	 * Cause file region to be mapped in only if it is not
	 * previously mapped. If previously mapped increment refcount.
	 */

	if (file_mapped(wp->fd, start, len, mpp)) {
		inc_map_refcount(mp);
#ifdef DEBUG
		ldr_msg("ldr_file_window: file mapped previously refcount++\n");
#endif
	}
	else {
		/* flush files (unmap them) off end of LRU queue if address
		 * space usage is over the high water mark.
		 */
		while ((ldr_map_address_sp_usage >= MAP_HIGHWATER) && 
		       (ldr_lru_q->sq_head != NULL)) {

			t = (ldr_mapping_t *)
				sq2_rem_tail((struct squeue2 *)ldr_lru_q);
			if (t == NULL) {
#ifdef DEBUG
				ldr_msg("ldr_file_window: error trying to flush empty lru queue\n");
#endif
				return NULL;
			}
			rc = ldr_munmap(t->data, t->length);
			ldr_map_address_sp_usage -= t->length;
#ifdef DEBUG
			ldr_msg("ldr_file_window: over high water mark fd = %d size = %d\n",
			       t->fd, t->length);
#endif
			ldr_free((univ_t)t);
		}

		rc = map_file(wp->fd, start, len, mpp);
		if (rc != SUCCESS) {
#ifdef DEBUG
			ldr_msg("ldr_file_window: map_file ret code = %d\n",
			       rc);
#endif
			return NULL;
		}
#ifdef DEBUG
		ldr_msg("ldr_file_window: file mapped now\n");
#endif
	}

	wp->map = (univ_t)mp;		/* set window attributes */
	wp->start = start;
	wp->length = len;

#ifdef DEBUG
	ldr_msg("ldr_file_window: windowed data at : 0x%x\n",
	       (mp->data + (wp->start - mp->start)));
#endif
	return (univ_t)(mp->data + (wp->start - mp->start));
}

/*
 * ldr_flush_lru_maps : unmap all mappings on the LRU queue
 */

int 
ldr_flush_lru_maps()
{
	int rc;
	ldr_mapping_t *t;

	while (ldr_lru_q->sq_head != NULL) {
		t = (ldr_mapping_t *)
			sq2_rem_tail((struct squeue2 *)ldr_lru_q);
		if (t == NULL) {
#ifdef DEBUG
			ldr_msg("ldr_flush_lru_maps: error trying to flush empty lru queue\n");
#endif
			return -1;
		}
		rc = ldr_munmap(t->data, t->length);
		ldr_map_address_sp_usage -= t->length;
		ldr_free((univ_t)t);
	}
	return LDR_SUCCESS;
}

/*
 *	ldr_flush_mappings: flushes all mappings from LRU queue for
 *	specified file descriptor.  For those in the mapped queue
 *	the file descriptor is set to an invalid value.
 */

int 
ldr_flush_mappings(ldr_file_t fd)
{
	int rc;
	ldr_mapping_t *prev, *cur, *next;

	prev = (ldr_mapping_t *)
		&ldr_lru_q->sq_head; /* reqd to delete from head of q with sq2_rem_after */
	cur = ldr_lru_q->sq_head; 
	next = (cur != NULL) ? cur->LRU : NULL;

	while (cur != NULL) {
		if (cur->fd == fd) {
			sq2_rem_after(ldr_lru_q, cur, prev);
			rc = ldr_munmap(cur->data, cur->length);
			ldr_map_address_sp_usage -= cur->length;
			ldr_free((univ_t)cur);
		}
		else {
			prev = cur; 
		}

		cur = next;
		next = (cur != NULL) ? cur->LRU : NULL;
	}

	/*
	 *	search the mapped queue for this file and invalidate 
	 *	the file descriptor.
	 */

	prev = NULL; 
	cur = ldr_map_q->sq_head; 

	while (cur != NULL) {
		if (cur->fd == fd) 
			cur->fd = LDR_FILE_NONE;
		prev = cur;
		cur = cur->LRU;
	}

	return LDR_SUCCESS;
}
