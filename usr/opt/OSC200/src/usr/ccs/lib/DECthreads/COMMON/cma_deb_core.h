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
 *	@(#)$RCSfile: cma_deb_core.h,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/05/25 19:58:14 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	This file defines the internal interface to the core of CMA 
 * 	debugging services. (The client interface to debugging services
 *	is provided by cma_debug_client.h).
 *
 *
 *  AUTHORS:
 *
 *	Bob Conti
 *
 *  CREATION DATE:
 *
 *	13 September 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Bob Conti	13 September 1990
 *		Create module
 *	002	Bob Conti	1 October 1990
 *		Move known object queues here
 *	003	Bob Conti	11 October 1990
 *		Add init entry 
 *	004	Paul Curtin	24 May 1991
 *		Added a prototype for cma__reinit_debug.
 *	005	Paul Curtin	10 December 1991
 *		Added current_sp as an argument to cma__deb_get.
 *	006	Dave Butenhof	01 September 1992
 *		Update OSF/1 SCCS header.
 *	007	Dave Butenhof	15 September 1992 (Second Anniversary!)
 *		Enhance the known object structure to include the sequence
 *		number (avoiding a separate lock to use a "sequence object").
 *	008	Dave Butenhof	 3 May 1993
 *		Add pointer to the last "live" known thread (preceding
 *		terminated -- zombie -- entries).
 *	009	Dave Butenhof	14 May 1993
 *		Track first dead thread rather than last living thread
 *		(cma__g_last_thread is now cma__g_dead_zone) -- otherwise,
 *		destroy_tcb and free_tcb would have to enter kernel and check
 *		pointer to avoid moving last_thread onto cache or pool list!
 */

#ifndef CMA_DEB_CORE
#define CMA_DEB_CORE

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_mutex.h>
#include <cma_queue.h>
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */


/*
 * TYPEDEFS
 */

/*
 * Type defing the format of known object lists
 */
typedef struct CMA__T_KNOWN_OBJECT {
    cma__t_queue	queue;		/* Queue header for known objects */
    cma__t_int_mutex	*mutex;		/* Mutex to control access to queue */
    cma_t_integer	sequence;	/* Sequence number for object type */
    } cma__t_known_object;

/*
 * Type defining the registration for one debug client (e.g. Ada)
 */
typedef struct CMA__T_DEB_REGISTRY {
    cma_t_address	entry;		/* Client's debug entry point */
    cma_t_key		key;		/* Client's context key */
    cma_t_integer	fac;		/* Client's debug facility number */
    cma_t_boolean 	has_prolog;	/* Client's TCBs have std prolog */
    } cma__t_deb_registry;

#define cma__c_deb_max_clients	10

/* 
 * Type defining the global debugging state for all threads.
 */
typedef struct CMA__T_DEBUG_STATE {
    /* 
     * The following flag is set if changes were made while in the
     * debugger that may make the ready lists inconsistent.  For 
     * example, if a thread priority is changed in the debugger, the
     * thread is not moved between queues.  Making things consistent
     * is deferred to when the dispatcher is next invoked -- which we
     * try to make very soon.
     */
    cma_t_boolean 	is_inconsistency;   /* Ready lists are inconsistent */
    cma_t_boolean 	events_enabled;	    /* Set if _any_ event is enabled */
    cma_t_boolean	flags[cma__c_debevt__dim];	/* Enabled events */
    cma__t_int_tcb	*next_to_run;	    /* TCB of thread to run next */ 
    cma__t_int_mutex	*mutex;		    /* Mutex for registering clients */
    cma_t_integer	client_count;	    /* Count of debug clients */
    cma__t_deb_registry	clients[cma__c_deb_max_clients+1];	/* Client list */
    } cma__t_debug_state;


/* 
 * Routine that will symbolize an address and print it.
 */
typedef void (*cma__t_print_symbol) _CMA_PROTOTYPE_ ((
	cma_t_address	the_address));

/*
 *  GLOBAL DATA
 */

/* 
 * Variable holding the global debugging state 
 *
 * (This is primarily written by the debugger interface and read
 * by the thread dispatcher).
 */
extern cma__t_debug_state	cma__g_debug_state;

/* 
 * Known object queues
 */
extern cma__t_known_object	cma__g_known_atts;
extern cma__t_known_object	cma__g_known_cvs;
extern cma__t_known_object	cma__g_known_mutexes;
extern cma__t_known_object	cma__g_known_threads;

extern cma__t_queue		*cma__g_dead_zone;

/*
 * INTERNAL INTERFACES
 */

/*
 * Get information in debugger context.
 */
extern void
cma__deb_get	_CMA_PROTOTYPE_ ((
	cma__t_int_tcb		*tcb, 
	cma_t_debug_get		item_requested,
	cma_t_address		buffer,
	cma_t_integer		buffer_size,
	cma_t_integer		current_sp));

/*
 * Set information while in debugger context
 */
extern void
cma__deb_set _CMA_PROTOTYPE_ ((
	cma__t_int_tcb		*tcb, 
	cma_t_debug_set		item_to_be_set,
	cma_t_address		buffer,
	cma_t_integer		buffer_size));

extern void
cma__init_debug	_CMA_PROTOTYPE_ ((void));

extern void
cma__reinit_debug _CMA_PROTOTYPE_ ((
	cma_t_integer		flag));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEB_CORE.H */
/*  *10   14-MAY-1993 15:55:08 BUTENHOF "Change 'last_thread' to 'dead_zone'" */
/*  *9     3-MAY-1993 13:44:17 BUTENHOF "Merge zombies with known threads" */
/*  *8    15-SEP-1992 13:49:33 BUTENHOF "Change sequencing" */
/*  *7     2-SEP-1992 16:24:40 BUTENHOF "Remove semaphore references" */
/*  *6    10-DEC-1991 15:18:47 CURTIN "cma__deb_get now takes SP as an argument" */
/*  *5    10-JUN-1991 19:51:59 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    10-JUN-1991 19:20:25 BUTENHOF "Fix the sccs headers" */
/*  *3    10-JUN-1991 18:19:47 SCALES "Add sccs headers for Ultrix" */
/*  *2    24-MAY-1991 16:46:07 CURTIN "Added a reinit routine" */
/*  *1    12-DEC-1990 21:43:56 BUTENHOF "Debug support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEB_CORE.H */
