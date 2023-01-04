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
 * @(#)$RCSfile: cma_vp_defs.h,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:56:05 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Header file for virtual processor services
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	03 April 1991
 *
 *  MODIFICATION HISTORY:
 * 
 *	001	Dave Butenhof	24 May 1991
 *		Change "history" value into flags word for flexibility.
 *	002	Dave Butenhof	02 June 1991
 *		Add new return status for trying to delete a VP that's not
 *		suspended.
 *	003	Dave Butenhof	11 September 1991
 *		Add support for caching VPs to improve performance.
 *	004	Dave Butenhof	08 February 1992
 *		Rework state flags for better debugging.
 *	005	Webb Scales	13 March 1992
 *		Move VP structure here from cma_sched.h
 *	006	Brian Keane	15 June 1992
 *		Change some int members of cma__t_vp to cma_t_integer.
 *	007	Dave Butenhof	11 August 1992
 *		Remove the vp_running counter.
 *	008	Dave Butenhof	 4 November 1992
 *		Add lifetime sequence counter to VP structure.
 *	009	Dave Butenhof	25 November 1992
 *		Add policy and priority "cache" to VP structure.
 *	010	Dave Butenhof	 5 January 1993
 *		Add new failure status.
 *	011	Dave Butenhof	 6 January 1993
 *		Same only different.
 *	012	Dave Butenhof	27 January 1993
 *		Add zombie "state"
 *	013	Dave Butenhof	26 February 1993
 *		Integrate review comments (23 February 1993)
 *	014	Dave Butenhof	 9 March 1993
 *		Add new status bit to tell 'new' VPs from 'recycled'.
 *	015	Dave Butenhof	10 March 1993
 *		Add values for maximum Mach & VP state constants.
 *	016	Dave Butenhof	 5 April 1993
 *		Remove recycle bit and "reset" pointer: make reclaimation
 *		synchronous with higher level.
 *	017	Dave Butenhof	14 May 1993
 *		Add an error definition for mach's 'no memory'
 *	018	Dave Butenhof	28 May 1993
 *		Add space for real mach state when thread is held.
 */

#ifndef CMA_VP_DEFS
#define CMA_VP_DEFS

/*
 *  INCLUDE FILES
 */

#if _CMA_KTHREADS_ == _CMA__MACH
# include <mach.h>
# include <mach/message.h>
#endif

#include <cma_queue.h>
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * Define return status values
 */
#if _CMA_KTHREADS_ == _CMA__MACH
# define cma__c_vp_normal	KERN_SUCCESS
# define cma__c_vp_err_resource	KERN_RESOURCE_SHORTAGE
# define cma__c_vp_err_inval	KERN_INVALID_ARGUMENT
# define cma__c_vp_err_timeout	RCV_TIMED_OUT
# define cma__c_vp_failure	KERN_FAILURE
# define cma__c_vp_insfmem	KERN_MEMORY_ERROR
#else
# define cma__c_vp_normal	0	/* Completed OK */
# define cma__c_vp_err_resource	1	/* Resource shortage (create) */
# define cma__c_vp_err_inval	2	/* Invalid parameter */
# define cma__c_vp_err_timeout	3	/* Timeout */
# define cma__c_vp_failure	4	/* Catchall */
# define cma__c_vp_insfmem	5	/* Memory error */
#endif

/*
 * The following additional codes were chosen to avoid conflict with O/S
 * codes.
 */
#define cma__c_vp_err_badstate	-500
#define cma__c_vp_err_nopriv	-501

#if _CMA_KTHREADS_ == _CMA__MACH
/*
 * Code for RPC message code (VP suspend/resume)
 */
# define cma__c_vp_msg_resume	0xACEFACE
# define cma__c_vp_msg_kernel	0xBEEFED
#endif

/*
 * TYPEDEFS
 */

typedef int	cma__t_vp_status;

typedef cma_t_address 	(*cma__t_vp_handler) _CMA_PROTOTYPE_ ((
	cma_t_address	arg));

typedef cma_t_address 	(*cma__t_vp_startroutine) _CMA_PROTOTYPE_ ((
	cma_t_address	arg));

/*
 * Flags to show the state of the kernel threads. The low word is "primary
 * state", the high word is modifiers.
 */
#if _CMA_KTHREADS_ == _CMA__MACH
# define cma__c_vp_new		0x00000001
# define cma__c_vp_running	0x00000002
# define cma__c_vp_susp		0x00000004
# define cma__c_vp_cached	0x00000008
# define cma__c_vp_zombie	0x00000010
# define cma__c_vp_default	0x00010000
# define cma__c_vp_hold		0x00020000

typedef struct CMA__T_VPID {
    cma__t_queue	queue;		/* Queue of VPs */
    thread_t		vp;		/* Actual Mach VP id */
    port_t		synch;		/* Synch port */
    cma_t_integer	flags;		/* What's it doing */
    cma_t_integer	seq;		/* Lifetime sequence counter */
    cma_t_sched_policy	policy;
    cma_t_priority	priority;
    cma_t_integer	hold_run;	/* run_state for held thread */
    cma_t_integer	hold_mach;	/* mach_state for held thread */
    cma_t_integer	hold_suspend;	/* suspend_count for held thread */
    } cma__t_vstruct, *cma__t_vpid;
#else
typedef cma_t_address 	cma__t_vpid;
#endif

/*
 * These are state values used in the "public" vp_state structure, for
 * external callers.
 */
#define cma__c_vp_st_run	1
#define cma__c_vp_st_stop	2
#define cma__c_vp_st_susp	3
#define cma__c_vp_st_hold	4
#define cma__c_vp_st_unks	5
#define cma__c_vp_st_unkw	6
#define cma__c_vp_st_unk	7
#define cma__c_vp_st_stopping	8
#define cma__c_vp_st_len	9	/* Number of elements */

typedef struct CMA__T_VP_STATE {
    cma_t_sched_policy	policy;
    cma_t_priority	priority;
    cma_t_integer	tcb;
    cma_t_integer	start_routine;
    cma_t_integer	start_arg;
    cma_t_integer	stack;
    cma_t_integer	run_state;
#if _CMA_KTHREADS_ == _CMA__MACH
    cma_t_integer	mach_state;
#endif
    cma_t_integer	suspend_count;
    } cma__t_vp_state;

/*
 * List of known virtual processors' per-processor data structures
 */
typedef struct CMA__T_VP {
    cma__t_queue	queue;
    cma__t_int_tcb	*current_thread;
    cma__t_vpid		vp_id;
    } cma__t_vp;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#endif
