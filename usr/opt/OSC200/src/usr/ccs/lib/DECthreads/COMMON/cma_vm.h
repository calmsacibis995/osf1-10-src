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
 *	@(#)$RCSfile: cma_vm.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:54:53 $
 */
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	Header file for VM functions
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	20 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	26 September 1989
 *		Add defs for new routines.
 *	002	Webb Scales	 7 November 1989
 *		Added cma__alloc_object macro
 *	003	Dave Butenhof	29 May 1990
 *		Make VM mutex extern so it can be accessed by rtl linking
 *		against libcma.a.
 *	004	Dave Butenhof	7 June 1990
 *		Add cma__clear_object macro to clear object in debug build.
 *	009	Dave Butenhof	26 June 1990
 *		Replace use of private "VM mutex" with the generic "global
 *		mutex".
 *	010	Paul Curtin	2 August 1990
 *		Added memory definitions and interface definitions.
 *		Replaced memset w/ cma__memset
 *	011	Paul Curtin	22 August 1990
 *		Removed prototype for cma__alloc_mem_nolock.  Adjusted
 *		alloc sizes so as to be dynamic within classes of
 *		structures.
 *	012	Dave Butenhof	29 October 1990
 *		Round up allocation sizes to next octaword (8 byte boundary)
 *		to ensure packets are aligned.
 *	013	Paul Curtin	 7 November 1990
 *		Added 'total' to lookup list structure for mem tracking.
 *	014	Paul Curtin	22 May 1991
 *		Adjusted the number of pages in a memory pool.
 *	015	Paul Curtin	24 May 1991
 *		Added a proto for cma__reinit_memory.
 *	016	Dave Butenhof	10 October 1991
 *		Add cma__free_area to handle recycling large packages
 *		allocated via cma__get_area (such as special stack clusters).
 *		Also, export some of the VM information so cma_debug can
 *		report it.
 *	017	Paul Curtin	18 November 1991
 *		Added a prototype for cma__init_mem_locks.
 *	018	Dave Butenhof	10 February 1992
 *		Add cma__alloc_zero. Make "get_area_nolock" private to vm.
 *	019	Webb Scales	13 May 1992
 *		Added (conditional) initialization to alloc-object.  Added
 *		a conditional routine to do the initialization (expands
 *		harmlessly as a macro for production builds).
 *	020	Webb Scales	19 May 1992
 *		Fix a conditional type-o in the init-object macro
 *	021	Brian Keane	09 June 1992
 *		Change definition of cma__c_mem_tag to be tied to size of cma_t_integer, to handle
 *	        both 32 and 64 bit platforms.
 *	022	Dave Butenhof	15 April 1993
 *		Add extern definition for array of VM queue names.
 *	023	Dave Butenhof	25 May 1993
 *		Minimize pool fragmentation by adjusting packet sizes.
 */

#ifndef CMA_VM
#define CMA_VM

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_mutex.h>
#include <cma_util.h>
#include <cma_condition.h>
#include <cma_attr.h>
#include <cma_defer.h>
#include <cma_dispatch.h>
#include <cma_stack.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma__c_small	    0		    /* Index for small areas	     */
#define	cma__c_med	    1		    /* Index for medium areas	     */
#define cma__c_large	    2		    /* Index for large areas	     */
#define cma__c_pool	    3		    /* Indicates size > cma__c_large */
#define cma__c_alloc_sizes  4		    /* Number of allocation sizes    */

#define cma__c_mem_tag	    sizeof(cma_t_integer)*2	/* bytes used to store size	     */
#define cma__c_pool_size    (20 * cma__c_page_size)	/* internal pool size */

#define cma__alloc_object(type) \
    (type *)cma__init_object (cma__alloc_mem (sizeof (type)))

#define cma__clear_object(object) \
    cma__memset ((cma_t_address)object, 0, sizeof *object)

#ifdef NDEBUG
# define cma__init_object(obj_ptr) (obj_ptr)
#endif

/*
 * TYPEDEFS
 */

typedef struct CMA__T_ALLOC_QUEUE {
    cma_t_integer	size;		/* Size of memory objects */
    cma_t_integer	count;		/* Number of objects in list */
    cma_t_integer	total;		/* Total of objects existing */
    cma__t_int_mutex	*mutex;		/* Mutex for queue access */
    cma__t_queue	queue;		/* Singular look up list */
    } cma__t_alloc_queue;

typedef struct CMA__T_POOL_STAT {
    cma_t_integer	allocs;		/* Number of sbrks or expregs */
    cma_t_integer	bytes_allocd;	/* Number of bytes sbrkd or expregd */
    cma_t_integer	exact_allocs;	/* Number of allocs for exact size */
    cma_t_integer	failures;	/* Number of times allocation failed */
    cma_t_integer	zero_allocs;	/* Number of zeroed allocations */
    cma_t_integer	zero_bytes;	/* Number of zeroed bytes */
    cma_t_integer	extractions;	/* Number of packets extracted */
    cma_t_integer	breakups;	/* Number of packets split up */
    cma_t_integer	returns;	/* Number of packets returned */
    cma_t_integer	merge_befores;	/* Returns merged with previous */
    cma_t_integer	merge_afters;	/* Returns merged with next */
    cma_t_integer	scrounges[3];	/* Scrounges from each list */
    } cma__t_pool_stat;

/*
 *  GLOBAL DATA
 */

extern cma__t_alloc_queue	cma__g_memory[cma__c_alloc_sizes];
extern cma_t_integer		cma__g_page_size;
#if _CMA_OS_ == _CMA__UNIX
 extern cma_t_integer		cma__g_sbrk_align;
#endif
extern cma__t_pool_stat		cma__g_pool_stat;
extern char			*cma__g_vm_names[cma__c_alloc_sizes];

/*
 * INTERNAL INTERFACE
 */

extern cma_t_address
cma__alloc_mem _CMA_PROTOTYPE_ ((
	cma_t_natural	size));		/* Size of VM to allocate, bytes */

extern cma_t_address
cma__alloc_zero _CMA_PROTOTYPE_ ((
	cma_t_natural	size));		/* Size of VM to allocate, bytes */

extern void
cma__free_mem _CMA_PROTOTYPE_ ((
	cma_t_address	vm));		/* Address of VM to free */

extern void
cma__init_memory _CMA_PROTOTYPE_ ((void));  /* memory initialization routine */

extern void
cma__init_mem_locks _CMA_PROTOTYPE_ ((void)); 

#ifndef NDEBUG
extern cma_t_address
cma__init_object _CMA_PROTOTYPE_ ((
	cma_t_address obj_ptr));	/* Address of object to initialize */
#endif

extern void
cma__reinit_memory _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));	
	
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_VM.H */
/*  *15   27-MAY-1993 14:32:59 BUTENHOF "Reduce fragmentation" */
/*  *14   16-APR-1993 13:06:59 BUTENHOF "Add static list of VM lookaside names" */
/*  *13   15-JUN-1992 16:45:25 KEANE "Octaword align memory allocations on Alpha OSF/1" */
/*  *12   19-MAY-1992 14:20:58 SCALES "Fix typo" */
/*  *11   15-MAY-1992 15:05:35 SCALES "Add additional queue consistency checks" */
/*  *10   18-FEB-1992 15:31:53 BUTENHOF "Add cma__alloc_zero (pre-cleared memory)" */
/*  *9    18-NOV-1991 11:19:40 CURTIN "Added a prototype for cma__init_mem_locks" */
/*  *8    14-OCT-1991 13:42:40 BUTENHOF "Add support for large allocations" */
/*  *7    10-JUN-1991 19:58:13 SCALES "Convert to stream format for ULTRIX build" */
/*  *6    10-JUN-1991 19:22:25 BUTENHOF "Fix the sccs headers" */
/*  *5    10-JUN-1991 18:25:17 SCALES "Add sccs headers for Ultrix" */
/*  *4    24-MAY-1991 16:52:30 CURTIN "Added a new reinit routine" */
/*  *3    23-MAY-1991 16:39:02 CURTIN "Changed the size of pools to accomodate all of init" */
/*  *2    17-DEC-1990 09:33:08 CURTIN "Added field for memory statistics" */
/*  *1    12-DEC-1990 21:55:59 BUTENHOF "VM management" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_VM.H */
