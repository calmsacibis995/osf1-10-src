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
static char *rcsid = "@(#)$RCSfile: str_memory.c,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1993/08/19 12:25:15 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/** Copyright (c) 1989-1991  Mentat Inc.  **/

#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/stat.h>

#if SEC_BASE
#include <kern/assert.h>
#endif

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/strstat.h>
#include <sys/sysconfig.h>

#include <net/netisr.h>

#include <streams_memdbg.h>
#if STREAMS_MEMDBG
#define LAST_MAX 5
typedef struct str_used {
       MBLKP     mblk;     /* Ptr to the mblk in use */
       int       line;     /* line # within file where alloc happened */
       char      file[255]; /* name of source file where alloc happened */
       int       last_cnt;
       struct {
	   char  last_file[255];
	   int   last_line;
       } last[LAST_MAX];
       struct str_used *next;
       struct str_used *prev;
} STRUSED;
struct str_used *str_memused = NULL;
MBLKP debug_allocb(int size, int pri, char *file, int line);
void debug_freeb (MBLKP mp, char *file, int line);
void str_addmemdebug(struct str_used **head, MBLKP addr, char *file, int line);
void str_remmemdebug(MBLKP addr);
#endif /* STREAMS_MEMDBG */

/*
 * Storage allocation for everything but messages
 *
 * CONTEXT	These routines are called only in process context
 *		(data structures other than messages are allocated
 *		only by stream head code, and that is always forced
 *		into process context) - thus, we may use blocking
 *		memory allocation services.
 *
 * PRIORITES	are passed to us for historical reasons. This module
 *		used to allocate any dynamically needed memory.
 *
 * EXPORTED: 	he_alloc(size, pri)
 *		he_free(data)
 *		he_realloc(data, old_size, new_size);
 */

caddr_t
he_alloc (size, pri)
	long	size;
	int	pri;
{
	register caddr_t addr;

	ENTER_FUNC(he_alloc, size, pri, 0, 0);

	STR_MALLOC(addr, caddr_t, size, M_STREAMS, M_WAITOK);

	LEAVE_FUNC(he_alloc, addr);
	return addr;
}

void
he_free (addr)
	caddr_t	addr;
{
	ENTER_FUNC(he_free, addr, 0, 0, 0);

	STR_FREE(addr, M_STREAMS);

	LEAVE_FUNC(he_free, 0);
}

caddr_t
he_realloc (old_addr, old_size, new_size)
	caddr_t	old_addr;
	long	old_size;
	long	new_size;
{
	register caddr_t	new_addr;

	ENTER_FUNC(he_realloc, old_addr, old_size, new_size, 0);

	if (new_size <= old_size) {
		LEAVE_FUNC(he_realloc, old_addr);
		return(old_addr);
	}

	if ((new_addr = he_alloc(new_size, BPRI_LO)) != NULL) {
		bcopy(old_addr, new_addr, old_size);
		bzero(new_addr + old_size, new_size - old_size);
		he_free(old_addr);
	}
	LEAVE_FUNC(he_realloc, new_addr);
	return(new_addr);
}
 
/*
 *	Storage allocation for messages
 */

decl_simple_lock_data(static,bufc_lock)
decl_simple_lock_data(,mblk_lock)

int	NSTREVENT = 20;

typedef struct bufcall_s {
	struct bufcall_s	* bc_next;	/* Must be first */
	struct bufcall_s	* bc_prev;
	uint			bc_size;
	int			bc_pri;
	bufcall_fcn_t		bc_fcn;
	bufcall_arg_t		bc_arg;
	int			bc_id;
	queue_t *		bc_queue;
} BUFCALL, * BUFCALLP, ** BUFCALLPP;

static struct module_info bminfo =  {
	0, "bufcall", 0, INFPSZ, 2048, 128
};

static struct qinit brinit = {
	NULL, bufcall_rsrv, NULL, NULL, NULL, &bminfo
};

struct streamtab bufcallinfo = { &brinit, &brinit };

int
bufcall_configure (op, indata, indatalen, outdata, outdatalen)
        sysconfig_op_t  op;
        str_config_t *  indata;
        size_t          indatalen;
        str_config_t *  outdata;
        size_t          outdatalen;
{
	struct streamadm	sa;
	dev_t			devno;

	if (op != SYSCONFIG_CONFIGURE)
		return EINVAL;

	sa.sa_version		= OSF_STREAMS_10;
	sa.sa_flags		= STR_IS_MODULE;
	sa.sa_ttys		= nil(caddr_t);
	sa.sa_sync_level	= SQLVL_MODULE;
	sa.sa_sync_info		= nil(caddr_t);
	strcpy(sa.sa_name, 	"bufcall");

	if ((devno = strmod_add(NODEV, &bufcallinfo, &sa)) == NODEV)
		return ENODEV;

        if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
                outdata->sc_version = OSF_STREAMS_CONFIG_10;
                outdata->sc_devnum = NODEV;
                outdata->sc_sa_flags = sa.sa_flags;
                strcpy(outdata->sc_sa_name, sa.sa_name);
        }

	return 0;
}

staticf	struct { BUFCALLP next, prev; } bc_q;
staticf	BUFCALLP	bc_free;
staticf	queue_t	*	bufcallq;
staticf	int		bufcall_needed  = 0;
staticf	int		bufcall_happened = 0;

int
bufcall_rsrv (q)
	queue_t	* q;
{
reg	BUFCALLP	bc, nextbc;
	int		threshold;
	SQ		sq_data;
	SIMPLE_LOCK_DECL

	ENTER_FUNC(bufcall_rsrv, q, 0, 0, 0);

	sq_init(&sq_data);
	SIMPLE_LOCK(&bufc_lock);
again:
	untimeout(qenable, bufcallq);
	bufcall_happened = 0;
	threshold = (STRMSGSZ > 0) ? STRMSGSZ+1 : INT_MAX;
	for ( nextbc = bc_q.next; nextbc != (BUFCALLP)&bc_q; ) {
		bc = nextbc;
		nextbc = nextbc->bc_next;
		if (bc->bc_size >= threshold)
			continue;
		if (testb(bc->bc_size, bc->bc_pri) == 0) {
			threshold = bc->bc_size;
			continue;
		}
		remque(bc);
		SIMPLE_UNLOCK(&bufc_lock);

		(void) csq_protect(	bc->bc_queue,
					nil(queue_t *),
					(csq_protect_fcn_t)bc->bc_fcn,
					(csq_protect_arg_t)bc->bc_arg,
					&sq_data, TRUE);

		SIMPLE_LOCK(&bufc_lock);
		bc->bc_next = bc_free;
		bc_free = bc;
		/*
		 * If bufcall() was called while we didn't
		 * hold the bufc_lock, we have to restart
		 * the loop.
		 */
		if ( bufcall_happened )
			goto again;
	}
	if (bc_q.next == (BUFCALLP)&bc_q)
		bufcall_needed = 0;
	else
		timeout(qenable, bufcallq, hz);
	SIMPLE_UNLOCK(&bufc_lock);

	LEAVE_FUNC(bufcall_rsrv, 0);
	return 0;
}

int
(bufcall)(s, p, f, a)
	uint s; int p; int (*f)(); long a;
{
	return bufcall(s, p, (bufcall_fcn_t)f, (bufcall_arg_t)a);
}

/** Recover from failure of allocb */
int
streams_bufcall (size, pri, func, arg)
	uint	size;
	int	pri;
	bufcall_fcn_t	func;
	bufcall_arg_t	arg;
{
static	int		bufcall_id;
reg	BUFCALLP	bc;
	SIMPLE_LOCK_DECL

	ENTER_FUNC(bufcall, size, pri, func, arg);

	SIMPLE_LOCK(&bufc_lock);
	if (!(bc = bc_free)) {
		SIMPLE_UNLOCK(&bufc_lock);
		STR_DEBUG(printf("WARNING: bufcall: could not allocate stream event\n"));
		LEAVE_FUNC(bufcall, 0);
		return 0;
	}
	bc_free = bc->bc_next;
	bc->bc_size = size;
	bc->bc_pri = pri;
	bc->bc_fcn = func;
	bc->bc_arg = arg;
	if ((bc->bc_id = ++bufcall_id) == 0)
		bc->bc_id = ++bufcall_id;

#if	NETISR_THREAD
	if (func != (bufcall_fcn_t)qenable &&
	    func != (bufcall_fcn_t)osrq_wakeup)
		bc->bc_queue = csq_which_q();
	else
#endif
		bc->bc_queue = nil(queue_t *);

	insque(bc, bc_q.prev);
	if (bufcall_needed++ == 0)
		timeout(qenable, bufcallq, hz);
	bufcall_happened = 1;
	SIMPLE_UNLOCK(&bufc_lock);

	LEAVE_FUNC(bufcall, 1);
	return bc->bc_id;
}

void
unbufcall (id)
	int	id;
{
reg	BUFCALLP	bc;
	SIMPLE_LOCK_DECL

	SIMPLE_LOCK(&bufc_lock);
	for (bc = bc_q.next; bc != (BUFCALLP)&bc_q; bc = bc->bc_next) {
		if (bc->bc_id == id) {
			remque(bc);
			bc->bc_next = bc_free;
			bc_free = bc;
			break;
		}
	}
	SIMPLE_UNLOCK(&bufc_lock);
}

/* Used to initialize mblks in allocb. */
static	MH	mh_template;
/* "Attached" mblks held here for safe freeing */
MHP	mh_freelater;

int
bufcall_init ()
{
reg	BUFCALL	* bc;

	bc_q.next = bc_q.prev = (BUFCALLP)&bc_q;
	simple_lock_init(&bufc_lock);
	simple_lock_init(&mblk_lock);
	mh_template.mh_dblk.db_ref = 1;
	mh_template.mh_dblk.db_type = M_DATA;
	/* Others are 0 or initialized on demand */

	STR_MALLOC(bc, BUFCALL *, NSTREVENT * sizeof *bc, M_STREAMS, M_WAITOK);
	if (bufcallq = q_alloc()) {
		sth_set_queue(	bufcallq,
				bufcallinfo.st_rdinit,
				bufcallinfo.st_wrinit);
		for (bc_free = bc; bc < &bc_free[NSTREVENT-1]; bc++)
			bc->bc_next = &bc[1];
		bc->bc_next = NULL;
		return 0;
	}
	STR_FREE(bc, M_STREAMS);
	panic("bufcall_init");
}

#if STREAMS_MEMDBG

/* 
 * Under the STREAMS_MEMDBG conditional, add code that tracks used
 * blocks. Achieve this by changing the definition of allocb
 * to a macro that calls debug_allocb with the file name being
 * compiled and the source line number in addition to the usual
 * arguments. Record this information in a queue that hangs off the
 * global str_memused.
 *
 * Be sure that all returns to normal when STREAMS_MEMDBG is not defined.
 */

/* allocb
 *
 * Include this definition of allocb for objects that will
 * not be rebuilt for this effort. The idea is supposed to be
 * that none of these objects are being called in our debug
 * environment. If in fact they do get called, this print
 * will show that something unexpected is being called.
 */

MBLKP allocb (size, pri)
	int size;
	uint pri;
{
    return (debug_allocb (size, pri, "Unknown", 0));
}


/*
 * allocb - allocate a message block.
 */

MBLKP debug_allocb (size, pri, file, line)
reg	int	size;
	int	pri;
	char   *file;
	int	line;

#else	/* ifdef STREAMS_MEMDBG */

/*
 * allocb - allocate a message block.
 */

MBLKP
allocb (size, pri)
reg	int	size;
	uint	pri;
#endif /* STREAMS_MEMDBG */
{
reg	MHP	mh;
	uchar *	mem;

	ENTER_FUNC(allocb, size, pri, 0, 0);

	if (size < 0 || (STRMSGSZ > 0 && size > STRMSGSZ))
		goto bad;
	pri = (pri == BPRI_WAITOK) ? M_WAITOK : M_NOWAIT;
	if (size <= 256 - sizeof (MH)) {
		STR_MALLOC(mh, MHP, 256, M_MBLK, pri);
		mem = (uchar *)(mh + 1);
		if (size) size = 256 - sizeof (MH);
	} else {
		STR_MALLOC(mh, MHP, sizeof (MH), M_MBLK, pri);
		if (mh) {
			/*
			 * Round up size to mallocator's quantum.
			 * XXX we know too much.
			 */
			if (size & (size - 1))
				size = (1 << BUCKETINDX(size));
			STR_MALLOC(mem, uchar *, size, M_MBDATA, pri);
		}
	}
	if (mh == 0 || mem == 0) {
		if (mh) STR_FREE(mh, M_MBLK);
bad:
		LEAVE_FUNC(allocb, 0);
		return nil(MBLKP);
	}
	*mh = mh_template;
	sq_init(&mh->mh_mblk.b_sq);
	mh->mh_mblk.b_datap = &mh->mh_dblk;
	mh->mh_mblk.b_rptr = mh->mh_mblk.b_wptr = mh->mh_dblk.db_base = mem;
	mh->mh_dblk.db_lim = &mem[size];
	mh->mh_dblk.db_size = size;
#if STREAMS_MEMDBG
        str_addmemdebug(&str_memused, &mh->mh_mblk, file, line);
#endif
	LEAVE_FUNC(allocb, &mh->mh_mblk);
	return &mh->mh_mblk;
}

#if STREAMS_MEMDBG
void
freeb (bp)
	MBLKP	bp;
{
    debug_freeb(bp, "Unknown", 0);
}


void
debug_freeb (mp, file, line)
        MBLKP    mp;
	char *file;
	int line;

#else /* if STREAMS_MEMDBG is not defined, then */

/*
 * freeb - free a message block
 */
void
freeb (mp)
	MBLKP	 mp;
#endif /* STREAMS_MEMDBG */
{
reg	MHP		mh;
	dblk_t *	db;
	uchar		*md1, *md2;
	MH		*mh1, *mh2;
	SIMPLE_LOCK_DECL

	if (mp->b_datap == 0 || mp->b_datap->db_ref == 0)
		panic("freeing free mblk");
	ENTER_FUNC(freeb, mp, 0, 0, 0);
#if SEC_BASE
	/*
	 * If there are security attributes associated with this buffer,
	 * release them first.
	 */
	if (mp->b_attr) {
		ASSERT(	!mp->b_attr->b_attr && \
			!mp->b_attr->b_cont && \
			!mp->b_attr->b_next);
		freeb(mp->b_attr);
		mp->b_attr = nil(MBLKP);
	}
#endif
	md1 = md2 = NULL;
	mh1 = mh2 = NULL;
	SIMPLE_LOCK(&mblk_lock);
	db = mp->b_datap;
	mp->b_datap = NULL;
	db->db_ref--;
	mh = (MHP)(&((MBLKP)db)[-1]);
	/*
	 * Following loop executed at most twice - once for the
	 * mp->b_datap and its associated mblk, and a second
	 * time if that mblk isn't mp. Set mh* and md* as we go
	 * in order to take mblk_lock and splstr only once.
	 */
	for (;;) {
		if ( db->db_ref == 0 ) {
			uchar *p = db->db_base;
			db->db_base = NULL;
			/*
			 * If free_func, defer free to memory isr thread when
			 * _both_ the mblk and dblk are unreferenced. (We need
			 * the mblk and the mh_frtn to queue it)
			 */
			if ( mh->mh_frtn.free_func ) {
				if ( mh->mh_mblk.b_datap == NULL
				&&   mh->mh_dblk.db_base == NULL ) {
					mh->mh_mblk.b_prev =(MBLKP)p;
					mh->mh_mblk.b_next =(MBLKP)mh_freelater;
					mh_freelater = mh;
				} else	/* later... */
					db->db_base = p;
			/*
			 * Else free any mbdata and unused mblk.
			 */
			} else {
				if ( p != (uchar *)(mh + 1) )
					if (md1) md2 = p; else md1 = p;
				if ( mh->mh_mblk.b_datap == NULL
				&&   mh->mh_dblk.db_base == NULL )
					if (mh1) mh2 = mh; else mh1 = mh;
			}
#if STREAMS_MEMDBG
			str_remmemdebug(&mh->mh_mblk);
#endif
		}
		if ( mh == (MHP)mp )
			break;
		mh = (MHP)mp;
		db = &mh->mh_dblk;
	}
	mh = mh_freelater;
	SIMPLE_UNLOCK(&mblk_lock);
	if (mh1) STR_FREE(mh1, M_MBLK);
	if (mh2) STR_FREE(mh2, M_MBLK);
	if (md1) STR_FREE(md1, M_MBDATA);
	if (md2) STR_FREE(md2, M_MBDATA);
	if (mh)  schednetisr(NETISR_MB);
	if (bufcall_needed)
		qenable(bufcallq);

	LEAVE_FUNC(freeb, 0);
}

/*
 * Called from NETISR_MB for interrupt-safe freeing.
 */
void
ext_freeb()
{
reg	MHP	mh, mh_next;
	char *	p;
	SIMPLE_LOCK_DECL

	SIMPLE_LOCK(&mblk_lock);
	mh = mh_freelater;
	mh_freelater = 0;
	SIMPLE_UNLOCK(&mblk_lock);
	while (mh) {
		if (p = (char *)mh->mh_mblk.b_prev)
			(*mh->mh_frtn.free_func)(mh->mh_frtn.free_arg, p);
		mh_next = (MHP)mh->mh_mblk.b_next;
		STR_FREE(mh, M_MBLK);
		mh = mh_next;
	}
}
#if STREAMS_MEMDBG
void
str_addmemdebug(struct str_used **head, MBLKP addr, char *file, int line)
{
    struct str_used *tail, *curr;

    if (!(curr = (struct str_used *) kalloc(sizeof(struct str_used))))
          printf("str_addmemdebug: unable to allocate space\n");
    bzero(curr, sizeof(struct str_used));
    curr->mblk = addr;
    strncpy(curr->file, file, sizeof(curr->file));
    curr->line = line;
    curr->prev = NULL;
    if (*head) {
        curr->next = *head;
	(*head)->prev = curr;
    } else
        curr->next = NULL;
    *head = curr;
}

void 
str_remmemdebug(MBLKP addr)
{
    struct str_used *curr, *next, *prev, *cur_list;
    for (curr = str_memused; curr; curr = curr->next) {
        if (curr->mblk == addr)
	    break;
    }
    if (!curr) {
        printf("str_remmemdebug: uanble to find msg 0x%lx\n", addr);
        return;
    }
    next = curr->next;
    prev = curr->prev;
    if (next)
        next->prev = curr->prev;
    if (prev)
        prev->next = curr->next;
    if (str_memused == curr)
        str_memused = curr->next;
    kfree(curr, sizeof(struct str_used));
}

void 
str_add_func(MBLKP addr, char *file, int line)
{
    struct str_used *curr;
    for (curr = str_memused; curr; curr = curr->next) {
        if (curr->mblk == addr)
	    break;
    }
    if (!curr) {
        printf("str_add_func: uanble to find msg 0x%lx\n", addr);
        return;
    }
    if (curr->last_cnt >= LAST_MAX)
        return;
    curr->last[curr->last_cnt].last_line = line;
    strncpy(curr->last[curr->last_cnt].last_file, file, 
	    sizeof(curr->last[curr->last_cnt].last_file));
    curr->last_cnt++;
}

#endif

