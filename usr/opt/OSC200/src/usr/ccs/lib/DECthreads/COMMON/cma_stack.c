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
static char *rcsid = "@(#)$RCSfile: cma_stack.c,v $ $Revision: 4.2.11.4 $ (DEC) $Date: 1993/08/18 14:51:51 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads Core Services
 *
 *  ABSTRACT:
 *
 *	Stack management
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	21 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	25 August 1989
 *		Implement cma_call_on_stack, provide header skeletons for
 *		other public functions.
 *	002	Dave Butenhof	26 August 1989
 *		Implement all public functions.
 *	003	Dave Butenhof	15 September 1989
 *		Fix get_self_tcb to return default TCB if stack isn't in list
 *		(it must be the default user stack, which is the default
 *		thread).
 *	004	Dave Butenhof	29 September 1989
 *		Queue stacks onto owner TCB so they can be cleaned up.  Also,
 *		remove the debugging function calls (conditionalize under
 *		STACK_TRACE).  Also change cma___assign_chunks to external
 *		cma__assign_stack, and simplify argument list.
 *	005	Dave Butenhof	9 October 1989
 *		Make use of cma__error to raise exceptions where necessary.
 *	006	Dave Butenhof	12 October 1989
 *		Use internal mutex operations on tcb->mutex.
 *	007	Dave Butenhof	19 October 1989
 *		Use new type-specific handle validation macros.
 *	008	Dave Butenhof	19 October 1989
 *		Substitute "cma_t_address" for explicit "void *" to make
 *		porting easier.
 *	009	Dave Butenhof	19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *	010	Webb Scales	19 October 1989
 *		Moved comment outside of cma___compute_chunk_index macro, 
 *		and deleted function prototype for it;  created cma__fetch_SP
 *		macro to move the #pragmas out of this file
 *	011	Dave Butenhof	20 October 1989
 *		Lowercase cma__fetch_SP to cma__fetch_sp to fit APA naming
 *		conventions.
 *	012	Webb Scales	20 October 1989
 *		Changed "cma_assem_vms.h" to "cma_assem.h"
 *	013	Dave Butenhof	Halloween 1989
 *		Fix error in delete_stack; unlock stack DB before returning.
 *	014	Dave Butenhof	1 November 1989
 *		Change locking of stack info.
 *	015	Bob Conti	4 November 1989
 *		Removed superfluous include
 *	016	Dave Butenhof	27 November 1989
 *		Implement cma_allocate_stack_np.
 *	017	Dave Butenhof	1 December 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	018	Dave Butenhof	8 January 1990
 *		Include cma_vm.h so allocation calls will be right.
 *	019	Dave Butenhof	24 January 1990
 *		Remove internal mutex for stack control: use kernel critical.
 *	020	Dave Butenhof	26 January 1990
 *		Convert stack cluster chain from queue to list, so we can
 *		traverse without locking.  This should increase efficiency in
 *		cma__get_self_tcb and also resolve fix BL1 locking protocol
 *		error.
 *	021	Dave Butenhof	9 February 1990
 *		Add conditional code to handle upward-growing stacks as well
 *		as downward-growing stacks.
 *	022	Dave Butenhof	10 April 1990
 *		Conditionalize the function cma__get_self_tcb; if
 *		_CMA_UNIPROCESSOR_ is defined, this is a macro which will
 *		return the dispatcher's current thread pointer.
 *	023	Webb Scales	16 April 1990
 *		Added a test to get_self_tcb which raises an exception if the
 *		caller is not on a CMA-allocated stack.
 *	024	Webb Scales	30 April 1990
 *		Made initialization of cma__g_current_thread static.
 *	025	Dave Butenhof	18 June 1990
 *		Use macros to clear object name (only defined for debug
 *		build).
 *	026	Paul Curtin	27 June	1990
 *		Reworked stacks; now calculate address. Use chunk boundaries
 *		Add to check_stack_limit, with Bob Conti's help.
 *	027	Paul Curtin	 2 July 1990
 *		Fixed get TCB code, and cleaned up a little
 *	028	Paul Curtin	17 July 1990
 *		Removed an elif.
 *	029	Paul Curtin	24 July 1990
 *		Added round up to stack size equations; using new macro
 *	030	Paul Curtin	 7 August 1990
 *		Replace printf w/ cma__put* functions.
 *		Replace cma__alloc_mem w/ cma__get_area
 *	031	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	032	Paul Curtin	10 September 1990
 *		Added explicit  guardsize roundup to cma__get_stack.
 *	033	Paul Curtin	11 September 1990
 *		Fixed check stack limit.  Go off of sp if necessary.
 *	034	Paul Curtin	11 September 1990
 *		Check stack limit nolonger does DEADBEEF, now read/write.
 *	035	Dave Butenhof	29 November 1990
 *		Implement new versions of 'get-self-tcb' and 'stack-info' to
 *		allow passing in an SP from outside. This supports the Ada
 *		RTL's need for getting per-thread context while operating off
 *		the assigned thread stack (for stack overflow handling).
 *		Eventually, Ada's overflow handling will be incorporated into
 *		CMA.
 *	036	Webb Scales	 6 December 1990
 *		Put in an HP-specific definition for chunk_size.
 *	037	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	038	Dave Butenhof	04 April 1991
 *		Change _CMA_UNIPROCESSOR_ to 0/1 instead of ifdef/ifndef
 *	039	Dave Butenhof	30 April 1991
 *		Add cma__trace calls to provide debug information.
 *	040	Paul Curtin	24 May 1991
 *		Added cma__reinit_stack routine.
 *	041	Paul Curtin	 5 June 1991
 *		Rearranged flags in reinit routine.
 *	042	Paul Curtin	10 June 1991
 *		Added large stack support (> default cluster)
 *		Create cma___stack_last_yellow, added cma___create
 *		_special_cluster, Added cma___delete_special_cluster.
 *	043	Dave Butenhof	19 September 1991
 *		Integrate HPUX CMA5 reverse drop: change size of HP stack
 *		chunk.
 *	044	Dave Butenhof	02 October 1991
 *		Integrate changes provided by Alan Peckham to unprotect guard
 *		pages on all stacks before aborting the process; this allows
 *		the UNIX core dump code (which makes the absurd assumption
 *		that the known universe stops just because a page is
 *		protected) to work on threaded processes.
 *	045	Dave Butenhof	07 November 1991
 *		Fix a bug in large stack creation (cma__get_stack): it fell
 *		through into common code that used "index" variable, which
 *		hadn't been initialized.
 *	046	Dave Butenhof	11 November 1991
 *		Fix compilation warnings in MIPS C.
 *	047	Dave Butenhof	22 November 1991
 *		Add typecast to cma__fetch_sp to simplify cma_host.h and
 *		remove its dependency on cma.h and cma_defs.h types.
 *	048	Paul Curtin	02 December 1991
 *		Removed an erroneous '$'
 *	049	Dave Butenhof	21 January 1992
 *		Fix bug in destroying special stack cluster; it was being
 *		deallocated to pool but left on the cluster list. Since items
 *		can't be removed from the list (it's traversed without
 *		synchronization by cma__get_self_tcb() in non-uniprocessor
 *		variants), leave the first "normal cluster" chunk on the list
 *		and deallocate the rest.
 *	050	Dave Butenhof	28 January 1992
 *		Resolve some remaining issues in "special cluster"
 *		management. First, improve memory usage a little by making
 *		them less like clusters; they really don't need the large
 *		thread map. Second, instead of arbitrarily breaking them up
 *		on deletion, leave them on the queue (but marked "free") so
 *		they can be reused. When a new stack is created and no normal
 *		clusters are available, create_cluster knows how to find a
 *		"big stack" cluster on the cluster list and break it up at
 *		that time.
 *	051	Dave Butenhof	10 February 1992
 *		Move cma__get_self_tcb() to the end of the module; it #undefs
 *		the quick-access uniprocessor cma__get_self_tcb macro, and
 *		(although there seems to be no bad effects now), that could
 *		potentially impact code added later to this module.
 *	052	Dave Butenhof	11 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	053	Dave Butenhof	06 March 1992
 *		Currently, due to the way "picie" (ld's processor for making
 *		RISC code position-independent) modifies the DECthreads
 *		assembly code, we are unable to make cma__do_call_on_stack
 *		function on MIPS DEC OSF/1. Because the failure modes can be
 *		obscure, avoid it by disabling the cma_stack_call_routine_np
 *		function.
 *	054	Webb Scales	13 May 1992
 *		(Conditionally) initialized hole queue links in stack header.
 *	055	Dave Butenhof	13 August 1992
 *		Don't compile normal get_self_tcb on Alpha OSF/1 -- use rd/wr
 *		unique PAL instead.
 *	056	Dave Butenhof	15 September 1992
 *		Change sequence number maintenance -- cheaper to combine it
 *		in the known stack list rather than have a separate mutex.
 *	057	Dave Butenhof	18 January 1993
 *		Fix calculation of "first cluster" pointer in bigstack
 *		creation: it adds size of "cma___t_clu_desc", where it should
 *		add size of "cma___t_bigstack". This shouldn't be critical,
 *		though, since the roundup to chunk size boundary should be
 *		enough to fit the rest of the header.
 *	058	Dave Butenhof	19 January 1993
 *		Fix check for available bigstack in get_stack -- don't reuse
 *		existing bigstack unless it's free. (Nice general rule to
 *		remember :-) ).
 *	059	Dave Butenhof	25 January 1993
 *		Currently, get_stack rounds up the requested stack to a
 *		protection boundary, then adds the reserve stack size also
 *		rounded up -- this results in allocating space the user
 *		claims they don't need and we claim WE don't need,
 *		pointlessly increasing the minimal stack size. Now I'm going
 *		to add them before doing the roundup.
 *	060	Dave Butenhof	 4 February 1993
 *		Arrgh! Bug in 059 -- I added reserve to attr->stack_size at
 *		each stack creation.
 *	061	Dave Butenhof	 8 February 1993
 *		Another 059 bug -- 'last_guard' is calculated with
 *		attr->guard_size, which is no longer rounded up.
 *	062	Webb Scales	19 March 1993
 *		Split check-stack-limit into two routines, to minimize impact
 *		of its stack usage on its result.
 *	063	Webb Scales	23 March 1993
 *		Fix initial boundary condition in check-stack-limit.
 *	064	Dave Butenhof	29 March 1993
 *		Fix typecast problem on 062.
 *	065	Dave Butenhof	30 March 1993
 *		Replace "%08x" formatting with "0x%lx" for 64-bit safety.
 *	066	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	067	Dave Butenhof	 5 May 1993
 *		Minor change to 'stack' command -- include size of stack
 *		cluster (even though it's fixed).
 *	068	Dave Butenhof	10 May 1993
 *		Show default stack as well as dynamic stacks (mostly so the
 *		output won't be null with only one thread).
 *	069	Dave Butenhof	25 May 1993
 *		Fix bug in cma__get_stack() for guardsize == 0.
 *	070	Brian Keane	2 June 1993
 *		Remove unused (and incorrect) proto for getpagesize().
 *	071	Brian Keane	15 June 1993
 *		Fix minor compile problems with DEC C on DEC OSF/1 AXP.
 */	


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_handle.h>
#include <cma_stack.h>
#include <cma_attr.h>
#include <cma_tcb.h>
#include <cma_queue.h>
#include <cma_list.h>
#include <cma_mutex.h>
#include <cma_errors.h>
#include <cma_assem.h>
#include <cma_dispatch.h>
#include <cma_vm.h>
#if _CMA_PROTECT_MEMORY_
# include <cma_vmprot.h>
#endif

/*
 * GLOBAL DATA
 */
cma__t_list	cma__g_stack_clusters;
cma__t_int_tcb	*cma__g_current_thread = &cma__g_def_tcb;
cma_t_integer	cma__g_chunk_size;

/*
 * LOCAL DATA
 */

static cma__t_int_mutex	*cma___g_stack_mutex;
static cma_t_natural	cma___g_def_cluster_size;
static cma_t_integer	cma___g_stack_seq;
#if _CMA_PROTECT_MEMORY_
static cma__t_queue	cma___g_holes;
#endif

/*
 * LOCAL MACROS
 */

/*
 * Convert an address (presumed to be within the specified stack cluster!)
 * into a chunk index.
 *
 * NOTE: If the computed chunk_index is greater than the specified
 * cma__c_chunk_count than we probably have a special, X-tra large
 * cluster here.  Inorder to prevent pointing off the end of the 
 * map[cma__g_chunk_count] entry, which isn't meant to handle special
 * stacks/clusters (it's not needed, there's 1 special stack per special
 * cluster) we will return the greatest valid index.
 *
 * 	cma___t_cluster	*cluster,
 *	cma_t_address	address
 */
#define cma___compute_chunk_index(cluster,address) \
    (((((cma_t_integer)address - (cma_t_integer)((cluster)->desc.stacks)) \
	    / cma__g_chunk_size) >= cma__c_chunk_count) ? \
	(cma__c_chunk_count - 1) : \
	(((cma_t_integer)address - (cma_t_integer)((cluster)->desc.stacks)) \
	    / cma__g_chunk_size))

/*
 * Given cluster address, and known cluster header size, calculate the 
 * address of first chunk in cluster.
 *
 *	cma___t_cluster	*cluster
 *	cma_t_int	header_size
 *
 */
#define cma___compute_first_chunk(cluster,header_size) \
    (((cma_t_integer)(cluster) + (header_size) \
	+ (cma__g_chunk_size)) & ~(cma__g_chunk_size - 1))

/*
 * Find which cluster and chunk index contains the specified SP value.
 *
 *	cma_t_address	sp		SP location to find
 *	cma___t_cluster	**cluster	Address to return cluster
 *	cma___t_index	*index		Address to return chunk index
 *
 */
#define cma___find_sp_cluster(sp,cluster,index) { \
    cma__t_list		*__fsc_clust_q__; \
    cma___t_clu_desc	*__fsc_clust__; \
    cma_t_boolean	__fsc_break__ = cma_c_false; \
    __fsc_clust_q__ = cma__list_next (&cma__g_stack_clusters); \
    while (__fsc_clust_q__ != cma__c_null_list) { \
	__fsc_clust__ = (cma___t_clu_desc *)__fsc_clust_q__; \
        if (((sp) >= __fsc_clust__->stacks) \
                && ((sp) <= __fsc_clust__->limit)) { \
	    if (__fsc_clust__->type == cma___c_bigstack) \
		*(index) = 0; \
	    else \
		*(index) = cma___compute_chunk_index ( \
			(cma___t_cluster *)__fsc_clust__, (sp)); \
	    *(cluster) = (cma___t_cluster *)__fsc_clust__; \
	    __fsc_break__ = cma_c_true; \
	    break; \
	    } \
	__fsc_clust_q__ = cma__list_next (&__fsc_clust__->list); \
	} \
    if (! __fsc_break__) { \
	*(cluster) = cma___c_null_cluster; \
	*(index) = 0; \
	} \
    }

/*
 * Get information on the current stack (stack pointer, which cluster it's
 * in, and what the chunk index is).  If running on the default stack or a
 * non-CMA stack, the cluster is cma_c_null_ptr and the index is 0.
 *
 *	cma_t_address	*sp		Address to return SP
 *	cma___t_cluster	**cluster	Address to return cluster
 *	cma___t_index	*index		Address to return chunk index
 *
 */
#define cma___get_stack_info(sp,cluster,index) { \
    cma_t_address __gsi_stkptr__; \
    __gsi_stkptr__ = (cma_t_address)cma__fetch_sp (); \
    *(sp) = __gsi_stkptr__; \
    cma___find_sp_cluster (__gsi_stkptr__, (cluster), (index)); \
    }

/*
 * LOCAL FUNCTIONS
 */
static void
cma___assign_stack_nolock _CMA_PROTOTYPE_ ((	/* Assign chunks to thread */
	cma__t_int_stack	*stack,		/*  stack to assign them to */
	cma__t_int_tcb		*tcb));		/* tcb to assign them to */

#if _CMA_STACK_TRACE_
static void 
cma___dump_cluster _CMA_PROTOTYPE_ ((
	char		*type,
	int		chunks,
	cma___t_cluster	*cluster));
#endif

static void
cma___free_chunks _CMA_PROTOTYPE_ ((	/* Deallocate chunks */
	cma___t_cluster	*cluster,	/* stack cluster */
	cma___t_index	low_chunk,	/* low limit */
	cma___t_index	high_chunk));	/* high limit */

static void
cma___free_stack_nolock _CMA_PROTOTYPE_ ((
	cma__t_int_stack	*desc));	/* Stack to free */

static cma_t_boolean
cma___int_check_limit _CMA_PROTOTYPE_ ((
	cma_t_integer	size,
	cma_t_address	sp,
	cma___t_cluster	*cluster,
	cma___t_index	index));

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine returns a new SP for the caller to use in allocating a
 *	specified amount of new stack space.
 *
 *  FORMAL PARAMETERS:
 *
 *	size		The number of bytes to allocate.
 *
 *	new_stack	The address to which SP should be set to allocate.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
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
 *	Raise an exception if the requested stack space would exceed the
 *	stack's size limit.
 */
extern void
cma_stack_allocate_np
#ifdef _CMA_PROTO_
	(
	cma_t_integer	size,
	cma_t_address	*new_stack)
#else	/* no prototypes */
	(size, new_stack)
	cma_t_integer	size;
	cma_t_address	*new_stack;
#endif	/* prototype */
    {
    cma_t_address	sp;
    cma___t_cluster	*cluster;
    cma___t_index	index;
    cma__t_int_stack	*stack;


    cma___get_stack_info (&sp, &cluster, &index);
#ifdef _CMA_UPSTACK_
    sp = (cma_t_address)((char *)sp + size);
#else
    sp = (cma_t_address)((char *)sp - size);
#endif

    if (cluster == cma___c_null_cluster)
	*new_stack = sp;
    else
	{

	if (cluster->desc.type == cma___c_cluster)
	    stack = cluster->map[index].mapped.stack;
	else
	    stack = ((cma___t_bigstack *)cluster)->stack;

#ifdef _CMA_UPSTACK_
	if (sp > stack->yellow_zone)
#else
	if (sp < stack->yellow_zone)
#endif
	    cma__error (cma_s_stackovf);
	else
	    *new_stack = sp;

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine assigns a stack to thread
 *
 *  FORMAL PARAMETERS:
 *
 *	stack   -  The address of the handle of the stack to be assigned
 *
 *	thread  -  The address of the handle of the thread to be assigned to
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Modify the appropriate thread map entries for the stack.
 */
extern void
cma_stack_assign_np
#ifdef _CMA_PROTO_
	(
	cma_t_stack_np	*stack,
	cma_t_thread	*thread)
#else	/* no prototypes */
	(stack, thread)
	cma_t_stack_np	*stack;
	cma_t_thread	*thread;
#endif	/* prototype */
    {
    cma__t_int_stack	*istack;	/* Address of internal stack object */
    cma__t_int_tcb	*tcb;


    cma__int_lock (cma___g_stack_mutex);
    istack = cma__validate_stack (stack);
    tcb = cma__validate_tcb (thread);
    cma___assign_stack_nolock (istack, tcb);
    cma__int_unlock (cma___g_stack_mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine calls the specified routine on the specified stack passing
 *	the specified parameter.  The stack is assigned to the thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	stack	The address of the handle of the stack to call on
 * 
 *	routine	The address of the routine to call
 * 
 *	arg	The parameter to pass to the routine
 *
 *	result	Return the result value of the routine
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
 *	None
 */
extern void
cma_stack_call_routine_np
#ifdef _CMA_PROTO_
	(
	cma_t_stack_np		*stack,
	cma_t_call_routine	routine,
	cma_t_address		arg,
	cma_t_address		*result)
#else	/* no prototypes */
	(stack, routine, arg, result)
	cma_t_stack_np		*stack;
	cma_t_call_routine	routine;
	cma_t_address		arg;
	cma_t_address		*result;
#endif	/* prototype */
    {
#ifdef _CMA_SHLIB_
    cma__error (cma_s_unimp);
#else
    cma__t_int_stack	*istack;    /* Address of internal stack object */
    cma__t_int_tcb	*tcb = cma__get_self_tcb ();


    cma__int_lock (cma___g_stack_mutex);
    istack = cma__validate_stack (stack);
    cma___assign_stack_nolock (istack, tcb);
    cma__int_unlock (cma___g_stack_mutex);

    *result = cma__do_call_on_stack (istack->stack_base, routine, arg);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine performs a limit check on the current stack
 *
 *  FORMAL PARAMETERS:
 *
 *	size  -  The number of bytes by which the stack will be extended
 *
 *  IMPLICIT INPUTS:
 *
 *	The address space  ;-)
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Returns true (1) if the requested stack extension is valid
 *	Returns false (0) if the requested stack extension is invalid
 *
 *  SIDE EFFECTS:
 *
 *	None (currently)
 */
extern cma_t_boolean
cma_stack_check_limit_np
#ifdef _CMA_PROTO_
	(
	cma_t_integer	size)
#else	/* no prototypes */
	(size)
	cma_t_integer	size;
#endif	/* prototype */
    {
    cma_t_address	sp;
    cma___t_cluster	*cluster;
    cma___t_index	index;


    cma___get_stack_info (&sp, &cluster, &index);

    return cma___int_check_limit (size, sp, cluster, index);
    }    

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine creates a new stack and returns the handle of the new
 *	stack.
 *
 *  FORMAL PARAMETERS:
 *
 *	stack  -  The address of the handle to receive the address of the stack
 *
 *	att    -  The address of the handle of the attributes for the stack
 *
 *  IMPLICIT INPUTS:
 *
 *	None (currently)
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None (currently)
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_stack_create_np
#ifdef _CMA_PROTO_
	(
	cma_t_stack_np	*stack,
	cma_t_attr	*att)
#else	/* no prototypes */
	(stack, att)
	cma_t_stack_np	*stack;
	cma_t_attr	*att;
#endif	/* prototype */
    {
    cma__t_int_stack	*istack;
    cma__t_int_attr	*iatt;
    cma__t_int_tcb	*tcb;


    iatt = cma__validate_default_attr (att);
    istack = cma__get_stack (iatt);

    if ((cma_t_address)istack == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	tcb = cma__get_self_tcb ();
	cma__assign_stack (istack, tcb);	/* Assign the chunks */
	cma__object_to_handle ((cma__t_object *)istack, stack);
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine deletes a stack 
 *
 *  FORMAL PARAMETERS:
 *
 *	stack  -  The address of the handle of the stack to be deleted
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None (currently)
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_stack_delete_np
#ifdef _CMA_PROTO_
	(
	cma_t_stack_np	*stack)
#else	/* no prototypes */
	(stack)
	cma_t_stack_np	*stack;
#endif	/* prototype */
    {
    cma__t_int_stack	*istack;    /* Address of internal stack object */


    cma__int_lock (cma___g_stack_mutex);
    istack = cma__validate_null_stack (stack);

    if (istack == (cma__t_int_stack *)cma_c_null_ptr) {
	cma__int_unlock (cma___g_stack_mutex);
	return;
	}

    cma__queue_remove (&istack->header.queue, istack, cma__t_int_stack);
    cma___free_stack_nolock (istack);
    cma__int_unlock (cma___g_stack_mutex);
    cma__clear_handle (stack);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Reassign a stack descriptor's chunks to a new TCB (note that this is
 *	just a wrapper for cma___assign_stack_nolock, for use by external
 *	routines which don't have access to the stack database lock, or where
 *	it's not already locked for other reasons).
 *
 *  FORMAL PARAMETERS:
 *
 *	stack		the stack to be reassigned
 *
 *	tcb		the tcb to which it is assigned
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
cma__assign_stack
#ifdef _CMA_PROTO_
	(				/* Assign stack chunks to thread */
	cma__t_int_stack	*stack,	/*  stack to assign them to */
	cma__t_int_tcb		*tcb)	/*  tcb to assign them to */
#else	/* no prototypes */
	(stack, tcb)
	cma__t_int_stack	*stack;	/*  stack to assign them to */
	cma__t_int_tcb		*tcb;	/*  tcb to assign them to */
#endif	/* prototype */
    {
    cma__int_lock (cma___g_stack_mutex);
    cma___assign_stack_nolock (stack, tcb);
    cma__int_unlock (cma___g_stack_mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Print all stacks
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	argument count
 *
 *	argv	argument array
 *
 *  IMPLICIT INPUTS:
 *
 *	cluster list
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
 *	Print information on each stack
 */
extern void
cma__debug_list_stacks
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma_t_boolean	show_free = cma_c_false;
    cma_t_boolean	show_full = cma_c_false;
    cma_t_boolean	show_sp = cma_c_false;
    int			arg, j;
    cma_t_address	sp;
    char		output[cma__c_buffer_size];
    cma___t_cluster	*cluster;
    cma___t_index	index;
    static char		*nofreestr = "";
    static char		*freestr = " (free)";
    cma_t_integer	size;
    cma_t_address	start, end;
    cma__t_int_stack	*stack, *last_stack;
    cma_t_boolean	free_chunk[cma__c_chunk_count + 1];
    cma___t_index	idx, limit;


    output[0] = '\0';

    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-') {	/* If a switch */
	    int ta = arg;

	    for (j = 1; argv[ta][j] != 0; j++) {
		switch (argv[ta][j]) {
		    case 'f' : {
			show_full = cma_c_true;
			break;
			}
		    case 'a' : {
			show_free = cma_c_true;
			break;
			}
		    default : {
			cma__putformat (
				output,
				"%s is not a valid switch",
				argv[ta][j]);
			cma__puteol (output);
			}

		    }

		}

	    }
	else {
	    show_sp = cma_c_true;
	    sp = (cma_t_address)cma__strtoul (
		    argv[arg],
		    (char **)cma_c_null_ptr,
		    0);
	    }

	}

    /*
     * If we're not selecting based on the SP value, or if we are and the SP
     * is within the range of the default stack, then display it.
     *
     * NOTE: this is based on our compile-time ASSUMPTIONS regarding the
     * location and size of the default stack, so it's not fool-proof.
     */
    if (!show_sp
	    || (sp >= (cma_t_address) cma__c_def_stack_min
	    && sp <= (cma_t_address) cma__c_def_stack_max)) {
	cma__putformat (
		output,
		"Default stack [assumed] 0x%lx to 0x%lx (%d bytes)",
		cma__c_def_stack_min,
		cma__c_def_stack_max,
		(cma__c_def_stack_max - cma__c_def_stack_min));
	cma__puteol (output);
	}

    cluster = (cma___t_cluster *)cma__list_next (&cma__g_stack_clusters);

    TRY {
	while ((cma__t_list *)cluster != cma__c_null_list) {

	    if (!show_sp
		    || (sp >= cluster->desc.stacks
		    && sp <= cluster->desc.limit)) {

		if (cluster->desc.type == cma___c_bigstack) {
		    cma___t_bigstack	*bs = (cma___t_bigstack *)cluster;

		    if (show_free || bs->in_use) {

			if (bs->in_use) {
			    cma__putformat (
				    output,
				    "Stack 0x%lx (%d bytes); 0x%lx to 0x%lx",
				    (void *)cluster,
				    bs->size,
				    bs->desc.stacks,
				    bs->desc.limit);
			    cma__puteol (output);

			    if (show_full) {
				stack = bs->stack;
				cma__putformat (
					output,
					" base 0x%lx, guard [0x%lx,0x%lx], thread %d",
					stack->stack_base,
					stack->yellow_zone,
					stack->last_guard,
					stack->tcb->header.sequence);
				cma__puteol (output);
				}

			    }
			else {
			    size = bs->size;
			    start = bs->desc.stacks;
			    end = bs->desc.limit;
			    cma__putformat (
				    output,
				    "Stack 0x%lx (%d bytes); 0x%lx to 0x%lx (free)",
				    (void *)cluster,
				    bs->size,
				    bs->desc.stacks,
				    bs->desc.limit);
			    cma__puteol (output);
			    }

			}

		    }
		else {
		    cma__putformat (
			    output,
			    "Stack cluster 0x%lx (%d bytes):",
			    (void *)cluster,
			    cma___g_def_cluster_size);
		    cma__puteol (output);

		    for (index = 0; index <= cma__c_chunk_count; index++)
			free_chunk[index] = cma_c_false;
			
		    index = cluster->free;

		    while (index != cma___c_end) {
			limit = index + cluster->map[index].free.size;

			for (idx = index; idx < limit; idx++)
			    free_chunk[idx] = cma_c_true;

			index = cluster->map[index].free.next;
			}
			
		    last_stack = (cma__t_int_stack *)cma_c_null_ptr;

		    for (index = 0; index < cma__c_chunk_count; index++)

			if (!free_chunk[index]) {
			    stack = cluster->map[index].mapped.stack;
			    size = stack->chunk_count * cma__g_chunk_size;
			    start = (cma_t_address)(
				(cma_t_integer)(cluster->desc.stacks)
				+ (stack->first_chunk * cma__g_chunk_size));
			    end = (cma_t_address)((long)start + size);

			    if (stack != last_stack) {
				last_stack = stack;

				if (!show_sp
					|| (sp >= start
					&& sp <= end)) {
				    cma__putformat (
					    output,
					    " [%d-%d] (%d bytes); 0x%lx-0x%lx",
					    index,
					    index + stack->chunk_count - 1,
					    size,
					    start,
					    end);
				    cma__puteol (output);

				    if (show_full) {
					cma__putformat (
						output,
						"  base 0x%lx, guard [0x%lx,0x%lx], thread %d",
						stack->stack_base,
						stack->yellow_zone,
						stack->last_guard,
						stack->tcb->header.sequence);
					cma__puteol (output);
					}

				    }

				}

			    }

		    if (show_free) {
			index = cluster->free;

			while (index != cma___c_end) {
			    limit = index + cluster->map[index].free.size - 1;
			    size = cluster->map[index].free.size
				* cma__g_chunk_size;
			    start = (cma_t_address)(
				(cma_t_integer)(cluster->desc.stacks)
				+ (stack->first_chunk * cma__g_chunk_size));
			    end = (cma_t_address)((long)start + size);
			    cma__putformat (
				    output,
				    " [%d-%d] (%d bytes); 0x%lx-0x%lx (free)",
				    index,
				    limit,
				    size,
				    start,
				    end);
			    cma__puteol (output);
			    index = cluster->map[index].free.next;
			    }

			}

		    }

		}

	    cluster = (cma___t_cluster *)cma__list_next (&cluster->desc.list);
	    }
	}
    CATCH_ALL {
	output[0] = '\0';
	cma__putformat (output, "Corruption in stack list: can't complete");
	cma__puteol (output);
	}
    ENDTRY
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free a stack... deallocate the stack chunks, and the stack descriptor
 *	structure.
 *
 *  FORMAL PARAMETERS:
 *
 *	desc		Stack to be freed
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
cma__free_stack
#ifdef _CMA_PROTO_
	(
	cma__t_int_stack	*desc)	/* Stack to free */
#else	/* no prototypes */
	(desc)
	cma__t_int_stack	*desc;	/* Stack to free */
#endif	/* prototype */
    {
    cma__int_lock (cma___g_stack_mutex);
    cma___free_stack_nolock (desc);
    cma__int_unlock (cma___g_stack_mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free a list of stacks (traverses from specified stack descriptor to
 *	end of list).
 *
 *  FORMAL PARAMETERS:
 *
 *	list		Queue header of the list to be freed
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
cma__free_stack_list
#ifdef _CMA_PROTO_
	(
	cma__t_queue	*list)		/* Head of stack list to free */
#else	/* no prototypes */
	(list)
	cma__t_queue	*list;		/* Head of stack list to free */
#endif	/* prototype */
    {
    cma__t_int_stack	*stack;


    /*
     * Lock the stack database, since we may be modifying the stack clusters
     */
    cma__int_lock (cma___g_stack_mutex);

    while (! cma__queue_empty (list)) {
	cma__queue_dequeue (list, stack, cma__t_int_stack)
	cma___free_stack_nolock (stack);
	}

    cma__int_unlock (cma___g_stack_mutex);
    }

/*
 * NOTE:
 *
 * cma__get_self_tcb belongs here, by the alphabetical ordering rule.
 * However, it has been moved to the end of the module. It #undefs the macro
 * defined for quicker C access on uniprocessor configurations, and that
 * means that any code later in the module which uses the function would get
 * the call rather than the macro. Moving the definition to the end of the
 * module eliminates that risk.
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return the TCB associated with some SP value (this is intended to
 *	allow returning the CURRENT thread's TCB when the thread has to
 *	temporarily execute on a different stack; e.g., to handle stack
 *	overflow.  Note that this should be callable from interrupt level!
 *
 *  FORMAL PARAMETERS:
 *
 *	sp	Address of the target SP (may be in yellow zone or guard
 *		page)
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
 *	TCB object (or null if SP isn't in valid thread stack).
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_tcb *
cma__get_sp_tcb
#ifdef _CMA_PROTO_
	(
	cma_t_address	sp)
#else	/* no prototypes */
	(sp)
	cma_t_address	sp;
#endif	/* prototype */
    {
    cma___t_cluster	*cluster;
    cma___t_index		index;
    cma__t_int_tcb		*tcb;


    cma___find_sp_cluster (sp, &cluster, &index);

    if (cluster == cma___c_null_cluster)
	if (((cma_t_natural)sp <= cma__c_def_stack_max) 
		&& ((cma_t_natural)sp >= cma__c_def_stack_min))
	    tcb = &cma__g_def_tcb;
	else
	    tcb = (cma__t_int_tcb *)cma_c_null_ptr;
    else {

	if (cluster->desc.type == cma___c_cluster)
	    tcb = cluster->map[index].mapped.tcb;
	else
	    tcb = ((cma___t_bigstack *)cluster)->tcb;

	}

    return tcb;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a stack... use the attributes object to determine the proper
 *	sizes.
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attributes object
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
 *	stack object
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_stack *
cma__get_stack
#ifdef _CMA_PROTO_
	(				/* Create a new stack */
	cma__t_int_attr	*attr)		/* Controlling attr. obj */
#else	/* no prototypes */
	(attr)
	cma__t_int_attr	*attr;		/* Controlling attr. obj */
#endif	/* prototype */
    {
    cma__t_int_stack	*new_stack;
    cma_t_natural	chunks, stack_size, guard_size;
    cma___t_cluster	*cluster;
    cma_t_boolean	found = cma_c_false;
    cma___t_index	*prev_index, index;

    
    guard_size = cma__roundup_chunksize (attr->guard_size);
    stack_size = cma__roundup_chunksize (
	    (attr->stack_size?attr->stack_size:1) + cma__c_reserve_size);
    chunks = (stack_size + cma__roundup_chunksize(cma__c_yellow_size) + 
	      guard_size) / cma__g_chunk_size;

    new_stack = cma__alloc_object (cma__t_int_stack);

    if ((cma_t_address)new_stack == cma_c_null_ptr)
	return (cma__t_int_stack *)cma_c_null_ptr;

    cma__trace ((
	    cma__c_trc_stack | cma__c_trc_obj,
	    "(get_stack) stack 0x%lx; %d chunks (%d data + %d guard)",
	    new_stack,
	    chunks,
	    stack_size,
	    guard_size));

    new_stack->header.type = cma__c_obj_stack;
    new_stack->header.revision = attr->cache[cma__c_obj_stack].revision;
    cma__obj_clear_name (new_stack);
    new_stack->attributes = attr;
    cma__int_lock (cma___g_stack_mutex);
    new_stack->header.sequence = cma___g_stack_seq++;

    /*
     * Determine if the requested stack will fit within normal cluster size
     * or if it needs a "big stack" cluster.
     */
    if (chunks <= cma__c_chunk_count) {
	cma___t_bigstack	*first_big = (cma___t_bigstack *)cma_c_null_ptr;


	/*
	 * Procede to carve the stack out of a normal cluster.
	 */
	cluster = (cma___t_cluster *)cma__list_next (&cma__g_stack_clusters);

	while ((cma__t_list *)cluster != cma__c_null_list) {
	    /*
	     * Traverse the list of stack clusters; look for 
	     * the first one with enough contiguous free chunks 
	     * to fit the new stack.
	     */
	    if (cluster->desc.type == cma___c_bigstack) {

		if (first_big == (cma___t_bigstack *)cma_c_null_ptr
			&& !((cma___t_bigstack *)cluster)->in_use)
		    first_big = (cma___t_bigstack *)cluster;

		}
	    else {

		if (cluster->free != cma___c_end) {
		    index = cluster->free;
		    prev_index = &cluster->free;

		    while (index != cma___c_end) {
			if (cluster->map[index].free.size >= chunks) {
			    found = cma_c_true;
			    new_stack->first_chunk = index;
			    new_stack->cluster = cluster;
			    break;
			    }

			prev_index = &cluster->map[index].free.next;
			index = cluster->map[index].free.next;
			}

		    }

		if (found)
		    break;

		}

	    cluster = (cma___t_cluster *)cma__list_next (
		    &cluster->desc.list);
	    }

	if (! found) {

	    if (first_big != (cma___t_bigstack *)cma_c_null_ptr) {
		/*
		 * If there's a big stack "cluster" on the list, break it up
		 * into a normal cluster and a smaller big stack (if there's
		 * any left).
		 */
		cma___t_bigstack	*rest;
		cma_t_natural		rest_size;


		cluster = (cma___t_cluster *)first_big;
		rest = (cma___t_bigstack *)(
		    (cma_t_integer)first_big + cma___g_def_cluster_size);
		rest_size = (((cma_t_integer)cluster->desc.limit
		    - (cma_t_integer)cluster) - cma___g_def_cluster_size) + 1;

		/*
		 * Convert the first part of the big stack into a
		 * normal cluster, so we can use it.
		 *
		 * Changing the fields of the cluster shouldn't hurt if any
		 * threads are currently traversing. A traverser may or may
		 * not see inconsistent address limit values as the cluster
		 * header is converted, but the limits will always be within
		 * the cluster... and no threads are running out of the
		 * cluster, or we wouldn't be deleting it!
		 */
		cluster->free = cma___c_first_free_chunk;
		cluster->map[0].free.size = cma__c_chunk_count;
		cluster->map[0].free.next = cma___c_end;
		cluster->desc.stacks =
		    (cma_t_address)(cma___compute_first_chunk (
			    cluster,
			    sizeof(cma___t_cluster)));
		cluster->desc.limit =
		    (cma_t_address)((cma_t_integer)(cluster->desc.stacks)
		    + (cma__c_chunk_count * cma__g_chunk_size) - 1);
		cluster->desc.type = cma___c_cluster;
		cma__trace ((
			cma__c_trc_stack,
			"(get_stack) split big stack: new stack cluster 0x%lx",
			cluster));

		/*
		 * Now, if anything is left, insert the remainder as a new
		 * "big stack".
		 */
		if (rest_size >= cma___g_def_cluster_size) {
		    rest->size = rest_size;
		    rest->desc.stacks = (cma_t_address)(cma___compute_first_chunk (
			rest,
			sizeof(cma___t_clu_desc)));
		    rest->desc.limit =
			(cma_t_address)((cma_t_integer)rest + rest_size - 1);
		    rest->desc.type = cma___c_bigstack;
		    cma__trace ((
			cma__c_trc_stack,
			"(get_stack) remainder: big stack 0x%lx (%d bytes)",
			rest,
			rest_size));
		    rest->in_use = cma_c_false;
		    cma__list_insert (&rest->desc.list, &cma__g_stack_clusters);
		    }
		else {
		    /*
		     * If we get here, "rest_size" ought to be 0, since big
		     * stacks are rounded up to a multiple of the cluster
		     * size. On the other hand, I want to check.
		     */
		    if (rest_size != 0) {
			cma__trace ((
			    cma__c_trc_stack,
			    "(get_stack) ** big stack remnant 0x%lx (%d bytes)",
			    rest,
			    rest_size));
			}

		    }

		}
	    else {
		/*
		 * If we didn't find any space, then we need to create a new
		 * stack cluster and add it to the list.
		 */
		cluster = (cma___t_cluster *)cma__alloc_mem (
			cma___g_def_cluster_size);

		if ((cma_t_address)cluster == cma_c_null_ptr) {
		    cma__int_unlock (cma___g_stack_mutex);
		    cma__free_mem ((cma_t_address)new_stack);
		    return (cma__t_int_stack *)cma_c_null_ptr;
		    }

		cluster->free = cma___c_first_free_chunk;
		cluster->map[0].free.size = cma__c_chunk_count;
		cluster->map[0].free.next = cma___c_end;
		cluster->desc.stacks = (cma_t_address)(
		    cma___compute_first_chunk (
			    cluster,
			    sizeof(cma___t_cluster)));
		cluster->desc.limit = (cma_t_address)(
		    (cma_t_integer)(cluster->desc.stacks)
		    + (cma__c_chunk_count * cma__g_chunk_size) - 1);
		cluster->desc.type = cma___c_cluster;
		cma__list_insert (
			&cluster->desc.list,
			&cma__g_stack_clusters);
		cma__trace ((
			cma__c_trc_stack,
			"(get_stack) new stack cluster 0x%lx",
			cluster));
		}

	    index = cluster->free;
	    prev_index = &cluster->free;
	    new_stack->cluster = cluster;
	    new_stack->first_chunk = index;
	    }

	cma__trace ((
		cma__c_trc_stack | cma__c_trc_obj,
		"(get_stack) stack 0x%lx in cluster 0x%lx at chunk %d",
		new_stack,
		new_stack->cluster,
		new_stack->first_chunk));
	/*
	 * To avoid wasting space, slice off just the piece that we need; but
	 * only if the space we found is large enough (we want to minimize
	 * fragmentation, so don't leave a space that's too small to use).
	 */
	if (cluster->map[index].free.size > (chunks + cma___c_min_count)) {
	    cluster->map[index + chunks].free.next
		= cluster->map[index].free.next;
	    cluster->map[index + chunks].free.size
		= cluster->map[index].free.size - chunks;

	    *prev_index = index + chunks;
	    new_stack->chunk_count = chunks;
	    }
	else
	    {
	    new_stack->chunk_count = cluster->map[index].free.size;
	    *prev_index = cluster->map[index].free.next;
	    }

	}
    else {
	cma_t_natural 		big_size;
	cma___t_bigstack	*big;


	/*
	 * Determine big stack cluster size: round up to the next multiple of
	 * the default cluster size, so that when we are completely done the
	 * big stack cluster can be used as default clusters.
	 */
	big_size = cma__roundup (
	    ((cma_t_natural)((sizeof(cma___t_bigstack)) 
	     + ((chunks + 1) * cma__g_chunk_size))),
	    cma___g_def_cluster_size
	    );
	
	big = (cma___t_bigstack *)cma__list_next (&cma__g_stack_clusters);

	while ((cma__t_list *)big != cma__c_null_list) {

	    if (big->desc.type == cma___c_bigstack && big->size >= big_size
		    && !big->in_use) {
		found = cma_c_true;
		big->in_use = cma_c_true;
		cluster = (cma___t_cluster *)big;
		break;
		}

	    big = (cma___t_bigstack *)cma__list_next (&big->desc.list);
	    }

	if (! found) {
	    big = (cma___t_bigstack *)(cma__alloc_mem (big_size));

	    if ((cma_t_address)big == cma_c_null_ptr) {
		cma__int_unlock (cma___g_stack_mutex);
		cma__free_mem ((cma_t_address)new_stack);
		return (cma__t_int_stack *)cma_c_null_ptr;
		}

	    big->desc.stacks = (cma_t_address)(cma___compute_first_chunk (
		big,
		sizeof(cma___t_bigstack)));
	    big->desc.limit =
		(cma_t_address)((cma_t_integer)big + big_size - 1);
	    big->size = big_size;
	    big->desc.type = cma___c_bigstack;
	    cma__trace ((
		cma__c_trc_stack,
		"(get_stack) new big stack cluster 0x%lx (%d bytes)",
		big,
		big_size));
	    big->tcb = (cma__t_int_tcb *)cma_c_null_ptr;
	    big->stack = (cma__t_int_stack *)cma_c_null_ptr;
	    big->in_use = cma_c_true;
	    cma__list_insert (&big->desc.list, &cma__g_stack_clusters);
	    cluster = (cma___t_cluster *)big;
	    }

	new_stack->first_chunk = 0;
	new_stack->chunk_count = chunks;
	new_stack->cluster = cluster;
	}
	     
    /*
     * Now compute the regions of the stack (reserved, yellow, and guard
     * pages), and set page protections for the guard pages if necessary.
     * Most of the code is conditionally divided into stacks that grow up or
     * stacks that grow down.
     */
#ifdef _CMA_UPSTACK_
    new_stack->last_guard =
	(cma_t_address)
	((cma_t_integer)(cluster->desc.stacks)
	+ ((new_stack->first_chunk + new_stack->chunk_count + 1)
	* cma__g_chunk_size) - 1);

    if (guard_size == 0)
	new_stack->yellow_zone = new_stack->last_guard;
    else
	new_stack->yellow_zone =
	    (cma_t_address)
	    ((cma_t_integer)new_stack->last_guard
	    - (guard_size + cma__c_yellow_size) + 1);

    /*
     * This sets the initial SP to the first address of the allocated stack.
     * Which assumes that a push onto SP is sp++.
     */
    new_stack->stack_base = (cma_t_address)(
	(cma_t_integer)(cluster->desc.stacks)
	+ (new_stack->first_chunk * cma__g_chunk_size));

# if _CMA_PROTECT_MEMORY_
    if (new_stack->yellow_zone != new_stack->last_guard) {
	cma__queue_zero (&(new_stack->hole.link));
	new_stack->hole.first = new_stack->yellow_zone;
	new_stack->hole.last = new_stack->last_guard;
	cma__queue_insert (&new_stack->hole.link, &cma___g_holes);
	cma__set_noaccess (new_stack->yellow_zone, new_stack->last_guard);
	new_stack->hole.protected = cma_c_true;
	}
    else
	new_stack->hole.protected = cma_c_false;
# endif
#else					/* ifndef _CMA_UPSTACK_ */
    new_stack->last_guard = 
	(cma_t_address)
	((cma_t_integer)(cluster->desc.stacks)
	+ (new_stack->first_chunk * cma__g_chunk_size));

    if (guard_size == 0)
	new_stack->yellow_zone = new_stack->last_guard;
    else
	new_stack->yellow_zone =
	    (cma_t_address)((cma_t_integer)new_stack->last_guard
	    + guard_size + cma__c_yellow_size - 1);

    /*
     * This sets the initial SP to the chunk ABOVE the last chunk we
     * allocated.  Note that if we allocated the last chunk, this is actually
     * outside of the cluster.  If any C RTL does runtime array range
     * checking, this could break.  On the other hand, this is the easiest
     * and probably most portable way to do it.
     *
     * Of course, it also assumes that the correct initial SP is at the
     * address ABOVE where the first datum should be pushed (i.e., that an SP
     * push is --sp).
     */

    new_stack->stack_base =
	(cma_t_address)((cma_t_integer)(cluster->desc.stacks)
	+ (new_stack->first_chunk + new_stack->chunk_count)
	* cma__g_chunk_size);

# if _CMA_PROTECT_MEMORY_
    if (new_stack->yellow_zone != new_stack->last_guard) {
	cma__queue_zero (&(new_stack->hole.link));
	new_stack->hole.first = new_stack->last_guard;
	new_stack->hole.last = new_stack->yellow_zone;
	cma__queue_insert (&new_stack->hole.link, &cma___g_holes);
	cma__set_noaccess (new_stack->last_guard, new_stack->yellow_zone);
	new_stack->hole.protected = cma_c_true;
	}
    else
	new_stack->hole.protected = cma_c_false;
# endif
#endif

    cma__trace ((
	    cma__c_trc_stack | cma__c_trc_obj,
	    "(get_stack) stack 0x%lx: base 0x%lx, yellow 0x%lx, last guard 0x%lx",
	    new_stack,
	    new_stack->stack_base,
	    new_stack->yellow_zone,
	    new_stack->last_guard));

#if _CMA_STACK_TRACE_
    cma___dump_cluster ("Getting", chunks, cluster);
#endif
    new_stack->tcb = (cma__t_int_tcb *)cma_c_null_ptr;
    cma__int_unlock (cma___g_stack_mutex);

    return new_stack;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize CMA_STACK.C local data
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma___g_first_array
 *
 *  IMPLICIT OUTPUTS:
 *
 *	creates initial stack cluster
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
cma__init_stack
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma___t_cluster	*cluster;


    cma___g_stack_mutex = cma__get_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma___g_stack_mutex, "stack mutex");
    cma___g_stack_seq = 1;

    /*
     * Right now, chunk size is the same as the system "hardware" page size
     * (the memory protection granularity). Since cma__init_memory has
     * already calculated the correct value, just copy it.
     */
    cma__g_chunk_size = cma__g_page_size;

    /*
     * Calculate the size of a default cluster at startup, since it involves
     * rounding up to the chunk size. Saves a lot of overhead later.
     */
    cma___g_def_cluster_size = cma__roundup (
	    ((cma_t_integer)((cma_t_integer)sizeof(cma___t_cluster)
		+ ((cma__c_chunk_count + 1) * cma__g_chunk_size))),
	    cma__g_chunk_size);

    /*
     * Set the default guard & stack sizes
     */
    cma__g_def_attr.guard_size = cma__c_default_guard;
    cma__g_def_attr.stack_size = cma__c_default_stack;

#if _CMA_PROTECT_MEMORY_
    cma__queue_init (&cma___g_holes);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Performs pre- or post- `fork() reinitialization' work.
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
cma__reinit_stack
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
    {

    if (flag == 0) {			    /* pre fork work */
	cma__int_lock(cma___g_stack_mutex);
	}
    else if (flag == 1) {   /* post fork work, out of kernel */
	cma__int_unlock(cma___g_stack_mutex);
	}

    }


#if _CMA_PROTECT_MEMORY_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Remap all stacks so that protected guard pages won't trigger a bug
 *	in UNIX core dump code (it stops when it finds a protected page).
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma___g_holes - queue of holes to re-map.
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
cma__remap_stack_holes
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma___t_int_hole *desc;

    /*
     * WARNING: This should really lock the stack database, but since it's
     * called from a signal handler, it can't. Therefore, be very careful not
     * to call it anywhere else!
     */
    while (!cma__queue_empty (&cma___g_holes)) {
	cma__queue_dequeue(&cma___g_holes, desc, cma___t_int_hole);
	cma__set_writable (desc->first, desc->last);
	desc->protected = cma_c_false;
	}

    }
#endif /* _CMA_PROTECT_MEMORY_ */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Assign a stack descriptor's chunks to a new TCB
 *
 *  FORMAL PARAMETERS:
 *
 *	stack		the stack to be reassigned
 *
 *	tcb		the tcb to which it is assigned
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
static void
cma___assign_stack_nolock
#ifdef _CMA_PROTO_
	(				/* Assign stack chunks to thread */
	cma__t_int_stack	*stack,	/*  stack to assign them to */
	cma__t_int_tcb		*tcb)	/*  tcb to assign them to */
#else	/* no prototypes */
	(stack, tcb)
	cma__t_int_stack	*stack;	/*  stack to assign them to */
	cma__t_int_tcb		*tcb;	/*  tcb to assign them to */
#endif	/* prototype */
    {
    cma___t_cluster	*cluster = stack->cluster;
    cma___t_index	index;
    cma___t_index	first = stack->first_chunk;
    cma_t_natural	count = stack->chunk_count;


    cma__trace ((
	    cma__c_trc_stack,
	    "(assign_stack_nolock) stack 0x%lx assigned to tcb 0x%lx",
	    stack,
	    tcb));

    /*
     * This routine assumes that the caller has locked the stack database!
     */
    cma__assert_warn (
	cma__int_mutex_locked (cma___g_stack_mutex),
	"cma___assign_stack_nolock called without stack database locked.");

    if (cluster->desc.type == cma___c_bigstack) {
	((cma___t_bigstack *)cluster)->tcb = tcb;
	((cma___t_bigstack *)cluster)->stack = stack;
	}
    else {

	for (index = first; index < (first + count); index++) {
	    cluster->map[index].mapped.tcb = tcb;
	    cluster->map[index].mapped.stack = stack;
	    }

	}

    /*
     * If the stack is currently attached to a TCB, remove it from the queue.
     */
    if (stack->tcb != (cma__t_int_tcb *)cma_c_null_ptr) {
	cma__int_lock (stack->tcb->mutex);
	cma__queue_remove (&stack->header.queue, stack, cma__t_int_stack);
	cma__int_unlock (stack->tcb->mutex);
	}

    cma__int_lock (tcb->mutex);
    cma__queue_insert (&stack->header.queue, &tcb->stack);
    cma__int_unlock (tcb->mutex);
    stack->tcb = tcb;
    }
#if _CMA_STACK_TRACE_

static void
cma___dump_cluster
#ifdef _CMA_PROTO_
	(
	char			*type,
	int			chunks,
	cma___t_cluster	*cluster)
#else	/* no prototypes */
	(type, chunks, cluster)
	char			*type;
	int			chunks;
	cma___t_cluster	*cluster;
#endif	/* prototype */
    {
    cma___t_index	index;
    char		output[cma__c_buffer_size];

    output[0] = '\0';
    cma__putstring  (output, type);
    cma__putstring  (output, " ");
    cma__putint	    (output, chunks);
    cma__putstring  (output, " chunks in C ");
    cma__puthex	    (output, (long)cluster);
    cma__putstring  (output, " (");
    cma__putint	    (output, cma__c_chunk_count);
    cma__putstring  (output, ",");
    cma__putint	    (output, cma__g_chunk_count * cma__g_chunk_size);
    cma__putstring  (output, "):");
    cma__puteol	    (output);

    if (cluster->desc.type == cma___c_bigstack) {
	cma__putstring (output, "  Big stack, ");
	cma__putint (output, ((cma___t_bigstack *)cluster)->size);
	cma__putstring (output, " bytes.");
	cma__puteol (output);
	}
    else
	for (
		index = cluster->free;
		index != cma___c_end;
		index = cluster->map[index].free.next) {
		    cma__putstring	(output, "  ");
		    cma__putint	(output, cluster->map[index].free.size);
		    cma__putstring	(output, " free chunks at ");
		    cma__putint	(output, index);
		    cma__puteol	(output);
		}

    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free a set of chunks in a stack cluster
 *
 *  FORMAL PARAMETERS:
 *
 *	cluster		Stack cluster
 *
 *	low_chunk	The lowest chunk to be freed
 *
 *	high_chunk	The highest chunk to be freed
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
static void
cma___free_chunks
#ifdef _CMA_PROTO_
	(				/* Deallocate chunks */
	cma___t_cluster	*cluster,	/*  stack cluster */
	cma___t_index	low_chunk,	/*  low limit */
	cma___t_index	high_chunk)	/*  high limit */
#else	/* no prototypes */
	(cluster, low_chunk, high_chunk)
	cma___t_cluster	*cluster;	/*  stack cluster */
	cma___t_index	low_chunk;	/*  low limit */
	cma___t_index	high_chunk;	/*  high limit */
#endif	/* prototype */
    {
    cma___t_index	index, next, first_chunk, previous, *pnext;
    cma_t_natural	total_size;


    cma__trace ((
	    cma__c_trc_stack,
	    "(free_chunks) freeing cluster 0x%lx, chunks %d to %d",
	    cluster,
	    low_chunk,
	    high_chunk));

    for (index = low_chunk; index <= high_chunk; index++) {
	cluster->map[index].mapped.tcb = (cma__t_int_tcb *)cma_c_null_ptr;
	cluster->map[index].mapped.stack = (cma__t_int_stack *)cma_c_null_ptr;
	}

    total_size = high_chunk - low_chunk + 1;

    if (cluster->free == cma___c_end) {
	/*
	 * If the free list is empty, just add the new batch and forget it.
	 */
	cluster->map[low_chunk].free.next = cma___c_end;
	cluster->map[low_chunk].free.size = total_size;
	cluster->free = low_chunk;
	}
    else {
	/*
	 * The free list isn't empty.  We want to find where the new batch
	 * should be (in index order), and attempt to merge it with the
	 * previous and/or following elements of the free list to avoid
	 * fragmentation.
	 */
	if (cluster->free > low_chunk) {
	    /*
	     * Special case if it goes before the first element on the list,
	     * since we don't have a previous element...
	     */
	    first_chunk = low_chunk;
	    next = cluster->free;
	    cluster->free = low_chunk;
	    }
	else {
	    /*
	     * Find the last element in the list with a lower index than the
	     * new block of chunks, and see if we can merge with it.
	     */
	    next = cluster->free;

	    while ((next != cma___c_end) && (next < low_chunk)) {
		previous = next;
		next = cluster->map[next].free.next;
		}

	    if (low_chunk == (cluster->map[previous].free.size + previous)) {
		/*
		 * If it rests neatly atop the previous element, merge them.
		 */
		first_chunk = previous;
		total_size += cluster->map[previous].free.size;
		}
	    else {
		/*
		 * If it can't merge, point the previous element at it and
		 * set up to check for merge with next element
		 */
		first_chunk = low_chunk;
		cluster->map[previous].free.next = low_chunk;
		}

	    }

	/*
	 * Now, see whether we can merge the element with the next one (if
	 * any).
	 */
	if (next != cma___c_end) {
	    if ((first_chunk + total_size) == next) {
		/*
		 * Merge them!
		 */
		total_size += cluster->map[next].free.size;
		next = cluster->map[next].free.next;
		}

	    cluster->map[first_chunk].free.next = next;
	    }
	else
	    cluster->map[first_chunk].free.next = cma___c_end;

	cluster->map[first_chunk].free.size = total_size;
	}

#if _CMA_STACK_TRACE_
    cma___dump_cluster ("Freeing", high_chunk - low_chunk + 1, cluster);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free a stack... deallocate the stack chunks, and the stack descriptor
 *	structure.  This assumes the caller has locked the stack database.
 *
 *  FORMAL PARAMETERS:
 *
 *	desc		Stack to be freed
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
static void
cma___free_stack_nolock
#ifdef _CMA_PROTO_
	(
	cma__t_int_stack	*desc)	/* Stack to free */
#else	/* no prototypes */
	(desc)
	cma__t_int_stack	*desc;	/* Stack to free */
#endif	/* prototype */
    {
    cma___t_cluster	*cluster = desc->cluster;
    cma_t_integer	low_chunk, high_chunk;
    cma_t_address	*qtmp;


    cma__trace ((
	    cma__c_trc_stack | cma__c_trc_obj,
	    "(free_stack_nolock) freeing stack 0x%lx",
	    desc));

    /*
     * This routine assumes that the caller has locked the stack database!
     */
    cma__assert_warn (
	cma__int_mutex_locked (cma___g_stack_mutex),
	"cma___free_stack_nolock called without stack database locked.");

    desc->header.sequence = 0;

#if _CMA_PROTECT_MEMORY_
    if (desc->hole.protected) {
	/*
	 * Unprotect the yellow zone and guard pages before returning stack
	 * chunks to the free list.
	 */
# ifdef _CMA_UPSTACK_
	cma__set_writable (desc->yellow_zone, desc->last_guard);
# else
	cma__set_writable (desc->last_guard, desc->yellow_zone);
# endif
	cma__queue_remove (&desc->hole.link, qtmp, cma_t_address);
	desc->hole.protected = cma_c_false;
	}
#endif

    if (cluster->desc.type == cma___c_cluster)
	cma___free_chunks (
		cluster,
		desc->first_chunk,
		desc->first_chunk + desc->chunk_count - 1);
    else
	((cma___t_bigstack *)cluster)->in_use = cma_c_false;

    cma__free_mem ((cma_t_address)desc);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine performs a limit check on the current stack
 *
 *  FORMAL PARAMETERS:
 *
 *	size    -  The number of bytes by which the stack will be extended
 *	sp      -  The address from which the extension will occur
 *	cluster -  The cluster into which SP points
 *	index	-  The index into the cluster
 *
 *  IMPLICIT INPUTS:
 *
 *	The address space  ;-)
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Returns true (1) if the requested stack extension is valid
 *	Returns false (0) if the requested stack extension is invalid
 *
 *  SIDE EFFECTS:
 *
 *	None (currently)
 */
static cma_t_boolean
cma___int_check_limit
#ifdef _CMA_PROTO_
	(
	cma_t_integer	size,
	cma_t_address	sp,
	cma___t_cluster	*cluster,
	cma___t_index	index)
#else	/* no prototypes */
	(size, sp, cluster, index)
	cma_t_integer	size;
	cma_t_address	sp;
	cma___t_cluster	*cluster;
	cma___t_index	index;
#endif	/* prototype */
    {
    cma__t_int_stack	*stack;
    cma_t_address	ptr;
    cma_t_address	cur_sp;
    cma_t_address	new_top;
    cma_t_boolean	alloc_ok = cma_c_true;


    if (cluster == cma___c_null_cluster) {
	/*
	 * "High water mark" for stack check.
	 */
	static cma_t_address	cur_stack_limit = 0;


	/*
	 * Null cluster implies default stack.  
	 *
	 * Check (static variable) to see if we have already performed a 
	 * stack check on the requested address range.  If not, then each 
	 * page between the last checked page and the end of the requested 
	 * allocation (and then some reserve space) inorder to force 
	 * the OS to create the pages and to make sure they're writable.
	 *
	 * Failure to create pages is assumed to raise an exception which
	 * is caught and converted to a return status.
	 *
	 * Start touching above the _current_ SP, to avoid corrupting the 
	 * current stack frame.  If the "current top of stack limit" is
	 * zero, then set it to the current SP.
	 */
	cur_sp = (cma_t_address)cma__fetch_sp ();

	if (cur_stack_limit == 0)
	    cur_stack_limit = cur_sp;

#ifdef _CMA_UPSTACK_
	new_top = (cma_t_address)((char *)sp + (size + cma__c_reserve_size));

	if (new_top > cur_stack_limit) {
	    TRY {
		ptr = cma__max (cur_stack_limit, cur_sp);

		while( ptr < new_top ) {
		    ptr = (cma_t_address)
			  ((cma_t_integer)ptr + cma__g_chunk_size);
		    *(cma_t_integer *)ptr = 0xdeadbeef;
		    }

		*(cma_t_integer *)new_top = 0xdeadbeef;
		cur_stack_limit = new_top;
		}
	    CATCH_ALL {		    
		alloc_ok = cma_c_false;
		}
	    ENDTRY;
	}
#else
	new_top = (cma_t_address)((char *)sp - (size + cma__c_reserve_size));

	if (new_top < cur_stack_limit) {
	    TRY {
		ptr = cma__min (cur_stack_limit, cur_sp);

		while( ptr > new_top ) {
		    ptr = (cma_t_address)
			  ((cma_t_integer)ptr - cma__g_chunk_size);
		    *(cma_t_integer *)ptr = 0xdeadbeef;
		    }

		*(cma_t_integer *)new_top = 0xdeadbeef;
		cur_stack_limit = new_top;
		}
	    CATCH_ALL {
		alloc_ok = cma_c_false;
		}
	    ENDTRY;
	    }
#endif
	}   /* close of cluster is null_cluster */
    else
	{

	if (cluster->desc.type == cma___c_cluster)
	    stack = cluster->map[index].mapped.stack;
	else
	    stack = ((cma___t_bigstack *)cluster)->stack;

#ifdef _CMA_UPSTACK_
	if (((cma_t_address)((char *)sp + (size + cma__c_reserve_size))) 
		> stack->yellow_zone)
#else
	if (((cma_t_address)((char *)sp - (size + cma__c_reserve_size))) 
		< stack->yellow_zone)
#endif
	    alloc_ok = cma_c_false;
	else
	    alloc_ok = cma_c_true;

	}

    return alloc_ok;		    
    }    

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Find the current thread's TCB.  This assumes that the SP is within a
 * 	stack controlled by the stack manager (or on the default process
 *	stack, which corresponds to the default thread).
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	SP
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Address of the TCB
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#if _CMA_HARDWARE_ != _CMA__ALPHA || _CMA_OSIMPL_ != _CMA__OS_OSF
# undef cma__get_self_tcb

extern cma__t_int_tcb *
cma__get_self_tcb
# ifdef _CMA_PROTO_
	(void)
# else	/* no prototypes */
	()
# endif	/* prototype */
    {
# if _CMA_UNIPROCESSOR_
    /*
     * This function isn't called from C code on a uniprocessor
     * configuration, however it's here for use by Assembler code.
     */
    return (cma__g_current_thread);
# else
    cma_t_address	sp;
    cma___t_cluster	*cluster;
    cma___t_index	index;
    cma__t_int_tcb	*tcb;


    cma___get_stack_info (&sp, &cluster, &index);

    if (cluster == cma___c_null_cluster)
	if (((cma_t_natural)sp <= cma__c_def_stack_max) 
		&& ((cma_t_natural)sp >= cma__c_def_stack_min))
	    tcb = &cma__g_def_tcb;
	else
	    RAISE (cma_e_notcmastack);
    else {

	if (cluster->desc.type == cma___c_cluster)
	    tcb = cluster->map[index].mapped.tcb;
	else
	    tcb = ((cma___t_bigstack *)cluster)->tcb;

	}

    return tcb;
# endif					/* _CMA_UNIPROCESSOR_ */
    }
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_STACK.C */
/*  *40   15-JUN-1993 08:08:15 KEANE "Touch up compile problems for DEC C on OSF/1 AXP" */
/*  *39    2-JUN-1993 16:28:58 KEANE "Remove unused getpagesize() prototype" */
/*  *38   27-MAY-1993 14:32:39 BUTENHOF "Fix guard size = 0 bug" */
/*  *37   10-MAY-1993 14:11:08 BUTENHOF "List default stack (assumed)" */
/*  *36    6-MAY-1993 19:07:08 BUTENHOF "Show size of stack cluster" */
/*  *35   16-APR-1993 13:05:34 BUTENHOF "Update queue operations to allow INSQUE/REMQUE" */
/*  *34    5-APR-1993 05:51:50 BUTENHOF "Fix MIPS compilation error" */
/*  *33    1-APR-1993 14:33:13 BUTENHOF "Remove use of %08x formatting" */
/*  *32   29-MAR-1993 13:56:29 BUTENHOF "Fix typecast problem" */
/*  *31   24-MAR-1993 14:17:38 SCALES "Touch up check-limit" */
/*  *30   19-MAR-1993 21:51:20 SCALES "Break check-stack-limit in two" */
/*  *29    8-FEB-1993 06:04:35 BUTENHOF "Fix stack protection" */
/*  *28    4-FEB-1993 15:43:58 BUTENHOF "Ooops!" */
/*  *27   25-JAN-1993 11:34:15 BUTENHOF "Round size & reserve as one unit" */
/*  *26   19-JAN-1993 12:56:37 BUTENHOF "Fix error in bigstack creation" */
/*  *25   15-SEP-1992 13:50:28 BUTENHOF "Change sequencing" */
/*  *24   13-AUG-1992 14:44:06 BUTENHOF "Set and use Alpha pal code unique value" */
/*  *23   15-MAY-1992 15:04:20 SCALES "Add additional queue consistency checks" */
/*  *22   13-MAR-1992 14:09:46 BUTENHOF "Rearrange stack/guard rounding" */
/*  *21    6-MAR-1992 13:34:43 BUTENHOF "Disable call_on_stack for _CMA_SHLIB_" */
/*  *20   18-FEB-1992 15:30:15 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *19   10-FEB-1992 08:50:44 BUTENHOF "Move get_self_tcb to avoid conflicts" */
/*  *18   29-JAN-1992 14:37:10 BUTENHOF "Improve special cluster management" */
/*  *17   21-JAN-1992 11:49:59 BUTENHOF "Fix special cluster bug" */
/*  *16    2-DEC-1991 13:40:34 CURTIN "Removed an erroneous dollar sign from within a comment" */
/*  *15   22-NOV-1991 11:56:49 BUTENHOF "Cast cma__fetch_sp() to cma_t_address" */
/*  *14   11-NOV-1991 11:56:05 BUTENHOF "Fix MIPS C compilation warnings" */
/*  *13    8-NOV-1991 00:38:03 BUTENHOF "Fix bug in large stack creation" */
/*  *12   14-OCT-1991 13:40:11 BUTENHOF "Unprotect guard pages on abort" */
/*  *11   24-SEP-1991 16:28:14 BUTENHOF "Fix MIPS C compilation warning" */
/*  *10   17-SEP-1991 13:23:44 BUTENHOF "Fix MIPS C compilation warning" */
/*  *9    26-JUL-1991 15:54:11 CURTIN "Added cast to a macro for mips cc" */
/*  *8    18-JUL-1991 09:45:59 CURTIN "Added large stack support" */
/*  *7    10-JUN-1991 18:23:46 SCALES "Add sccs headers for Ultrix" */
/*  *6     5-JUN-1991 16:14:46 CURTIN "fork work" */
/*  *5    24-MAY-1991 16:47:32 CURTIN "Added a new reinit routine" */
/*  *4     2-MAY-1991 13:59:08 BUTENHOF "Add trace statements" */
/*  *3    12-APR-1991 23:36:53 BUTENHOF "Change _CMA_UNIPROCESSOR_ to 0/1" */
/*  *2    14-DEC-1990 00:55:52 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:53:24 BUTENHOF "Thread stacks" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_STACK.C */
