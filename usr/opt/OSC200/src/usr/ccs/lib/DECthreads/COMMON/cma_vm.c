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
 * @(#)$RCSfile: cma_vm.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:54:20 $
 */
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	Manage virtual memory (isolate O/S dependencies)
 *
 *  AUTHORS:
 *
 *	Dave Butenhof, Paul Curtin
 *
 *  CREATION DATE:
 *
 *	19 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	30 August 1989
 *		Fix assertion macro to proper name.
 *	002	Dave Butenhof	26 September 1989
 *		Add locking.
 *	003	Dave Butenhof	12 October 1989
 *		Use internal mutex for locking (less overhead)
 *	004	Dave Butenhof	Halloween 1989
 *		Use cma_error to report malloc failure instead of assert.
 *	005	Dave Butenhof	1 December 1989
 *		Add external malloc/free routines which use interlocks.
 *	006	Dave Butenhof	7 December 1989
 *		Add cma_cfree routine.
 *	007	Dave Butenhof	10 April 1990
 *		Name the VM mutex for debugging.
 *	008	Dave Butenhof	29 May 1990
 *		Make VM mutex extern so it can be accessed by rtl linking
 *		against libcma.a.
 *	009	Dave Butenhof	26 June 1990
 *		Replace use of private "VM mutex" with the generic "global
 *		mutex".
 *	010	Paul Curtin	14 August 1990
 *		Added internal memory allocation scheme w/ look-aside lists
 *		and memory pool.
 *	011	Webb Scales	16 August 1990
 *		Replace #ifdef's with #if's
 *	012	Dave Butenhof	21 August 1990
 *		Fix errors in function declarations detected by the MIPS C
 *		compiler.
 *	013	Webb Scales	21 August 1990
 *		Reworked get_area to remove duplicated code between locking
 *		and non-locking versions.  Broke dependency of CMA memory 
 *		management on stack chunk size.
 *	014	Paul Curtin	22 August 1990
 *		Removed cma__alloc_mem_nolock code.  Added cma___mem_lock, 
 *		cma___mem_unlock, and cma___segment_pool.  Perform run-time 
 *		check to lock or not now.  Round pool to largest mem object.
 *	015	Paul Curtin	10 September 1990
 *		Fix allocation assertion check, had a 1 byte hole.
 *	016	Dave Butenhof	3 October 1990
 *		Fix deallocation of "other" sized chunks; there's no count or
 *		lock in the pool array, and it's nice to avoid memory access
 *		violations...
 *	017	Dave Butenhof	25 October 1990
 *		Add name to mutexes created at init time.
 *	018	Dave Butenhof	29 October 1990
 *		Fix several occurrences of "cma__c_large" which ought to be
 *		"cma__c_large_alloc".  Also fix cma__free_mem to break up and
 *		use the "oversized" packet being returned, rather than the
 *		remaining memory pool.
 *	019	Paul Curtin	 7 November 1990
 *		Added some variables to track internal mem alloc.
 *	020	Paul Curtin	 8 January 1991
 *		Added try block to cma__get_area to catch exc's and
 *		unlock a mutex.
 *	022	Dave Butenhof	22 January 1991
 *		Fix exception names
 *	023	Paul Curtin	20 February 1991
 *		Round up return address from sbrk to be quadword aligned
 *	024	Webb Scales	12 March 1991
 *		Added HP-specific page sizing code.
 *	025	Dave Butenhof	25 March 1991
 *		Change from cma_exception to exc_handling
 *	026	Dave Butenhof	11 April 1991
 *		CATCH exc_e_insfmem from cma__get_area_nolock and resignal
 *		after appropriate cleanup.
 *	027	Paul Curtin	18 April 1991
 *		Only round up in __get_area_nolock if succussful.
 *	028	Dave Butenhof	01 May 1991
 *		Add arguments to cma__bugcheck() calls.
 *	029	Paul Curtin	09 May 1991
 *		Added a RERAISE to a CATCH_ALL clause.
 *	030	Paul Curtin	24 May 1991
 *		Added cma__reinit_memory.
 *	031	Paul Curtin	29 May 1991
 *		get_area for init pool uses cma__c_pool_init solely.
 *	032	Paul Curtin	 5 June 1991
 *		Rearranged flags in reinit routine.
 *	033	Dave Butenhof	19 September 1991
 *		Integrate HPUX CMA5 reverse drop: change HP page size.
 *	034	Dave Butenhof	08 October 1991
 *		With advent of dynamically allocated file descriptor related
 *		structures, we may need to allocated more than the "pool
 *		size" for the structures (for example, just any array of 4096
 *		pointers is 16,384 bytes where the current pool size limit is
 *		10,240). Add code to allocate directly instead of going to
 *		pool. Also, remove a lot of excess locking in cma__alloc_mem.
 *	035	Dave Butenhof	30 October 1991
 *		Handle exceptions on allocation calls.
 *	036	Paul Curtin	02 December 1991
 *		Added a call to getsyiw on Alpha to determine page size.
 *	037	Paul Curtin	20 December 1991
 *		Remove starlet.h include on VAX
 *	038	Dave Butenhof	10 February 1992
 *		Add cma__alloc_zero to allocate pre-cleared memory. Otherwise
 *		identical to cma__alloc_mem. Also, change allocation
 *		functions to return NULL rather than raising cma_e_insfmem;
 *		this will remove a lot of TRY overhead within DECthreads.
 *		It's too bad we have to compromise our clean exception based
 *		design: but the fact is that performance is more important
 *		within the library.
 *	039	Dave Butenhof	12 February 1992
 *		Rework VM to merge freed general pool packets, and provide
 *		logic to reclaim unused lookaside list packets when memory
 *		gets tight. Remove some entry points (like cma__get_area) and
 *		merge others. Generally clean up some. Also, provide more
 *		pool statistics in !NDEBUG builds.
 *	040	Dave Butenhof	18 February 1992
 *		Fix a typo (missing ';' in UNIX code).
 *	041	Webb Scales	18 February 1992
 *		Fixed a compilation error in memory packet counting.
 *	042	Dave Butenhof	19 February 1992
 *		Yet another bug... UNIX cma___get_area detects unaligned sbrk
 *		result incorrectly.
 *	043	Webb Scales	11 May 1992
 *		"Clean" queue pointers in freed memory block before enqueuing.
 *		Added routine for debugging use which clean's queue pointers
 *		in a object's header.
 *	044	Paul Curtin	26 June 1992
 *		Changed hardcode value for EVMS syi$_page_size to reflect
 *		EVMS changes from FT2 to FT3 (sigh... wish it was in starlet.h)
 *	045	Paul Curtin	4 September 1992
 *		Added an IOSB to sys$getsyi call, added include for 
 *		cma_vp_defs.h.
 *	046	Dave Butenhof & Webb Scales	10 September 1992
 *		Back off the pool lock before calling cma___get_area, to
 *		avoid potential deadlocks between pool and the global lock.
 *	047	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	048	Dave Butenhof	15 April 1993
 *		Add tracing
 *	049	Dave Butenhof	17 April 1993
 *		Fix type of return pointer for queue op
 *	050	Dave Butenhof	 5 May 1993
 *		Unconditionalize some VM statistics (relatively cheap and
 *		infrequent).
 *	051	Dave Butenhof	25 May 1993
 *		Reduce memory fragmentation -- the "medium" packet is an odd
 *		fraction of a page (and the most common allocation) and thus
 *		leaves holes in pool. Change vm init to round packet sizes up
 *		to next power of 2 (this is what malloc almost always does).
 *	052	Dave Butenhof	 3 June 1993
 *		Fix cma__init_object(), which was apparently supposed to
 *		return the input argument, but did so only by sheer accident
 *		since there was no return statement!
 *	053	Dave Butenhof	 1 July 1993
 *		Add a cma__queue_zero() call before inserting to avoid
 *		spurious queue corruption assertions on debug builds.
 */

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_defs.h>
#include <cma_mutex.h>
#include <cma_vm.h>
#include <cma_errors.h>
#include <cma_queue.h>
#include <cma_assert.h>
#include <cma_vp_defs.h>
#if _CMA_OS_ == _CMA__VMS
#include <stdlib.h>			/* Define UNIX VM prototypes */
# if _CMA_HARDWARE_ == _CMA__ALPHA
#  include <starlet.h>
# endif
#else
#include <sys/types.h>
#endif


/*
 *  LOCAL MACROS
 */

/*
 * Round byte-count up to the next integral page
 */
#define cma___roundup_page_size(sz)	cma__roundup(sz, cma__g_page_size)

/*
 * Perform run time checks on cma___g_mem_init_done inorder to decide
 * whether or not to perform locking on the given mutex.  The flag
 * should be set during initialization.
 */

#define cma___mem_lock(mutex) \
    if (cma___g_mem_init_done)  cma__int_lock(mutex)

#define cma___mem_unlock(mutex) \
    if (cma___g_mem_init_done)  cma__int_unlock(mutex)

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
# ifndef SYI$_PAGE_SIZE
#  define SYI$_PAGE_SIZE 4452
# endif
#endif

/*
 *  GLOBAL DATA
 */

cma__t_alloc_queue	cma__g_memory[cma__c_alloc_sizes];
cma_t_integer		cma__g_page_size;
#if _CMA_OS_ == _CMA__UNIX
 cma_t_integer		cma__g_sbrk_align = 0;
#endif
cma__t_pool_stat	cma__g_pool_stat = {0};
char			*cma__g_vm_names[cma__c_alloc_sizes] = {
    "small",
    "medium",
    "large",
    "pool"
    };


/*
 *  LOCAL DATA
 */

static cma_t_boolean	cma___g_mem_init_done = cma_c_false; 

/*
 * Packet sizes (size of structure rounded up to next power of two and then
 * the fixed header subtracted to make range checking faster)
 */
static cma_t_natural	cma___g_small_packet;
static cma_t_natural	cma___g_med_packet;
static cma_t_natural	cma___g_large_packet;

/*
 * LOCAL FUNCTIONS
 */

static cma_t_address
cma___alloc_from_pool _CMA_PROTOTYPE_ ((
	cma_t_natural	size));

static void
cma___free_to_pool _CMA_PROTOTYPE_ ((
	cma_t_address	packet));

static cma_t_address
cma___get_area _CMA_PROTOTYPE_ ((
        cma_t_natural    size_in,
        cma_t_natural   *size_out));

static cma_t_boolean
cma___swipe_some_vm _CMA_PROTOTYPE_ ((void));

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__alloc_mem - takes an allocation request and returns memory
 *	the size of the next closest predefined unit.  Lookup lists are
 *	checked for equivalent size units, if free memory is not found
 *	the current pool is checked to see if it large enough, otherwise 
 *	a new pool is created to handle the request.
 *
 *  FORMAL PARAMETERS:
 * 
 *	size_in		The requested size (in bytes) to be allocated
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Address of the memory allocated (or NULL)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_address
cma__alloc_mem
#ifdef _CMA_PROTO_
	(
	cma_t_natural	 size)		/* Requested allocation, bytes */
#else	/* no prototypes */
	(size)
	cma_t_natural	 size;		/* Requested allocation, bytes */
#endif	/* prototype */
    {
    cma_t_integer	allocation;	/* Actual area allocated	*/
    cma_t_integer	alloc_type;	/* Type of allocation (s,m,l..) */
    cma_t_integer	return_size;	/* Size returned from get_bytes */
    cma_t_address	ret_addr;	/* Address of allocated memory  */
    cma__t_queue	*entry;
    cma_t_address	temp_ptr;	/* Used in managing pool	*/


    /* 
     *	Round up for CMA memory management (classification) purposes.
     */
    if (size <= cma___g_small_packet) {
	alloc_type = cma__c_small;
	allocation = cma___g_small_packet;
	}
    else if (size <= cma___g_med_packet) {
	alloc_type = cma__c_med;
	allocation = cma___g_med_packet;
	}
    else if (size <= cma___g_large_packet) {
	alloc_type = cma__c_large;
	allocation = cma___g_large_packet;
	}
    else {				/* request larger than lookasides */
	cma_t_natural	ts;

	/*
	 * Round up to next power of 2
	 */
	ts = size + cma__c_mem_tag;
	allocation = cma__g_memory[cma__c_large].size;
	while (allocation < ts) allocation <<= 1;
	allocation -= cma__c_mem_tag;	/* Request without tag */
	alloc_type = cma__c_pool;
	}
    
    cma__trace ((
	    cma__c_trc_vm,
	    "(alloc_mem) requesting %d bytes [%d] (%s)",
	    size,
	    allocation,
	    cma__g_vm_names[alloc_type]));

    /*
     * Check the appropriate lookup list to see if there is available memory
     * already there. If something is available we will get back a pointer
     * to useable memory. If the requested size is larger than the "large"
     * lookaside packets, it will come out of general pool instead.
     */

    if (alloc_type != cma__c_pool) {
	cma___mem_lock (cma__g_memory[alloc_type].mutex);

	if (!cma__queue_empty (&cma__g_memory[alloc_type].queue)) {
	    /*
	     * cma__queue_dequeue casts the dequeued item to type (type *)
	     * [where 'type' in this case is cma_t_address] when assigning to
	     * the output variable. On ANSI C systems, cma_t_address is
	     * "void*", and assigning a "void**" to a "void*" is fine. On VAX
	     * ULTRIX, cma_t_address is "char*", and the compiler doesn't
	     * appreciate assigning a "char**" to a "char*". So instead of
	     * returning the dequeued item directly to "ret_addr" where we
	     * want it, we use a cma_t_address* temporary, qtmp, and then
	     * cast that appropriately (VAX ULTRIX also doesn't support casts
	     * on the left hand side of an assignment, so
	     * "(cma_t_address*)ret_addr" won't work). Well, the compiler
	     * ought to optimize qtmp away, so it's not TOO ugly.
	     */
	    cma_t_address	*qtmp;


	    cma__queue_dequeue (
		    &cma__g_memory[alloc_type].queue,
		    qtmp,
		    cma_t_address);
	    ret_addr = (cma_t_address)qtmp;
	    cma__g_memory[alloc_type].count--;
	    cma___mem_unlock (cma__g_memory[alloc_type].mutex);
	    cma__trace ((
		    cma__c_trc_vm,
		    "(alloc_mem) 0x%lx, %d bytes from lookaside (%s)",
		    ret_addr,
		    allocation,
		    cma__g_vm_names[alloc_type]));
	    return ret_addr;
	    }
	else
	    /*
	     * Otherwise, we need to create a new one: count it now while we
	     * have the correct structure locked.
	     */
	    cma__g_memory[alloc_type].total++;

	cma___mem_unlock (cma__g_memory[alloc_type].mutex);
	}

    /*
     * If we get here, we can't pull the packet off a lookaside list: either
     * the correct list is empty, or we want something too big.
     */

    temp_ptr = cma___alloc_from_pool (allocation);

    while (temp_ptr == cma_c_null_ptr) {

	if (!cma___swipe_some_vm ()) {
	    cma__trace ((
		    cma__c_trc_vm,
		    "(alloc_mem) unable to swipe memory for %s (%d)",
		    cma__g_vm_names[alloc_type],
		    allocation));
	    return cma_c_null_ptr;
	    }

	temp_ptr = cma___alloc_from_pool (allocation);
	}

    cma__trace ((
	    cma__c_trc_vm,
	    "(alloc_mem) 0x%lx, %d bytes from pool (%s)",
	    temp_ptr,
	    allocation,
	    cma__g_vm_names[alloc_type]));
    return temp_ptr;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__alloc_zero - Allocate memory and set it to zero.
 *
 *  FORMAL PARAMETERS:
 * 
 *	size_in		The requested size (in bytes) to be allocated
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Address of the memory allocated (or NULL)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_address
cma__alloc_zero
#ifdef _CMA_PROTO_
	(
	cma_t_natural	 size)		/* Requested allocation, bytes */
#else	/* no prototypes */
	(size)
	cma_t_natural	 size;		/* Requested allocation, bytes */
#endif	/* prototype */
    {
    cma_t_address	allocation;
    long int		*memlong;
    char		*memchar;
    int			longs, bytes;

    allocation = cma__alloc_mem (size);

    if (allocation == cma_c_null_ptr)
	return allocation;

#ifndef NDEBUG
    /*
     * We need to interlock the statistics stuff, even though this routine
     * doesn't really need to take out any locks. So, for convenience, use
     * the general pool lock.
     */
    cma___mem_lock (cma__g_memory[cma__c_pool].mutex);
    cma__g_pool_stat.zero_allocs++;
    cma__g_pool_stat.zero_bytes += size;
    cma___mem_unlock (cma__g_memory[cma__c_pool].mutex);
#endif
    cma__trace ((
	    cma__c_trc_vm,
	    "(alloc_zero) clearing 0x%lx, %d bytes",
	    allocation,
	    size));
    memlong = (long int *)allocation;

    for (longs = size / sizeof (long int); longs > 0; longs--) {
	*memlong = 0;
	memlong++;
	}

    memchar = (char *)memlong;

    for (bytes = size % sizeof (long int); bytes > 0; bytes--) {
	*memchar = 0;
	memchar++;
	}

    return allocation;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__free_mem - takes in the address of a memory area
 *	allocated useing cma__alloc_mem and places it on the 
 *	appropriate lookup list.
 *
 *
 *  FORMAL PARAMETERS:
 * 
 *	mem area	address of area to be freed.
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__free_mem
#ifdef _CMA_PROTO_
	(
	cma_t_address	mem_area)	/* Address of memory to be freed */
#else	/* no prototypes */
	(mem_area)
	cma_t_address	mem_area;	/* Address of memory to be freed */
#endif	/* prototype */
    {
    cma_t_integer	allocation;	/* Size of area - for lookup list */
    cma_t_integer	alloc_type;	/* Type of area - for lookup list */

    allocation = *((cma_t_integer *)
	(((cma_t_integer)mem_area) - cma__c_mem_tag));

    /*
     * Get the allocation type.
     */
    if (allocation < cma___g_small_packet)
	alloc_type = cma__c_pool;
    else if (allocation < cma___g_med_packet)
	alloc_type = cma__c_small;
    else if (allocation < cma___g_large_packet)
	alloc_type = cma__c_med;
    else if (allocation == cma___g_large_packet)
	alloc_type = cma__c_large;
    else
	alloc_type = cma__c_pool;
	
    cma__trace ((
	    cma__c_trc_vm,
	    "(free_mem) 0x%lx, %d bytes (%s)",
	    mem_area,
	    allocation,
	    cma__g_vm_names[alloc_type]));
    cma___mem_lock (cma__g_memory[alloc_type].mutex);

    cma__queue_zero ((cma__t_queue *)mem_area);

    if (alloc_type == cma__c_pool) {
	cma___free_to_pool (mem_area);
	}
    else {
	cma__queue_insert_after (
		((cma__t_queue *)mem_area),
		&cma__g_memory[alloc_type].queue);
	cma__g_memory[alloc_type].count++;
	}

    cma___mem_unlock (cma__g_memory[alloc_type].mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Performs initialization for internal CMA memory management
 *	locks.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__init_mem_locks
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer	pool_initial;
    cma_t_integer	pool_size;


    cma__g_memory[cma__c_small].mutex = 
	cma__get_first_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_memory[cma__c_small].mutex, "VM, small");
    cma__g_memory[cma__c_med].mutex = 
	cma__get_first_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_memory[cma__c_med].mutex, "VM, medium");
    cma__g_memory[cma__c_large].mutex = 
	cma__get_first_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_memory[cma__c_large].mutex, "VM, large");
    cma__g_memory[cma__c_pool].mutex = 
	cma__get_first_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_memory[cma__c_pool].mutex, "VM, pool");
    cma___g_mem_init_done = cma_c_true;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Performs initialization work for internal CMA memory management 
 *	constants and queues.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__init_memory
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_natural	temp, alloc;

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
    struct {
        unsigned short  buffer_length;
        unsigned short  item_code;
        char            *buffer_address;
        short           *return_length_address;
        int             end;
        } itmlst = {
	    (sizeof (cma__g_page_size)),
	    SYI$_PAGE_SIZE, 
	    ((char *)&cma__g_page_size), 
	    0, 
	    0};
    double  iosb;

    sys$getsyiw ( 0, 0, 0, &itmlst, &iosb, 0, 0 );
#endif

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
    cma__g_page_size = cma__c_page_size;
#endif    

#if _CMA_OS_ == _CMA__UNIX
# if _CMA_VENDOR_ == _CMA__HP
    cma__g_page_size = NBPG;
# else
    cma__g_page_size = getpagesize(); 
# endif 
#endif 

    temp = cma__max (sizeof (cma__t_vp), sizeof (cma__t_int_alert))
	+ cma__c_mem_tag;
    alloc = 16;				/* Start with small power of 2 */
    while (alloc < temp) alloc <<= 1;
    cma__g_memory[cma__c_small].size = alloc;
    cma___g_small_packet = alloc - cma__c_mem_tag;
    cma__g_memory[cma__c_small].count = 0;
    cma__g_memory[cma__c_small].total = 0;
    cma__queue_init (&cma__g_memory[cma__c_small].queue);
    
    temp = cma__max (sizeof (cma__t_int_mutex),
	    cma__max (sizeof (cma__t_int_cv), sizeof (cma__t_int_stack)))
	+ cma__c_mem_tag;
    /*
     * Assume medium is at least as big as small, so no need to reinit
     * 'alloc'!
     */
    while (alloc < temp) alloc <<= 1;
    cma__g_memory[cma__c_med].size = alloc;
    cma___g_med_packet = alloc - cma__c_mem_tag;
    cma__g_memory[cma__c_med].count = 0;
    cma__g_memory[cma__c_med].total = 0;
    cma__queue_init (&cma__g_memory[cma__c_med].queue);

    temp = cma__max (sizeof (cma__t_int_tcb), sizeof (cma__t_int_attr))
	+ cma__c_mem_tag;
    /*
     * Assume large is at least as big as medium, so no need to reinit
     * 'alloc'!
     */
    while (alloc < temp) alloc <<= 1;
    cma__g_memory[cma__c_large].size = alloc;
    cma___g_large_packet = alloc - cma__c_mem_tag;
    cma__g_memory[cma__c_large].count = 0;
    cma__g_memory[cma__c_large].total = 0;
    cma__queue_init (&cma__g_memory[cma__c_large].queue);

    cma__g_memory[cma__c_pool].size = 0;
    cma__g_memory[cma__c_pool].count = 0;
    cma__g_memory[cma__c_pool].total = 0;
    cma__queue_init (&cma__g_memory[cma__c_pool].queue);
    }

#ifndef NDEBUG
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initializes an object header.
 *
 *  FORMAL PARAMETERS:
 *
 *	obj_ptr
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Returns the address passed to it.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_address
cma__init_object
#ifdef _CMA_PROTO_
	(
	cma_t_address	obj_ptr)
#else	/* no prototypes */
	(obj_ptr)
	cma_t_address	obj_ptr;
#endif	/* prototype */
    {
    if (obj_ptr != cma_c_null_ptr) {
	cma__queue_zero (&(((cma__t_object *)obj_ptr)->queue));
	}
    return obj_ptr;
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Performs pre- or post- `fork() reinitialization' work.
 *
 *  FORMAL PARAMETERS:
 *
 *	flag
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__reinit_memory
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
    {

    if (flag == cma__c_reinit_prefork_lock) {
	cma__int_lock(cma__g_memory[cma__c_small].mutex);
	cma__int_lock(cma__g_memory[cma__c_med].mutex);
	cma__int_lock(cma__g_memory[cma__c_large].mutex);
	cma__int_lock(cma__g_memory[cma__c_pool].mutex);
	}
    else if (flag == cma__c_reinit_postfork_unlock) {
	cma__int_unlock(cma__g_memory[cma__c_pool].mutex);
	cma__int_unlock(cma__g_memory[cma__c_large].mutex);
	cma__int_unlock(cma__g_memory[cma__c_med].mutex);
	cma__int_unlock(cma__g_memory[cma__c_small].mutex);
	}

    }

/*
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___alloc_from_pool
 *
 *	Try to find a large enough chunk in the pool to satisfy the
 *	request. If there are none, allocate some fresh VM.
 *
 *  FORMAL PARAMETERS:
 *
 *	size	requested length.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Returns pointer to newly allocated area, upon successful
 *	completion, or NULL if unsuccessful.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static cma_t_address
cma___alloc_from_pool
#ifdef _CMA_PROTO_
	(
	cma_t_natural	 size)
#else	/* no prototypes */
	(size)
	cma_t_natural	 size;
#endif	/* prototype */
    {
    cma__t_queue	*qp, *bp, *np;	/* Queue pointers */
    cma_t_natural	*ip, *nip;	/* Integer pointer */
    cma_t_natural	asz, rsz, bsz;


    cma__trace ((
	    cma__c_trc_vm,
	    "(alloc_from_pool) requesting %d",
	    size));
    cma___mem_lock (cma__g_memory[cma__c_pool].mutex);
    qp = cma__queue_previous (&cma__g_memory[cma__c_pool].queue);
    bp = (cma__t_queue *)cma_c_null_ptr;

    /*
     * Search the pool for a chunk that's big enough. We search from the end
     * of the list since newly allocated pool gets put at the end, and is
     * most likely to be large enough -- "end bits" should tend to get left
     * behind and ignored. (While this may cause more fragmentation than the
     * earlier "best fit" search, it's also faster if there are many pieces
     * left over; especially as it has fewer pagefaults!)
     */
    while (qp != &cma__g_memory[cma__c_pool].queue) {
	ip = (cma_t_natural *)qp;

	cma__trace ((
		cma__c_trc_vm,
		"(alloc_from_pool) item 0x%lx [%d]",
		qp,
		ip[-2]));

	if (ip[-2] >= size) {		/* If this one's big enough */
	    bsz = ip[-2];
	    bp = qp;
	    break;			/* Take the first fit! */
	    }

	qp = cma__queue_previous (qp);
	}

    if ((cma_t_address)bp != cma_c_null_ptr) {

	/*
	 * If we found one...
	 */
	np = cma__queue_next (bp);
	cma__queue_remove (bp, qp, cma__t_queue);

	cma__trace ((
		cma__c_trc_vm,
		"(alloc_from_pool) using 0x%lx",
		qp));


	/*
	 * If what we found is at least enough larger than what we need that
	 * we can split off another small-sized package from it, do so;
	 * otherwise just leave it in one piece.
	 */
	if (bsz >= (size + cma__g_memory[cma__c_small].size)) {
	    ip = (cma_t_natural *)qp;
	    nip = (cma_t_natural *)((cma_t_natural)qp + size + cma__c_mem_tag);
	    nip[-2] = bsz - cma__c_mem_tag - size;
	    ip[-2] = size;

	    cma__trace ((
		    cma__c_trc_vm,
		    "(alloc_from_pool) inserting split item 0x%lx [%d]",
		    nip,
		    nip[-2]));

	    cma__queue_zero ((cma__t_queue *)nip);
	    cma__queue_insert ((cma__t_queue *)nip, np);
	    cma__g_pool_stat.breakups++;
	    }
	else
	    cma__g_memory[cma__c_pool].count--;

	}
    else {
	/*
	 * There aren't any pieces in pool that fit. So allocate some fresh
	 * VM large enough to hold it. If it's bigger than the "standard pool
	 * size" just allocate enough to fit -- otherwise allocate the
	 * standard pool size and leave the remainder on the pool queue for
	 * later use.
	 */
	if (size > cma__c_pool_size)
	    asz = size + cma__c_mem_tag;
	else
	    asz = cma__c_pool_size;

	cma___mem_unlock (cma__g_memory[cma__c_pool].mutex);
	nip = (cma_t_natural *)cma___get_area (asz, &rsz);
	cma___mem_lock (cma__g_memory[cma__c_pool].mutex);
	cma__g_pool_stat.allocs++;
	cma__g_pool_stat.bytes_allocd += rsz;

	if ((cma_t_address)nip == cma_c_null_ptr) {
	    /*
	     * Uh oh! We're out of memory. Let higher level code scrounge
	     */
	    cma__g_pool_stat.failures++;
	    cma___mem_unlock (cma__g_memory[cma__c_pool].mutex);
	    cma__trace ((
		    cma__c_trc_vm,
		    "(alloc_from_pool) unable to allocate %ld bytes",
		    asz));
	    return cma_c_null_ptr;
	    }

	if (rsz == size + cma__c_mem_tag) {
	    nip[0] = rsz - cma__c_mem_tag;
	    qp = (cma__t_queue *)&nip[2];
	    cma__g_pool_stat.exact_allocs++;
	    }
	else {
	    nip[0] = rsz - size - (cma__c_mem_tag * 2);
	    np = (cma__t_queue *)&nip[2];
	    ip = (cma_t_natural *)((cma_t_natural)np + nip[0]);
	    qp = (cma__t_queue *)&ip[2];
	    ip[0] = size;
	    cma___free_to_pool ((cma_t_address)np);
	    }

	}

    cma__g_pool_stat.extractions++;
    cma___mem_unlock (cma__g_memory[cma__c_pool].mutex);
    cma__trace ((
	    cma__c_trc_vm,
	    "(alloc_from_pool) acquired 0x%lx",
	    qp));
    return (cma_t_address)qp;
    }

/*
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___free_to_pool -
 *		Fit a packet into the pool list: merge with adjacent packets if
 *		possible.
 *
 *  FORMAL PARAMETERS:
 * 
 *	pack	Address of packet.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static void
cma___free_to_pool
#ifdef _CMA_PROTO_
	(cma_t_address	pack)
#else
	(pack)
	cma_t_address	pack;
#endif
    {
    cma_t_natural	*ip, *bi, *ni, size;
    cma__t_queue	*qp, *qb, *qn, *qh, *qt;
    cma_t_boolean	found = cma_c_false, insert = cma_c_true;
    cma_t_boolean	merged = cma_c_false;


    cma__g_pool_stat.returns++;
    ip = (cma_t_natural *)((cma_t_natural)pack - cma__c_mem_tag);
    qp = (cma__t_queue *)pack;
    size = ip[0];

    cma__trace ((
	    cma__c_trc_vm,
	    "(free_to_pool) item 0x%lx [%d]",
	    qp,
	    size));

    qh = &cma__g_memory[cma__c_pool].queue;

    qb = cma__queue_previous (qh);	/* Point to last entry */

    while (qb != qh) {

	if (qp > qb) {
	    found = cma_c_true;
	    break;
	    }

	qb = cma__queue_previous (qb);
	}

    /*
     * As long as we found a packet with a lower address, check whether we
     * can just tack this onto the end of it.
     */
    if (found) {
	bi = (cma_t_natural *)((cma_t_natural)qb - cma__c_mem_tag);

	if ((cma_t_natural *)((cma_t_natural)qb + bi[0]) == ip) {
	    bi[0] = bi[0] + cma__c_mem_tag + size;
	    merged = cma_c_true;
	    cma__g_pool_stat.merge_befores++;

	    /*
	     * Use the new combined packet to see if we can merge with the
	     * next packet.
	     */
	    ip = bi;
	    qp = qb;
	    size = ip[0];
	    insert = cma_c_false;
	    }

	}

    qn = cma__queue_next (qb);

    if (qn != qh) {
	/*
	 * If there's a following packet, see if we can merge with it
	 */
	ni = (cma_t_natural *)((cma_t_natural)qn - cma__c_mem_tag);

	if ((cma_t_natural *)((cma_t_natural)qp + size) == ni) {
	    ip[0] = ip[0] + ni[0] + cma__c_mem_tag;
	    qt = cma__queue_next (qn);
	    cma__queue_remove (qn, qn, cma__t_queue);

	    /*
	     * If we already merged with the previous packet, then this merge
	     * decreases the packet count by one.
	     */
	    if (merged)
		cma__g_memory[cma__c_pool].count--;

	    cma__g_pool_stat.merge_afters++;
	    merged = cma_c_true;
	    qn = qt;
	    }

	}

    /*
     * If the packet needs to be inserted, do so (before qn -- which is
     * either the queue header or the packet with the next highest address).
     */
    if (insert) {
	cma__queue_zero ((cma__t_queue *)qp);
	cma__queue_insert (qp, qn);

	if (!merged)
	    cma__g_memory[cma__c_pool].count++;

	return;
	}

    }

/*
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___get_area -
 *	    Unsynchronized jacket for basic system-specific memory allocation 
 *	    service.  This is intended to be used internally for allocation 
 *	    of large areas of memory to be managed by CMA.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *	    size_in - requested length.
 *	    size_out - (by reference) actual acquired length.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Returns pointer to newly allocated area, upon successful
 *	completion, or NULL if unsuccessful.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static cma_t_address
cma___get_area
#ifdef _CMA_PROTO_
	(
	cma_t_natural	 size_in,
	cma_t_natural	*size_out)
#else	/* no prototypes */
	(size_in, size_out)
	cma_t_natural	 size_in;
	cma_t_natural	*size_out;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    cma_t_natural	pagcnt;
    cma_t_natural	retadr[2];
    cma_t_natural	status;


    pagcnt = (size_in + cma__c_page_size - 1) / cma__c_page_size;
    status = sys$expreg (pagcnt, retadr, 0, 0);

    cma__trace ((
	    cma__c_trc_vm,
	    "(get_area) 0x%lx, %d pages [%d bytes] (status = 0x%lx)",
	    retadr[0],
	    pagcnt,
	    retadr[1] - retadr[0] + 1,
	    status));

    if (status != SS$_NORMAL) {
	*size_out = 0;
	return cma_c_null_ptr;
	}

    *size_out = retadr[1] - retadr[0] + 1;
    return ((cma_t_address)retadr[0]);
#endif
#if _CMA_OS_ == _CMA__UNIX
    cma_t_integer	incr, round_bit;
    cma_t_integer	ret_addr;


    incr = cma___roundup_page_size ((cma_t_integer)size_in + 8);

    /*
     * We need to allocate a chunk of memory at least as large as the caller
     * requested. We also need to make sure that it's aligned on at least an
     * 8-byte boundary (that's how we've defined cma__alloc_mem). Since
     * someone else could do sbrk() calls for random sizes, the address we
     * get may need to be adjusted... in that case, we do another sbrk() for
     * the missing bytes. That gives this bunch the proper size, and aligns
     * the break for the next allocation.
     */
    if (cma__g_global_lock) cma__int_lock (cma__g_global_lock);
    ret_addr = (cma_t_integer)(sbrk (incr));
    cma__trace ((
	    cma__c_trc_vm,
	    "(get_area) %d bytes (return = 0x%lx)",
	    incr,
	    ret_addr));

    if (ret_addr == -1) {
	*size_out = 0;
	if (cma__g_global_lock) cma__int_unlock (cma__g_global_lock);
	return cma_c_null_ptr;
	}

    if (((long)ret_addr & 7) != 0) {
	round_bit = 8 - ((long int)ret_addr % 8);
	ret_addr += round_bit;
	(void)sbrk (round_bit);
	cma__g_sbrk_align++;
	}

    if (cma__g_global_lock) cma__int_unlock (cma__g_global_lock);
    *size_out = (cma_t_natural)incr;
    return ((cma_t_address)ret_addr);
#endif
    }

/*
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___swipe_some_vm -
 *		Take one element from each lookaside list and free it to the
 *		general pool.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	cma_c_true if any memory was moved; cma_c_false if lookasides were
 *	already empty.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static cma_t_boolean
cma___swipe_some_vm
#ifdef _CMA_PROTO_
	(void)
#else
	()
#endif
    {
    cma__t_queue	*tmp[cma__c_large + 1];
    cma_t_boolean	found = cma_c_false;
    cma_t_integer	i;


    cma__trace ((
	    cma__c_trc_vm,
	    "(swipe_some_vm) cleaning up..."));

    for (i = 0; i <= cma__c_large; i++) {
	cma___mem_lock (cma__g_memory[i].mutex);

	if (!cma__queue_empty (&cma__g_memory[i].queue)) {
	    found = cma_c_true;
	    cma__queue_dequeue (&cma__g_memory[i].queue, tmp[i], cma__t_queue);
	    cma__g_pool_stat.scrounges[i]++;
	    cma__g_memory[i].count--;
	    cma__g_memory[i].total--;
	    }
	else
	    tmp[i] = (cma__t_queue *)cma_c_null_ptr;

	cma___mem_unlock (cma__g_memory[i].mutex);
	}

    if (!found)
	return cma_c_false;

    cma___mem_lock (cma__g_memory[cma__c_pool].mutex);

    for (i = 0; i <= cma__c_large; i++) {

	if ((cma_t_address)tmp[i] != cma_c_null_ptr)
	    cma___free_to_pool ((cma_t_address)tmp[i]);

	}

    cma___mem_unlock (cma__g_memory[cma__c_pool].mutex);
    return cma_c_true;
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_VM.C */
/*  *38    2-JUL-1993 14:38:14 BUTENHOF "Add a queue_zero" */
/*  *37    3-JUN-1993 14:15:35 BUTENHOF "Fix init_object" */
/*  *36   27-MAY-1993 14:32:54 BUTENHOF "Reduce fragmentation" */
/*  *35    6-MAY-1993 19:07:19 BUTENHOF "Add to NDEBUG VM stats" */
/*  *34   17-APR-1993 11:16:17 BUTENHOF "Fix for vax" */
/*  *33   17-APR-1993 09:01:16 BUTENHOF "Fix VAX compile warning" */
/*  *32   16-APR-1993 13:06:47 BUTENHOF "Update queue operations to allow INSQUE/REMQUE" */
/*  *31   10-SEP-1992 14:26:24 BUTENHOF "Back off pool lock before sbrk()" */
/*  *30    4-SEP-1992 16:30:55 CURTIN "Add IOSB to getsyiw call" */
/*  *29   26-JUN-1992 10:53:13 CURTIN "Fix a hard coded constant for EVMS" */
/*  *28   15-MAY-1992 15:05:06 SCALES "Add queue consistency checks" */
/*  *27   19-FEB-1992 07:16:33 BUTENHOF "Fix bug in UNIX code (sigh)" */
/*  *26   19-FEB-1992 04:34:57 BUTENHOF "Fix typos" */
/*  *25   18-FEB-1992 16:30:47 BUTENHOF "Fix stuff" */
/*  *24   18-FEB-1992 16:01:50 BUTENHOF "Fix a typo" */
/*  *23   18-FEB-1992 15:31:35 BUTENHOF "Add cma__alloc_zero (pre-cleared memory)" */
/*  *22   20-DEC-1991 11:31:34 CURTIN "removed starlet.h on VAX/VMS" */
/*  *21    2-DEC-1991 16:55:23 CURTIN "Added a call to getsyi on Alpha to get system page size" */
/*  *20   31-OCT-1991 12:40:17 BUTENHOF "Fix handling of exceptions" */
/*  *19   14-OCT-1991 14:44:36 BUTENHOF "Fix best_fit routine" */
/*  *18   14-OCT-1991 13:42:32 BUTENHOF "Add support for large allocations" */
/*  *17   24-SEP-1991 16:30:06 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *16   10-JUN-1991 18:25:13 SCALES "Add sccs headers for Ultrix" */
/*  *15    5-JUN-1991 16:16:22 CURTIN "fork work" */
/*  *14   29-MAY-1991 13:18:21 CURTIN "Adjust init pool size allocation" */
/*  *13   24-MAY-1991 16:52:09 CURTIN "Added a new reinit routine" */
/*  *12    9-MAY-1991 14:55:45 CURTIN "Added a RERAISE to a CATCH_ALL clause" */
/*  *11    2-MAY-1991 14:00:01 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *10   18-APR-1991 16:42:46 CURTIN "changed position of roundup in __get_area_nolock" */
/*  *9    12-APR-1991 23:37:35 BUTENHOF "Fix bug in cma__get_area()" */
/*  *8     1-APR-1991 18:10:00 BUTENHOF "QAR 93, exception text" */
/*  *7    14-MAR-1991 13:46:08 SCALES "Convert to stream format for ULTRIX build" */
/*  *6    12-MAR-1991 19:40:51 SCALES "Merge in HP change to CD4" */
/*  *5    20-FEB-1991 14:26:43 CURTIN "Now round to quadword, that returned by sbrk" */
/*  *4    24-JAN-1991 00:35:25 BUTENHOF "Fix exception name references" */
/*  *3     8-JAN-1991 14:51:42 CURTIN "fixed internal cma qar 063" */
/*  *2    17-DEC-1990 09:32:24 CURTIN "Added statistics taking for memory" */
/*  *1    12-DEC-1990 21:55:56 BUTENHOF "VM management" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_VM.C */
