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
 * @(#)$RCSfile: cma_tis_sup.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/11 22:02:53 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for DECthreads 'thread independent services' internal
 *	interfaces.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	6 May 1993
 *
 *  MODIFICATION HISTORY:
 *
 */

#ifndef CMA_TIS_SUP
#define CMA_TIS_SUP

/*
 *  INCLUDE FILES
 */

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

extern cma_t_boolean	cma__g_tis_disable;

/*
 * INTERNAL INTERFACES
 */

extern void
cma__init_tis _CMA_PROTOTYPE_ ((void));	/* Initialize interface */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIS_SUP.H */
/*  *1     7-MAY-1993 06:21:38 BUTENHOF "Header file for DECthreads TIS" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIS_SUP.H */
