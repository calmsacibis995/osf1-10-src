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
static char *rcsid = "@(#)$RCSfile: cmalib_seq.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:59:22 $";
#endif
/*
 *  Copyright (c) 1990, 1992 by
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
 *	Concert Multithread Architecture (CMA) Library services
 *
 *  ABSTRACT:
 *
 *	Routine to generate unique sequence numbers for CMA Library objects
 *	(Copied from CMA_SEQUENCE.C)
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	9 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	29 August 1990
 *		Fixed off-by-one in initialization: object numbers start at one.
 *		Change calls to CMA to pass structures by reference.
 *	002	Dave Butenhof	03 June 1992
 *		Modify for new build environment.
 */


/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_defs.h>
#include <cmalib_seq.h>


/*
 *  GLOBAL DATA
 */
cma_lib__t_sequence	cma_lib__g_sequence[cma_lib__c_obj_num];


/*
 *  LOCAL DATA
 */

/*
 *  LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Assign a unique sequence number for some type of object
 *
 *  FORMAL PARAMETERS:
 *
 *	control		Sequence control structure (cma_t_sequence)
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
 *	The sequence number of the new object
 *
 *  SIDE EFFECTS:
 *
 *	Increments number in sequence control structure
 */
extern cma_t_natural
cma_lib__assign_sequence
#ifdef _CMA_PROTO_
	(
	cma_lib__t_sequence	*control)	/* Sequence control block */
#else	/* no prototypes */
	(control)
	cma_lib__t_sequence	*control;	/* Sequence control block */
#endif	/* prototype */
    {
    cma_t_natural	seq;		/* Local copy of next sequence */


    cma_mutex_lock (&(control->mutex));
    seq = control->seq++;
    cma_mutex_unlock (&(control->mutex));
    return seq;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize a sequence generator control structure
 *
 *  FORMAL PARAMETERS:
 *
 *	control		Sequence control structure (cma_t_sequence)
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
cma_lib__init_sequence
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer   i;


    for (i = 1; i < cma_lib__c_obj_num; i++) {
	cma_mutex_create (&cma_lib__g_sequence[i].mutex, &cma_c_null);
	cma_lib__g_sequence[i].seq = 1;
	}
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_SEQ.C */
/*  *3     3-JUN-1992 06:16:56 BUTENHOF "Fix errors in update" */
/*  *2    29-AUG-1990 17:03:57 SCALES "Fix off-by-one in initialization" */
/*  *1    27-AUG-1990 02:15:50 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_SEQ.C */
