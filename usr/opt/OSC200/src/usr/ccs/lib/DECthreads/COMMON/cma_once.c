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
 * @(#)$RCSfile: cma_once.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/08/18 14:48:36 $
 */
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	Support client one-time initialization
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	19 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	31 August 1989
 *		Fix calls which ought to pass handle addresses.
 *	002	Dave Butenhof	12 October 1989
 *		Convert to use internal mutex operations.
 *	003	Dave Butenhof	1 December 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	004	Dave Butenhof	11 April 1990
 *		Catch possible errors from cma__alloc_mem while mutex is
 *		held, and unlock before reraising exception.
 *	005	Dave Butenhof	5 October 1990
 *		Add argument to client init routine!
 *	017	Dave Butenhof	25 October 1990
 *		Add name to mutexes created for one-time init.
 *	018	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	019	Paul Curtin	21 May 1991
 *		Added a queue to hold once block mutexes.
 *	020	Paul Curtin	24 May 1991
 *		Added a cma__reinit_once routine.
 *	021	Dave Butenhof	11 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	022	Dave Butenhof	 4 June 1993
 *		Update the names of once block mutexes, and remove the queue
 *		of once block mutexes (which was never used anyway).
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_once.h>
#include <cma_mutex.h>
#include <cma_attr.h>
#include <cma_queue.h>
#include <cma_vm.h>

/*
 *  GLOBAL DATA
 */

/*
 *  LOCAL DATA
 */

static cma__t_int_mutex   *cma___g_init_mutex;

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize the control block if necessary.  Determine whether the
 *	initialization function has already been called; if not, call it.
 *
 *  FORMAL PARAMETERS:
 *
 *	init_block	control block for initialization
 *
 *	init_routine	initialization routine
 *
 *  IMPLICIT INPUTS:
 *
 *	cma___g_init_mutex	global init. control mutex
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
 *	· initialization routine is called if necessary.
 *
 *	· a mutex is created in the control block if not already done.
 */
void
cma_once
#ifdef _CMA_PROTO_
	(
	cma_t_once		*init_block,
	cma_t_init_routine	init_routine,
	cma_t_address		arg)
#else	/* no prototypes */
	(init_block, init_routine, arg)
	cma_t_once		*init_block;
	cma_t_init_routine	init_routine;
	cma_t_address		arg;
#endif	/* prototype */
    {
    /*
     * Cast the external init block to the internal format
     */
    cma__t_int_once *init_record = (cma__t_int_once *)init_block;


    if (init_record->flag != cma__c_once_inited) {
	cma__int_lock (cma___g_init_mutex);

	/*
	 * Make sure it's a valid once block... check the mbz field and the
	 * flag.
	 */
	if ((init_record->mbz != 0)
		|| ((int)init_record->flag < (int)cma__c_once_uninit)
		|| ((int)init_record->flag > (int)cma__c_once_inited)
		) {
	    cma__int_unlock (cma___g_init_mutex);
	    return;
	    }

	/*
	 * If the control block hasn't been initialized yet (flag ==
	 * cma__c_once_uninit), then lock the global init control mutex, and
	 * create a mutex for this control block.  Set the flag to
	 * cma__c_once_initing, which indicates the block is ready for use.
	 */
	if (init_record->flag == cma__c_once_uninit) {
	    init_record->mutex = cma__get_mutex (&cma__g_def_attr);

	    if ((cma_t_address)init_record->mutex == cma_c_null_ptr) {
		cma__int_unlock (cma___g_init_mutex);
		cma__error (exc_s_insfmem);
		}

	    cma__obj_set_owner (init_record->mutex, (cma_t_integer)init_record);
	    cma__obj_set_name (init_record->mutex, "<once block@0x%lx>");
	    init_record->flag = cma__c_once_initing;
	    }

	cma__int_unlock (cma___g_init_mutex);
	}
    
    /*
     * Now lock the control block mutex, and see whether some other thread
     * has already done the initialization for us... if not, call the init
     * routine and set the flag to cma__c_once_inited (done).
     */
    cma__int_lock (init_record->mutex);

    if (init_record->flag == cma__c_once_initing) {
	/*
	 * Call the client's init routine.  If it raises an exception, unlock
	 * the control block and reraise the exception (without declaring the
	 * initialization complete).
	 */
	TRY {
	    (init_routine) (arg);
	    }
	CATCH_ALL {
	    cma__int_unlock (init_record->mutex);
	    RERAISE;
	    }
	ENDTRY

	init_record->flag = cma__c_once_inited;
	}

    cma__int_unlock (init_record->mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize module static data.
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
 *	initialize static data
 */
extern void
cma__init_once
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma___g_init_mutex = cma__get_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma___g_init_mutex, "one time init");
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Perform work prior to and after fork(), depending on flag.
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
cma__reinit_once
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
    {
    
    if (flag == cma__c_reinit_prefork_lock) {
	cma__int_lock(cma___g_init_mutex);
	}
    else if (flag == cma__c_reinit_postfork_unlock) {
	cma__int_unlock(cma___g_init_mutex);
	}

    }

/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ONCE.C */
/*  *6     4-JUN-1993 11:29:10 BUTENHOF "Name user once mutexes" */
/*  *5    18-FEB-1992 15:29:40 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *4    10-JUN-1991 18:22:34 SCALES "Add sccs headers for Ultrix" */
/*  *3     5-JUN-1991 16:13:40 CURTIN "fork work" */
/*  *2    14-DEC-1990 00:55:48 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:47:49 BUTENHOF "Client one-time init" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ONCE.C */
