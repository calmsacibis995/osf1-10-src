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
 * @(#)$RCSfile: cmalib_init.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:59:07 $
 */
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
 *	CMA Library services
 *
 *  ABSTRACT:
 *
 *	Header file for CMA Library initialization
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
 *	001	Paul Curtin    22-March-1991
 *		Added an extern.
 *	002	Paul Curtin    25-March-1991
 *		Made extern conditional on unix.
 */

#ifndef CMALIB_INIT
#define CMALIB_INIT

/*
 *  INCLUDE FILES
 */

#include <signal.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma_lib__c_env_maxattr	0
#define cma_lib__c_env_minattr	1
#define cma_lib__c_env_maxqueue	2
#define	cma_lib__c_env_minqueue	3

#define cma_lib__c_env_count	4

/*
 * TYPEDEFS
 */

typedef struct CMA_LIB__T_ENV {
    char		*name;		/* Name of environment variable */
    cma_t_integer	value;		/* Numeric value of the variable */
    } cma_lib__t_env;

/*
 *  GLOBAL DATA
 */

extern cma_lib__t_env	cma_lib__g_env[cma_lib__c_env_count];

#ifdef unix
 extern sigset_t		cma_lib__g_sig_block_mask;
#endif

/*
 * INTERNAL INTERFACES
 */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_INIT.H */
/*  *4     1-JUN-1992 14:40:00 BUTENHOF "Modify for new build environment" */
/*  *3    25-MAR-1991 11:15:14 CURTIN "made an extern conditional on unix" */
/*  *2    22-MAR-1991 13:28:28 CURTIN "added an extern" */
/*  *1    27-AUG-1990 02:15:45 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_INIT.H */
