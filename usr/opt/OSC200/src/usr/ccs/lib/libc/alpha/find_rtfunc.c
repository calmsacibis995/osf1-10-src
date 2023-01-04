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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef DEBUG
static char *rcs_id="$Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/alpha/find_rtfunc.c,v 1.1.8.4 1993/07/07 20:49:42 Mark_Himelstein Exp $";
#endif
#ifdef index
#undef index
#endif

#ifdef _NAME_SPACE_WEAK_STRONG
#pragma weak exc_add_gp_range = __exc_add_gp_range
#pragma weak exc_add_pc_range_table = __exc_add_pc_range_table
#pragma weak exc_remove_gp_range = __exc_remove_gp_range
#pragma weak exc_remove_pc_range_table = __exc_remove_pc_range_table
#endif

#include "cmplrs/synonyms.h"
#include <excpt.h>

static int		read_semaphore;	/* counting semaphore */
static int		write_semaphore; /* exclusive of reads */
static int		cr_semaphore;	/* cache exclusive lock */

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#include "ts_supp.h"
extern struct rec_mutex _exc_cr_mutex;
extern struct rec_mutex _exc_write_mutex;
extern struct rec_mutex _exc_read_mutex;
extern struct rec_mutex _exc_read_access_mutex;

#define READLOCK 				\
	TS_LOCK(&_exc_write_mutex);		\
	TS_LOCK(&_exc_read_access_mutex);	\
	TS_READLOCK(&_exc_read_mutex);		\
	TS_UNLOCK(&_exc_read_access_mutex);	\
	TS_UNLOCK(&_exc_write_mutex);

#define READUNLOCK 				\
	TS_LOCK(&_exc_read_access_mutex);	\
	if (TS_READUNLOCK(&_exc_read_mutex) == 0) { \
		TS_UNLOCK(&_exc_read_access_mutex); \
		__exc_raise_status_exception(EXC_LOCK_ERROR); \
	}					\
	TS_UNLOCK(&_exc_read_access_mutex);


/* wait till all readers are done and the lock for writing, back off if
 *	we get in at the same time as a reader.
 */
#define WRITELOCK					\
	TS_LOCK(&_exc_write_mutex);			\
	TS_LOCK(&_exc_read_mutex);

#define WRITEUNLOCK \
	TS_UNLOCK(&_exc_read_mutex);			\
	TS_UNLOCK(&_exc_write_mutex);
	
#define CRLOCK	TS_LOCK(&_exc_cr_mutex)
#define CRUNLOCK TS_UNLOCK(&_exc_cr_mutex)
#else
#define WRITELOCK
#define WRITEUNLOCK
#define READLOCK
#define READUNLOCK
#define CRLOCK
#define CRUNLOCK
/* _THREAD_SAFE */
#endif

#define CACHESIZE 128		/* should be power of two */


/* the following structure helps implement a linked list scheme for
 *	dealing with shared objects. When an executable or shared
 *	object starts, it's init section will contain a call to
 *	exc_add_pc_range_table and its fini will call exc_remove_pc_range_table
 *
 * the search routine will first find the correct pc range table and then
 *	binary search it. The linked list of tables is not currently
 *	ordered and shares a single cache.
 */
typedef struct code_range_entry {
    pdsc_crd		*pbase;		/* base of crd table */
    pdsc_count		count;		/* how many entries */
    pdsc_address	first_addr;	/* lowest address covered by table */
    pdsc_address	max_addr;	/* first address not covered by table */
    pdsc_address	gp;		/* used for gp range */
    struct code_range_entry	*next;	/* linked list */
} code_range_entry;


static code_range_entry *
cr_lookup(code_range_entry	*head, exc_address pc)
{
    code_range_entry	*cr;
    static code_range_entry	*last_cr;
    static code_range_entry	*last_head;

    /* lock because there are two elements to our cache and they need 
     *	to be in sync 
     */
    CRLOCK;
    if (last_head == head && last_cr && 
	pc >= last_cr->first_addr && pc < last_cr->max_addr) {
	cr = last_cr;
	CRUNLOCK;
	return cr;
    } /* if */

    for (cr = head; cr != 0; cr = cr->next) {
	if (pc >= cr->first_addr && pc < cr->max_addr) {
	    last_cr = cr;
	    last_head = head;
	    CRUNLOCK;
	    return cr;
	} /* if */
    } /* for */
    CRUNLOCK;
    return 0;
} /* cr_lookup */


static code_range_entry	*cr_head;		/* head of linkled list */

extern void
__exc_add_pc_range_table(
	PRUNTIME_FUNCTION	pbase,		/* base of function table */
	pdsc_count		count)		/* how many */
{
    /* add a table to our linked list of tables which represent a pc range.
     *	the last entry in the table must bound the previous entry.
     */
    /* thread note: we need to lock the head of the list here */
    code_range_entry	*cr;

    if (count == 0) {
	__exc_raise_status_exception(EXC_INVALID_ARGUMENT);
    } /* if */

    WRITELOCK;
    cr = (code_range_entry *)malloc(sizeof(*cr));
    if (cr == 0) {
	/* catastrophic */
	WRITEUNLOCK;
	__exc_raise_status_exception(EXC_OUT_OF_MEMORY);
    } /* if */

    cr->next = cr_head;
    cr->pbase = pbase;
    cr->count = count;
#if defined(__mips64) || defined(__alpha)
    /* get actual address-- these BEGIN ADDRESSES are only relative */
    cr->first_addr = EXCPT_BEGIN_ADDRESS(pbase) + (pdsc_address)pbase;
    cr->max_addr = EXCPT_BEGIN_ADDRESS(pbase+(count-1)) + (pdsc_address)pbase;
#else
    cr->first_addr = EXCPT_BEGIN_ADDRESS(pbase);
    cr->max_addr = EXCPT_BEGIN_ADDRESS(pbase+(count-1));
#endif /* defined(__mips64) || defined(__alpha) */

    cr_head = cr;
    WRITEUNLOCK;
} /* __exc_add_pc_range_table */


extern PRUNTIME_FUNCTION
__exc_lookup_function_table(exc_address pc)
{
    code_range_entry	*cr;
    volatile PRUNTIME_FUNCTION	pbase;
    READLOCK;
    cr = cr_lookup(cr_head, pc);
    if (cr == 0) {
	READUNLOCK;
	return 0;
    } /* if */
    pbase = cr->pbase;
    READUNLOCK;

    return pbase;
} /* __exc_lookup_function_table */

extern void
__exc_remove_pc_range_table(
	PRUNTIME_FUNCTION	pbase)		/* base of function table */
{
    /* remove a table from our linked list of tables which represent a pc range.
     */
    /* thread note: we need to lock the head of the list here */
    code_range_entry	*cr;
    code_range_entry	*last_cr;
    int			found;

    /* find the entry matching the pbase arg and maintain a pointer to
     *	the entry preceding it so we can fix this singley linked list.
     */
    WRITELOCK;
    for ((found = 0), (cr = cr_head), (last_cr = 0); cr != 0 && !found ; 
	cr = cr->next) {
	if (cr->pbase == pbase) {
	    found = 1;
	    break;
	} /* if */
	last_cr = cr;
    } /* for */

    if (!found) {
	/* catastrophic */
	WRITEUNLOCK;
	__exc_raise_status_exception(EXC_RUNTIME_FUNCTION_NOT_FOUND);
    } /* if */

    if (last_cr == 0) {
	/* fix the head */
	cr_head = cr->next;
    } else {
	/* fix preceding element */
	last_cr->next = cr->next;
    } /* if */
    WRITEUNLOCK;

    /* free the entry */
    free(cr);

} /* __exc_remove_pc_range_table */



static code_range_entry	*gp_head;		/* head of linkled list */

extern void
__exc_add_gp_range(
	exc_address		first_addr,	/* base of function table */
	pdsc_count		length,		/* in bytes */
	exc_address		gp)		/* gp for this range */
{
    /* add a table to our linked list of tables which represent a pc range.
     *	the last entry in the table must bound the previous entry.
     */
    /* thread note: we need to lock the head of the list here */
    code_range_entry	*cr;

    cr = (code_range_entry *)malloc(sizeof(*cr));
    if (cr == 0) {
	/* catastrophic */
	__exc_raise_status_exception(EXC_OUT_OF_MEMORY);
    } /* if */

    WRITELOCK;
    cr->next = gp_head;
    cr->first_addr = first_addr;
    cr->max_addr = first_addr+length-1;
    cr->gp = gp;

    gp_head = cr;
    WRITEUNLOCK;
} /* __exc_add_gp_range */

extern void
__exc_remove_gp_range(
	exc_address		first_addr)	/* base of function table */
{
    /* remove a table from our linked list of tables which represent a pc range.
     */
    /* thread note: we need to lock the head of the list here */
    code_range_entry	*cr;
    code_range_entry	*last_cr;
    int			found;

    /* find the entry matching the pbase arg and maintain a pointer to
     *	the entry preceding it so we can fix this singley linked list.
     */
    WRITELOCK;
    for ((found = 0), (cr = gp_head), (last_cr = 0); cr != 0 && !found ; 
	cr = cr->next) {
	if (cr->first_addr == first_addr) {
	    found = 1;
	    break;
	} /* if */
	last_cr = cr;
    } /* for */

    if (!found) {
	/* catastrophic */
	__exc_raise_status_exception(EXC_RUNTIME_FUNCTION_NOT_FOUND);
    } /* if */

    if (last_cr == 0) {
	/* fix the head */
	gp_head = cr->next;
    } else {
	/* fix preceding element */
	last_cr->next = cr->next;
    } /* if */
    WRITEUNLOCK;

    /* free the entry */
    free(cr);

} /* __exc_remove_gp_range */


extern exc_address
__exc_lookup_gp(exc_address	pc)
{
    code_range_entry	*cr;
    extern exc_address	_gp;
    exc_address		gp;

    READLOCK;
    cr = cr_lookup(gp_head, pc);
    if (cr == 0) {
	/* no code range for this entry */
	READUNLOCK;
	return (exc_address)0;
    } /* if */
    gp = cr->gp;
    READUNLOCK;
    return gp;
} /* exc_lookup_gp */


/* function table access function */
extern PRUNTIME_FUNCTION
__exc_lookup_function_entry(exc_address pc)
{
    unsigned int		ilow;
    unsigned int		ihigh;
    unsigned int		ihalf;
    unsigned int		index;
    exc_address			adr;
    exc_address			original_pc;
    static unsigned int		cache[CACHESIZE];
    static code_range_entry	*cr_cache[CACHESIZE];

    code_range_entry		*cr;
    static int			added_function_table;
    extern exc_address		_DYNAMIC_LINK;
    volatile PRUNTIME_FUNCTION	pbase;

    if (!(unsigned long)&_DYNAMIC_LINK && !added_function_table &&
	!_msem_tas(&added_function_table)) {

	/* this only works with non-shared binaries and
	 *	it will create one entry in the linked list of pc ranges
	 *	for the local _fpdata symbol and enter the gp ranges.
	 */

	extern unsigned long	_gpinfo[];
	extern unsigned long	_fdata[];
	extern unsigned long _gp;
	extern _ftext();
	extern _etext();
	unsigned long		*gpinfo;


	gpinfo = (unsigned long *)_gpinfo;

	__exc_add_pc_range_table(function_table, function_table_size);
	added_function_table = 1;

	if (gpinfo[0] == GPINFO_MAGIC) {


	    gpinfo++;


	    while (gpinfo[0] != GPINFO_LAST) {
		__exc_add_gp_range(gpinfo[0]+(exc_address)_ftext, gpinfo[1], 
			gpinfo[2]+(exc_address)_fdata);
		gpinfo += 3;
	    } /* while */

	} else {
	    __exc_add_gp_range((exc_address)_ftext,
		    (exc_address) _etext - (exc_address)_ftext, 
		    (exc_address)&_gp);
	} /* if */
    } /* if */

    READLOCK;
    cr = cr_lookup(cr_head, pc);
    if (cr == 0) {
	/* no code range for this entry */
	READUNLOCK;
	return 0;
    } /* if */

#if defined(__mips64) || defined(__alpha)
    original_pc = pc;
    /* cheaper to use function table relative addresses */
    pc = (pdsc_address)((long)pc - (long)cr->pbase);
#endif


    /* don't need to lock-- if we're wrong, we're wrong */
    ihalf = cache[index = ((pc >> 2) & (CACHESIZE - 1))];
    if (cr_cache[index] != cr) {
	/* probably a lookup in some other table since it's beyond the end of
	 * our table
	 */
	ihalf = cr->count >> 1;
    } /* if */
    adr = EXCPT_BEGIN_ADDRESS(cr->pbase+ihalf);
    if (pc < adr)
    {
	ihigh = ihalf;
	ilow = 0;
    }
    else
    {
	if (pc < EXCPT_BEGIN_ADDRESS(cr->pbase+ihalf+1))
	    goto gotit;
	ihigh = cr->count - 1;
	ilow = ihalf + 1;
    } /* if */

    /* binary search function_table */
    ihalf = (ilow + ihigh) >> 1;

    while (ilow < ihigh)
    {
	adr = EXCPT_BEGIN_ADDRESS(cr->pbase+ihalf);
	if (pc < adr)
	    ihigh = ihalf;
	else if (pc > adr)
	{
	    ilow = ihalf;
	    if (ilow == ihigh - 1)
		break;
	}
	else
	{
	    ilow = ihigh = ihalf;
	    break;
	} /* if */
	ihalf = (ilow + ihigh) >> 1;
    } /* while */

    if (ilow < ihigh && pc > EXCPT_BEGIN_ADDRESS(cr->pbase+ihigh))
	ihalf = ihigh;

    /* need a lock since there are two datums that need to be in sync */
    pbase = cr->pbase + ihalf;
    READUNLOCK;
    WRITELOCK;
    cache[index] = ihalf;
    cr_cache[index] = cr;
    WRITEUNLOCK;
    goto over;

gotit:
    pbase = cr->pbase + ihalf;
    READUNLOCK;
over:

#ifdef DEBUG
    printf("__exc_lookup_function_entry: (pc = 0x%lx, 0x%lx) => (entry = 0x%lx) (index = %d)\n",
	original_pc, pc, EXCPT_BEGIN_ADDRESS(cr->pbase+ihalf), ihalf);
#endif DEBUG


    return pbase;
} /* __exc_lookup_function_entry */



extern char *
__exc_internal_to_ascii(unsigned long	status)
{
    switch (status) {
    case EXC_STATUS_UNWIND: 
	return "unwinding";
    case EXC_STATUS_NONCONTINUABLE_EXCEPTION: 
	return "tried to continue non-continuable exception";
    case EXC_STATUS_INVALID_DISPOSITION: 
	return "handler returned invalid disposition";
    case EXC_SIGNAL_EXPECTED: 
	return "handler expected valid signal value";
    case EXC_RUNTIME_FUNCTION_NOT_FOUND: 
	return "runtime function entry or table not found";
    case EXC_INFINITE_LOOP_UNWIND: 
	return "exception dispatch or unwind stuck in infinite loop";
    case EXC_INVALID_DISPATCHER_CONTEXT: 
	return "dispatcher context corrupted on collided unwind";
    case EXC_LOCK_ERROR: 
	return "locking error on exception resources shared among threads";
    case EXC_INVALID_EXCEPTION_RECORD: 
	return "encountered invalid exception record";
    case EXC_UNSUPPORTED: 
	return "unsupported functionality";
    case EXC_STACK_OVERFLOW: 
	return "stack overflow";
    case EXC_OUT_OF_MEMORY: 
	return "out of memory trying to allocate exception system resources";
    case EXC_INVALID_ARGUMENT: 
	return "invalid argument to exception system routine";
    default:
	return 0;
    } /* switch */
} /* exc_internal_to_ascii */

unsigned long
__exc_get_fp_type(
	unsigned long	levels_up	/* how far to traceback */
)
{
    /* trace back stack levels_up frames where 0 is the frame which
     *	call this routine, 1 is their callers, etc.
     */

    struct sigcontext	scp;
    PRUNTIME_FUNCTION	pfunc;

    /* find my context */
    setjmp(&scp);

    /* get to my callers context */
    (void)__exc_virtual_unwind(0, &scp);

    /* get desired context */
    while(levels_up--) {
	(void)__exc_virtual_unwind(0, &scp);
    } /* while */

    pfunc = __exc_lookup_function_entry(scp.sc_pc);
    return ((PDSC_RPD_FLAGS(PDSC_CRD_PRPD(pfunc))) & PDSC_EXC_MASK);
} /* __exc_get_fp_type */
