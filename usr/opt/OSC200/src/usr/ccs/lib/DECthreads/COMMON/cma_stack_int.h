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
 *	@(#)$RCSfile: cma_stack_int.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:33:55 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for stack management (internal to cma_stack.c, but
 *	separate for convenience, and unit testing).
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	30 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	002	Paul Curtin	27 June	1990
 *		Changed stack structure to reflect new indexing, Added
 *		cma___c_null_reserve_size.
 *	003	Paul Curtin	3 July 1990
 *		Added cma__c_ stack constants here from cma_defs.h
 *	004	Paul Curtin	24 July 1990
 *		Added cma__roundup_chunksize macro
 *	005	Paul Curtin	10 September 1990
 *		Changed default guard size from 10 to 8 (512 byte) pages.
 *	006	Webb Scales	 6 December 1990
 *		Added HP-specific constants.
 *	007	Paul Curtin	19 February 1991
 *		Adjusting cluster size.
 *	008	Dave Butenhof	25 February 1991
 *		Set stack chunk count for AIX.
 *	009	Paul Curtin	23 May 1991
 *		Adjust stack clusters to handle 8 threads.
 *	010	Dave Butenhof and Webb Scales	05 June 1991
 *		Conditionalize vacuous (forward) structure defs, since MIPS C
 *		V2.1 doesn't like (or, apparently, need).
 *	011	Paul Curtin	11 June 1991
 *		Added large stack support; two constants and a struct field.
 *	012	Dave Butenhof	02 October 1991
 *		Integrate changes provided by Alan Peckham to unprotect guard
 *		pages on all stacks before aborting the process; this allows
 *		the UNIX core dump code (which makes the absurd assumption
 *		that the known universe stops just because a page is
 *		protected) to work on threaded processes.
 *	013	Dave Butenhof	11 November 1991
 *		Fix type of cluster_limit and stacks fields to cma_t_address
 *		(rather than cma_t_address *, which doesn't make much sense).
 *	014	Dave Butenhof	22 November 1991
 *		Add a chunk count value for Alpha.
 *	015	Dave Butenhof	28 January 1992
 *		Add new structure type for special clusters, rather than
 *		wasting space on a "thread map" that's not relevant.
 *	016	Dave Butenhof	06 March 1992
 *		Save on swapfile space by cutting stack size (but not on VMS,
 *		since current values were selected for VAX Ada compatibility,
 *		which was considered important).
 *	017	Webb Scales	27 March 1992
 *		Fix a cut-and-paste-o from 016: reset VMS reserve size to the
 *		appropriate numbers for VAX Ada.
 *	018	Dave Butenhof	02 September 1992
 *		New logic for stack sizes -- allow for 20 "average frames" on
 *		each architecture.
 *	019	Dave Butenhof	08 September 1992
 *		Enhance the comment explaining 018, by including the table
 *		Webb and I developed to estimate the sizes. Also, add
 *		explanation of the exceptions.
 *	020	Dave Butenhof	 2 December 1992
 *		In checking stack sizes, I realize I inadvertently typed in
 *		the "b" (for "byte") from the original stack size list, as
 *		the digit "6" -- effectively multiplying the desired AXP
 *		stack sizes by 10.
 *	021	Dave Butenhof	25 January 1993
 *		Resolve an ancient FIX-ME in cma_deb_core.c by moving
 *		cma___c_reserve_size to cma_stack.h (as cma__c_reserve_size).
 */

#ifndef CMA_STACK_INT
#define CMA_STACK_INT

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_queue.h>
#include <cma_list.h>
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma___c_first_free_chunk	0
#define cma___c_min_count	2	/* Smallest number of chunks to leave */
#define cma___c_end		(-1)	/* End of free list (flag) */
#define cma__c_yellow_size	0

/* 
 * Cluster types
 */
#define cma___c_cluster  0	/* Default cluster */
#define cma___c_bigstack 1	/* Looks like a cluster, but it's a stack */

#define cma___c_null_cluster	(cma___t_cluster *)cma_c_null_ptr

/*
 * Define the default stack sizes on the various platforms that DECthreads
 * supports. We want the stack to be large enough for "typical threads"
 * (whatever that means) without making the stack so large as to unreasonably
 * limit the number of those threads that can exist within a "typical address
 * space".
 *
 * The way we approach this (as of BL11) is to make some arbitrary
 * assumptions, based on the calling standard for each platform, as to what
 * constitutes a "typical stack frame", and allow for 20 of these in a
 * default stack. We recognize that this calculation is arbitrary; but at
 * least it provides some consistency across platforms.
 *
 *	1)	Normal "frame" (at least return PC, maybe more)
 *
 *	2)	Typical callee- and caller-saved register set (we assumed 6
 *		saved registers).
 *
 *	3)	Local (automatic) variables (we assumed 4).
 *
 *	4)	Argument list (or part of it on stack -- note that MIPS, for
 *		example, provides stack "homes" for arguments passed in
 *		registers). (We assumed 4 arguments).
 *
 *	5)	A single DECthreads TRY block.
 *
 * The estimates try to account for "common usage" as well as the basic
 * rules. For example, VAX ULTRIX code would tend to save fewer
 * "callee-saved" registers and is more like to use "caller-saved" registers
 * than VAX VMS code, since on VAX VMS only R0 and R1 (return value) are
 * caller-saved; whereas on VAX ULTRIX R0-R5 are caller-saved.
 *
 * The estimates used:
 *
 *	(format note: each field contains "x/y (z)" notation, where "x" is
 *	number of integer registers, "/y" is number of float registers if
 *	there's a difference, and "(z)" is the number of bytes allocated to
 *	the sum).
 *
 * Content		VMS	VAX	MIPS	EVMS	ALPHA
 * -----------		-------	-------	-------	-------	-------
 * Callee-save		5(20)	4(16)	2/1(16)	4/2(48)	2/2(32)
 * Caller-save		0(00)	2(08)	2/1(16) 0/0(00)	2/1(24)
 * Local vars		4(16)	4(16)	4(16)	4(32)	4(32)
 * Arg list		4(16)	4(16)	4(16)	4(32)	4(32)
 * Frame		(20)	(20)	(4)	(32)	(8)
 * Try block		(272)	(248)	(540)	(416)	(928)
 * -----------		-------	-------	-------	-------	-------
 * Total bytes/frame:	348	324	608	560	1056
 *
 * Frames in 8K stack:	23	24	13	14	7
 *
 * size for 20 frames:	6960	6480	12160	11200	21120
 *  machine prot units:	14x512	2x4096	3x4096	2x8192	3x8192
 *  rounded up to page:	7168	8192	12288	16384	24576
 *
 * Note that the size given as cma__c_default_stack is automatically rounded
 * up to the target's protection boundary before use -- the constants given
 * here are those for the "20 frames" line, but the actual size used will be
 * that given for "rounded up". Also note that, due to concerns for VMS
 * upwards compatibility requirements, the OpenVMS VAX default size remains
 * the same as in VAX VMS V5.5 -- the size originally used by the VAX Ada
 * RTL.
 *
 * Also, the sizes specified by Hewlett-Packard for their targets have been
 * left unchanged. Other non-Digital UNIX platforms are currently given the
 * default of 16 512-byte pages.
 */
#if _CMA_VENDOR_ == _CMA__HP
# if _CMA_HARDWARE_ == _CMA__HPPA
#  define cma__c_default_stack	cma__c_page_size * 15	/* Default stack */
#  define cma__c_default_guard	cma__c_page_size * 4	/* Default guard */
# else
#  define cma__c_default_stack	cma__c_page_size * 8	/* Default stack */
#  define cma__c_default_guard	cma__c_page_size * 2	/* Default guard */
# endif
#else
# if _CMA_OS_ == _CMA__UNIX
#  if _CMA_HARDWARE_ == _CMA__VAX
#   define cma__c_default_stack	6480
#  elif _CMA_HARDWARE_ == _CMA__MIPS
#   define cma__c_default_stack	12160
#  elif _CMA_HARDWARE_ == _CMA__ALPHA
#   define cma__c_default_stack	21120	/* 64 bit architecture! */
#  else
#   define cma__c_default_stack	cma__c_page_size * 16	/* Default stack */
#  endif
#  define cma__c_default_guard	cma__c_page_size * 4	/* Default guard */
# else					/* VMS */
#  if _CMA_HARDWARE_ == _CMA__VAX
#   define cma__c_default_stack	cma__c_page_size * 60	/* Default stack */
#  else
#   define cma__c_default_stack	11200
#  endif
#  define cma__c_default_guard	cma__c_page_size * 8	/* Default guard */
# endif
#endif

#if _CMA_VENDOR_ == _CMA__HP
# if _CMA_HARDWARE_ == _CMA__HPPA
#  define cma__c_chunk_count	96	/* This many chunks in an array */
# else
#  define cma__c_chunk_count	48	/* This many chunks in an array */
# endif
#else
# if _CMA_PLATFORM_ == _CMA__VAX_VMS
#  define cma__c_chunk_count	752	/* Allow for 8 default stacks */
# endif
# if _CMA_PLATFORM_ == _CMA__VAX_UNIX
#  define cma__c_chunk_count	188	/* Allow for 8 default stacks */
# endif
# if _CMA_PLATFORM_ == _CMA__MIPS_UNIX
#  define cma__c_chunk_count	94	/* Allow for 8 default stacks */
# endif
# if _CMA_HARDWARE_ == _CMA__ALPHA
#  define cma__c_chunk_count	94	/* Allow for 8 default stacks */
# endif
# if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
#  define cma__c_chunk_count	1458
# endif
# if _CMA_PLATFORM_ == _CMA__M68K_UNIX
#  define cma__c_chunk_count	384
# endif
#endif

/*
 * TYPEDEFS
 */

#ifndef __STDC__
struct CMA__T_INT_STACK;
#endif

typedef cma_t_natural	cma___t_index;	/* Type for chunk index */

typedef struct CMA___T_CLU_DESC {
    cma__t_list		list;		/* Queue element for cluster list */
    cma_t_integer	type;		/* Type of cluster */
    cma_t_address	stacks;
    cma_t_address	limit;
    } cma___t_clu_desc;

typedef union CMA___T_MAP_ENTRY {
    struct {
	cma__t_int_tcb	*tcb;		/* TCB associated with stack chunk */
	struct CMA__T_INT_STACK	*stack;	/* Stack desc. ass. with stack chunk */
	} mapped;
    struct {
	cma___t_index		size;	/* Number of chunks in block */
	cma___t_index		next;	/* Next free block */
	} free;
    } cma___t_map_entry;

/*
 * NOTE: It is VERY IMPORTANT that both cma___t_cluster and cma___t_bigstack
 * begin with the cma___t_clu_desc structure, as there is some code in the
 * stack manager that relies on being able to treat both as equivalent!
 */
typedef struct CMA___T_CLUSTER {
    cma___t_clu_desc	desc;		/* Describe this cluster */
    cma___t_map_entry	map[cma__c_chunk_count];	/* thread map */
    cma___t_index	free;		/* First free chunk index */
    } cma___t_cluster;

/*
 * NOTE: It is VERY IMPORTANT that both cma___t_cluster and cma___t_bigstack
 * begin with the cma___t_clu_desc structure, as there is some code in the
 * stack manager that relies on being able to treat both as equivalent!
 */
typedef struct CMA___T_BIGSTACK {
    cma___t_clu_desc	desc;		/* Describe this cluster */
    cma__t_int_tcb	*tcb;		/* TCB associated with stack */
    struct CMA__T_INT_STACK	*stack;	/* Stack desc. ass. with stack */
    cma_t_natural	size;		/* Size of big stack */
    cma_t_boolean	in_use;		/* Set if allocated */
    } cma___t_bigstack;

#if _CMA_PROTECT_MEMORY_
typedef struct CMA___T_INT_HOLE {
    cma__t_queue	link;		/* Link holes together */
    cma_t_boolean	protected;	/* Set when pages are protected */
    cma_t_address	first;		/* First protected byte */
    cma_t_address	last;		/* Last protected byte */
    } cma___t_int_hole;
#endif

typedef struct CMA__T_INT_STACK {
    cma__t_object	header;		/* Common header (sequence, type info */
    cma__t_int_attr	*attributes;	/* Backpointer to attr obj */
    cma___t_cluster	*cluster;	/* Stack's cluster */
    cma_t_address	stack_base;	/* base address of stack */
    cma_t_address	yellow_zone;	/* first address of yellow zone */
    cma_t_address	last_guard;	/* last address of guard pages */
    cma_t_natural	first_chunk;	/* First chunk allocated */
    cma_t_natural	chunk_count;	/* Count of chunks allocated */
    cma__t_int_tcb	*tcb;		/* TCB backpointer */
#if _CMA_PROTECT_MEMORY_
    cma___t_int_hole	hole;		/* Description of hole */
#endif
    } cma__t_int_stack;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_STACK_INT.H */
/*  *21   25-JAN-1993 11:34:20 BUTENHOF "Move reserve size to cma_stack.h" */
/*  *20    2-DEC-1992 13:29:10 BUTENHOF "Fix typo in stack size" */
/*  *19    8-SEP-1992 11:00:26 BUTENHOF "Update comments" */
/*  *18    2-SEP-1992 16:26:24 BUTENHOF "New logic for stack sizing" */
/*  *17   27-MAR-1992 18:05:41 SCALES "Fix VMS reserve size" */
/*  *16   19-MAR-1992 13:17:52 BUTENHOF "Cut stack size on DEC OSF/1" */
/*  *15   29-JAN-1992 14:37:30 BUTENHOF "Improve special cluster management" */
/*  *14   22-NOV-1991 13:38:33 BUTENHOF "Set a chunk count for Alpha" */
/*  *13   11-NOV-1991 11:56:49 BUTENHOF "Fix type of fields" */
/*  *12   14-OCT-1991 13:40:29 BUTENHOF "Unprotect guard pages on abort" */
/*  *11   18-JUL-1991 09:46:17 CURTIN "Added large stack support" */
/*  *10   10-JUN-1991 19:56:48 SCALES "Convert to stream format for ULTRIX build" */
/*  *9    10-JUN-1991 19:21:47 BUTENHOF "Fix the sccs headers" */
/*  *8    10-JUN-1991 18:23:55 SCALES "Add sccs headers for Ultrix" */
/*  *7     5-JUN-1991 17:31:39 BUTENHOF "Conditionalize vacuous defs" */
/*  *6    23-MAY-1991 16:39:26 CURTIN "Changed cluster sizes to handle 8 stacks" */
/*  *5    14-MAR-1991 13:45:54 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    12-MAR-1991 19:36:46 SCALES "Merge Apollo changes to CD4" */
/*  *3    25-FEB-1991 18:26:02 BUTENHOF "Fix up chunk size as variable" */
/*  *2    19-FEB-1991 17:17:29 CURTIN "Made cluster size larger, also uniform on our platforms" */
/*  *1    12-DEC-1990 21:53:29 BUTENHOF "Thread stacks" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_STACK_INT.H */

