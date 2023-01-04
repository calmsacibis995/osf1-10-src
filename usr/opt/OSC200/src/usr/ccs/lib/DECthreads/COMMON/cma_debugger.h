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
 *	@(#)$RCSfile: cma_debugger.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:45:42 $
 */
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	Debug structures
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	9 April 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	16 June 1992
 *		Implement this module (with exported function prototypes).
 *	002	Dave Butenhof	31 July 1992
 *		Add "handle" command to decode a user's DECthreads object
 *		handle.
 *	003	Dave Butenhof	30 April 1993
 *		Add globals
 *	004	Dave Butenhof	 4 May 1993
 *		It's nice to use cma_debug_cmd() to turn VPs on and off, but
 *		the "kernel is locked" warning can block and let other
 *		threads go. Don't give messages for cma_debug_cmd.
 *	005	Dave Butenhof	26 July 1993
 *		Add a "follow queue" command to aid in debugging an OSF/1
 *		problem.
 */

#ifndef CMA_DEBUGGER
#define CMA_DEBUGGER

/*
 *  INCLUDE FILES
 */

#include <cma.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

/*
 * GLOBAL DATA
 */

extern char	*cma__g_hardware;
extern char	*cma__g_os;

/*
 * FUNCTION PROTOTYPES
 */

extern void
cma__debug_format_handle _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char	*argv[]));

extern void
cma__debug_format_thread _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb,
	cma_t_integer	display_flags));

extern void
cma__debug_get_system _CMA_PROTOTYPE_ ((
	char		*buffer,
	int		length));

extern void
cma__debug_help _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char	*argv[]));

extern void
cma__debug_list_atts _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char	*argv[]));

extern void
cma__debug_list_cvs _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char		*argv[]));

extern void
cma__debug_list_mutexes _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char		*argv[]));

extern void
cma__debug_list_threads _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char		*argv[]));

extern cma_t_boolean
cma__debug_parse _CMA_PROTOTYPE_ ((
	char		*cmd,
	cma_t_boolean	locked));

extern void
cma__debug_queue _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char		*argv[]));

extern void
cma__debug_show _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char	*argv[]));

extern void
cma__debug_set_thread _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char	*argv[]));

extern cma_t_boolean
cma__debug_trylock _CMA_PROTOTYPE_ ((cma_t_boolean message));

extern void
cma__debug_versions _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char	*argv[]));

extern void
cma__debug_vp _CMA_PROTOTYPE_ ((
	cma_t_integer	argc,
	char	*argv[]));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEBUGGER.H */
/*  *11   26-JUL-1993 13:34:48 BUTENHOF "Add queue walker" */
/*  *10    4-MAY-1993 11:38:21 BUTENHOF "Don't type long message on debug_cmd" */
/*  *9    30-APR-1993 18:13:48 BUTENHOF "Add globals" */
/*  *8    31-JUL-1992 15:33:24 BUTENHOF "Add handle function" */
/*  *7    23-JUL-1992 17:08:34 KEANE "Make param types match what is acutaqlly declared in routines" */
/*  *6    10-JUL-1992 08:02:23 BUTENHOF "Move exported functions where they belong" */
/*  *5    24-MAR-1992 13:46:07 BUTENHOF "Put bugcheck output in file" */
/*  *4    10-JUN-1991 19:51:33 SCALES "Convert to stream format for ULTRIX build" */
/*  *3    10-JUN-1991 19:20:19 BUTENHOF "Fix the sccs headers" */
/*  *2    10-JUN-1991 18:19:08 SCALES "Add sccs headers for Ultrix" */
/*  *1    12-DEC-1990 21:43:48 BUTENHOF "Debug support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEBUGGER.H */
