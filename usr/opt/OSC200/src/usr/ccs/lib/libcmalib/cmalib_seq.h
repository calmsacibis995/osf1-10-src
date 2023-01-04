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
 * @(#)$RCSfile: cmalib_seq.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:59:27 $
 */
/*
 *  Copyright (c) 1990 by
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
 *	Header file for library-object sequence generator functions
 *	(Copied from CMA_SEQUENCE.H)
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
 */


#ifndef CMALIB_SEQUENCE
#define CMALIB_SEQUENCE

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_defs.h>


/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */
typedef struct CMA_LIB__T_SEQUENCE {
    cma_t_mutex	    mutex;	/* Serialize access to counter */
    cma_t_natural   seq;	/* Sequence number for object */
    } cma_lib__t_sequence;

/*
 *  GLOBAL DATA
 */
extern cma_lib__t_sequence	cma_lib__g_sequence[cma_lib__c_obj_num];

/*
 * INTERNAL INTERFACES
 */

extern cma_t_natural
cma_lib__assign_sequence _CMA_PROTOTYPE_ ((
	cma_lib__t_sequence	*control));	/* Sequence control block */

extern void
cma_lib__init_sequence _CMA_PROTOTYPE_ ((void));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_SEQ.H */
/*  *1    27-AUG-1990 02:15:53 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_SEQ.H */
