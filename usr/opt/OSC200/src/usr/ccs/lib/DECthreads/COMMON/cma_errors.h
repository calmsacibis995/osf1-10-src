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
 *	@(#)$RCSfile: cma_errors.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:31:48 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	This module is the interface between CMA services and 
 * 	the platform-specific error reporting mechanism.
 *
 *  AUTHORS:
 *
 *	Bob Conti
 *
 *  CREATION DATE:
 *
 *	6 October 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	17 October 1989
 *		Corrected case of #include filenames
 *	002	Bob Conti	10 December 1989
 *		Move cma__unimplemented here from cma_exception.c
 *	003	Dave Butenhof	14 December 1989
 *		Add cma__report_error to output messages for exceptions.
 *	004	Dave Butenhof & Bob Conti	15 December 1989
 *		Remove cma__report_error to exception package & rename to
 *		cma__exc_report.
 *	005	Dave Butenhof	01 May 1991
 *		Add string argument to cma__bugcheck() (written out before
 *		raising exception).
 *	006	Dave Butenhof	24 March 1992
 *		Enhance cma__bugcheck() prototype to take printf argument
 *		list.
 *	007	Dave Butenhof	28 January 1993
 *		Add cma__g_bugchecked extern.
 *	008	Dave Butenhof	26 February 1993
 *		Fix type mismatch in cma__error (should be cma_t_status, was
 *		int -- on OSF/1 AXP, long != int).
 */

#ifndef CMA_ERRORS
#define CMA_ERRORS

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

extern cma_t_boolean	cma__g_bugchecked;

/*
 * INTERNAL INTERFACES
 */

/*
 * The cma__bugcheck function will print information to stderr (or sys$error
 * on VMS), and more extensive information to the file cma_dump.log in the
 * current working directory.
 */
extern void				/* Internal CMA coding error */
cma__bugcheck _CMA_PROTOTYPE_ ((
	char		*text,
	...));

extern void
cma__error _CMA_PROTOTYPE_ ((		/* Raise CMA error exception */
	cma_t_status	code));

extern void				/* Raise "cma_e_unimp" exception */
cma__unimplemented ();

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ERRORS.H */
/*  *9    26-FEB-1993 10:06:52 BUTENHOF "Fix AXP compilation error" */
/*  *8    28-JAN-1993 14:42:13 BUTENHOF "Turn TIS locking off on bugcheck" */
/*  *7    24-MAR-1992 13:46:31 BUTENHOF "Put bugcheck output in file" */
/*  *6    22-NOV-1991 11:56:05 BUTENHOF "Remove unnecessary includes" */
/*  *5    10-JUN-1991 19:53:08 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    10-JUN-1991 19:20:47 BUTENHOF "Fix the sccs headers" */
/*  *3    10-JUN-1991 18:21:47 SCALES "Add sccs headers for Ultrix" */
/*  *2     2-MAY-1991 13:58:18 BUTENHOF "Add string argument to cma__bugcheck()" */
/*  *1    12-DEC-1990 21:45:29 BUTENHOF "Error handling" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ERRORS.H */
