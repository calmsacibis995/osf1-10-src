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
 * @(#)$RCSfile: cmalib_aq_op.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:18 $
 */
/*
 *  Copyright (c) 1990, 1991 by
 *  Digital Equipment Corporation, Maynard Massachusetts.
 *  All rights reserved.
 *
 *  This software is furnished under a license and may be used and  copied
 *  only  in  accordance  with  the  terms  of  such  license and with the
 *  inclusion of the above copyright notice.  This software or  any  other
 *  copies  thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of  the  software  is  hereby
 *  transferred.
 *
 *  The information in this software is subject to change  without  notice
 *  and  should  not  be  construed  as  a commitment by DIGITAL Equipment
 *  Corporation.
 *
 *  DIGITAL assumes no responsibility for the use or  reliability  of  its
 *  software on equipment which is not supplied by DIGITAL.
 */

/*
 *  FACILITY:
 *
 *	CMA Library services
 *
 *  ABSTRACT:
 *
 *	Header file for hardware-independent Unix (atomic) queue functions
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	26 August 1990
 *
 *  MODIFICATION HISTORY:
 *	001	Paul Curtin	22 March 1991
 *		Added some interrupt enable/disable macros.
 *	002	Paul Curtin	25 March 1991
 *		Placed global lock around enable/disable macros.
 *	003	Paul Curtin	26 March 1991
 *		Added interrupt level interrupt enable/disable macros.
 *	004	Paul Curtin	28 March 1991
 *		Removed global locks from interrupt enable.
 *	005	Dave Butenhof	23 May 1991
 *		Changes to compile on new MIPS C
 */


#ifndef CMALIB_ATOMICQ_UNIX
#define CMALIB_ATOMICQ_UNIX

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_queue.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * Macro to initialize an atomic queue
 */
#define cma_lib__aq_init(queue)	(	\
	cma_lib__queue_init (&((queue)->items)),	\
	cma_lib__queue_init (&((queue)->free)),		\
	(queue)->deleted = cma_c_false)

/*
 * Disable interrupts (signals)
 * FIX-ME: place these interrupt macros in host.h when there is one.
 */
# define cma_lib__interrupt_disable(sig_mask)	{ \
    cma_lock_global ();	\
    (sigprocmask (SIG_SETMASK, &cma_lib__g_sig_block_mask, &sig_mask) == -1 ? \
        (RAISE (cma_e_assertion), 0) : \
        0); \
    cma_unlock_global (); \
    }

/*
 * Enable interrupts (signals)
 */
# define cma_lib__interrupt_enable(sig_mask)	{ \
    (sigprocmask (SIG_SETMASK, &sig_mask, (sigset_t *)cma_c_null_ptr) == -1 ? \
        (RAISE (cma_e_assertion), 0) : \
        0); \
    }

/*
 * Disable interrupts (signals) at SIGNAL level 
 */
# define cma_lib__interrupt_disable_int(sig_mask) { \
    (sigprocmask (SIG_SETMASK, &cma_lib__g_sig_block_mask, &sig_mask) == -1 ? \
        (RAISE (cma_e_assertion), 0) : \
        0); \
    }

/*
 * Enable interrupts (signals) at SIGNAL level
 */
# define cma_lib__interrupt_enable_int(sig_mask) { \
    (sigprocmask (SIG_SETMASK, &sig_mask, (sigset_t *)cma_c_null_ptr) == -1 ? \
        (RAISE (cma_e_assertion), 0) : \
        0); \
    }

/*
 * TYPEDEFS
 */

typedef cma_lib__t_queue    cma_lib__t_aq_link;

typedef struct CMA_LIB__T_AQ_HEADER {
	cma_lib__t_queue    items;
	cma_lib__t_queue    free;
	cma_t_boolean	    deleted;
	} cma_lib__t_aq_header;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */
extern void
cma_lib__aq_augment _CMA_PROTOTYPE_ ((
	cma_lib__t_aq_header	*queue_head,
	cma_t_integer		node_cnt));

extern void
cma_lib__aq_disperse _CMA_PROTOTYPE_ ((
	cma_lib__t_aq_header	*queue_head));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_OP.H */
/*  *10   23-MAY-1991 13:46:49 BUTENHOF "Satisfy new MIPS C compiler" */
/*  *9    28-MAR-1991 17:24:28 CURTIN "removed global locking on enable" */
/*  *8    26-MAR-1991 14:17:02 CURTIN "fixed a name" */
/*  *7    26-MAR-1991 14:12:43 CURTIN "added _int interrupt macros" */
/*  *6    25-MAR-1991 13:47:08 CURTIN "added use of global lock" */
/*  *5    22-MAR-1991 13:27:38 CURTIN "name change" */
/*  *4    22-MAR-1991 12:33:22 CURTIN "quick name fix" */
/*  *3    22-MAR-1991 12:08:48 CURTIN "added a couple of macros" */
/*  *2    29-AUG-1990 17:03:12 SCALES "Convert to stream format" */
/*  *1    27-AUG-1990 02:15:08 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_OP.H */
