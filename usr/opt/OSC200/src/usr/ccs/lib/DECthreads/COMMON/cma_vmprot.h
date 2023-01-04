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
 *	@(#)$RCSfile: cma_vmprot.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:59:19 $
 */
/*
 *  Copyright (c) 1989, 1990 by
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
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for VM protection services
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	23 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 */

#ifndef CMA_VMPROT
#define CMA_VMPROT

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

/*
 * INTERNAL INTERFACES
 */

extern void
cma__set_noaccess _CMA_PROTOTYPE_ ((
	cma_t_address	low_address,	/* Lowest address to change */
	cma_t_address	high_address));	/* Highest address to change */

extern void
cma__set_writable _CMA_PROTOTYPE_ ((
	cma_t_address	low_address,	/* Lowest address to change */
	cma_t_address	high_address));	/* Highest address to change */

#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_VMPROT.H */
/*  *4    10-JUN-1991 19:58:24 SCALES "Convert to stream format for ULTRIX build" */
/*  *3    10-JUN-1991 19:22:28 BUTENHOF "Fix the sccs headers" */
/*  *2    10-JUN-1991 18:25:20 SCALES "Add sccs headers for Ultrix" */
/*  *1    12-DEC-1990 21:56:01 BUTENHOF "VM management" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_VMPROT.H */
