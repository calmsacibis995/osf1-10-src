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
 *	@(#)$RCSfile: cma_message.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/08/06 17:38:00 $
 */

/*
 * FACILITY:
 *
 *	CMA services
 *
 * ABSTRACT:
 *
 *	Header for interface to the message catalog services.
 *
 * AUTHORS:
 *
 *	Dave Butenhof
 *
 * CREATION DATE:
 *
 *	28 May 1991
 *
 * MODIFICATION HISTORY:
 * 
 *	001	Paul Curtin	12 June 1991
 *		DECthreads integration work.
 *	002	Dave Butenhof	13 June 1991
 *		Add SCCS information, move to COMMON domain.
 *	003	Paul Curtin	20 August 1991
 *		Conditionalized out the inclusion of stdio.h on VMS.
 *	004	Dave Butenhof	20 November 1991
 *		Major rewrite: instead of using the old cma__message_*printf
 *		routines (we only called cma__message_fprintf anyway, and
 *		only from one place in exc_handling.c), replace with a single
 *		function, cma__error_inq_text, which returns the text of any
 *		dce message.
 */
#ifndef CMA_MESSAGE
#define CMA_MESSAGE

/*
 * INCLUDE FILES
 */

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

/*
 * GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

extern void
cma__error_inq_text _CMA_PROTOTYPE_ ((
	unsigned long	status_to_convert,
	unsigned char	*error_text,
	int		*status));

#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_MESSAGE.H */
/*  *4    22-NOV-1991 11:56:42 BUTENHOF "Integrate dce message formatting" */
/*  *3    21-AUG-1991 16:43:22 CURTIN "Removed VMS include of stdio.h" */
/*  *2    13-JUN-1991 19:33:16 BUTENHOF "fix history" */
/*  *1    13-JUN-1991 19:29:14 BUTENHOF "Message catalog support" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_MESSAGE.H */
