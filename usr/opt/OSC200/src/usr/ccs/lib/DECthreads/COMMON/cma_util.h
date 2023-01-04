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
 *	@(#)$RCSfile: cma_util.h,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/12/10 18:22:34 $
 */

/*
 *  FACILITY:
 *
 *	CMA internal services
 *
 *  ABSTRACT:
 *
 *	Header file for CMA internal UTIL operations
 *
 *  AUTHORS:
 *
 *	Paul Curtin
 *
 *  CREATION DATE:
 *
 *	1 August 1990
 *
 *  MODIFICATION HISTORY:
 * 
 *	001	Paul Curtin	14 September 1990
 *		Added cma__ftime function.
 *	002	Bob Conti	6 October 1990
 *		Added cma__putint_5, cma__putint_10, cma__puthex_8, 
 *		and cma__set_eol_routine.
 *	003	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	004	Dave Butenhof	7 March 1991
 *		Remove cma__strtok function, which is no longer used.
 *	005	Dave Butenhof	27 March 1991
 *		Add conditional cma__trace function.
 *	006	Dave Butenhof	12 April 1991
 *		Move cma__trace definition to cma_defs.h (even though the
 *		code is in cma_util.c) to make it more accessible.
 *	007	Dave Butenhof	24 April 1991
 *		Add cma__init_trace to do more sophisticated trace init.
 *	008	Paul Curtin	20 August 1991
 *		Conditionalized out the include of stdio.h on VMS.
 *	009	Dave Butenhof	18 December 1991
 *		Change interface to cma__getenv() -- it's incorrect, since it
 *		returns address of stack buffer; but while I'm fixing it, I
 *		might as well clean it up and make it reentrant.
 *	010	Dave Butenhof	10 March 1992
 *		Now that we're using gettimeofday() instead of ftime() on
 *		UNIX, change cma__ftime() to cma__gettimeval() [which is like
 *		gettimeofday() except it doesn't return the timezone
 *		information that we don't need anyway].
 *	011	Dave Butenhof	23 March 1992
 *		Separate file open logic from init_trace into a new
 *		cma__int_fopen() that can be used for bugcheck output, too.
 *		Also add cma__putformat to do formatted write using debug
 *		output indirection.
 *	012	Dave Butenhof	30 July 1992
 *		Change "cma__puthex_8" to cma__puthex_long; it'll do long int
 *		formatting (whether 16, 32, or 64 bits).
 *	013	Dave Butenhof	31 July 1992
 *		Add cma__strtol function.
 *	014	Dave Butenhof	14 September 1992
 *		Return length of formatted strings.
 */

#ifndef	CMA_UTIL
#define CMA_UTIL

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_attr.h>
#include <cma_defs.h>

#if _CMA_OS_ == _CMA__VMS
# include <cma_rms.h>
#endif

#if _CMA_VENDOR_ == _CMA__SUN
# include <sys/time.h>
#else
# include <time.h>
#endif

#if _CMA_OS_ == _CMA__UNIX
# include <stdio.h>
#endif

/*
 * CONSTANTS AND MACROS
 */

#define cma__c_buffer_size  256		    /* Size of output buffer	    */

/*
 * TYPEDEFS
 */

/* 
 * Alternate eol routine
 */
typedef void		(*cma__t_eol_routine) _CMA_PROTOTYPE_ ((
	char		*buffer));

#if _CMA_OS_ == _CMA__VMS
 typedef struct CMA__T_VMSFILE {
    struct RAB	rab;
    struct FAB	fab;
    } cma__t_vmsfile, 	*cma__t_file;
#else
 typedef FILE		*cma__t_file;
#endif

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

extern void
cma__abort _CMA_PROTOTYPE_ ((void));

extern cma_t_integer
cma__atol _CMA_PROTOTYPE_ ((
	char		*string));

extern cma_t_integer
cma__atoi _CMA_PROTOTYPE_ ((
	char		*string));

extern char *
cma__getenv _CMA_PROTOTYPE_ ((
	char		*name,
	char		*buffer,
	int		size));

extern int
cma__gettimespec _CMA_PROTOTYPE_ ((
	struct timespec	*time));

extern cma__t_file
cma__int_fopen _CMA_PROTOTYPE_ ((
	char	*file,
	char	*access));

#ifndef NDEBUG
extern void
cma__init_trace _CMA_PROTOTYPE_ ((
	char		*trace_env));
#endif

extern char *
cma__memcpy _CMA_PROTOTYPE_ ((
	char		*s1,
	char		*s2,
	cma_t_integer	size));
	
extern char *
cma__memset _CMA_PROTOTYPE_ ((
	char		*s,
	cma_t_integer	value,
	cma_t_integer	size));

extern int
cma__putformat _CMA_PROTOTYPE_ ((
	char		*buffer,
	char		*format,
	...));

extern int
cma__putstring _CMA_PROTOTYPE_ ((
	char		*buffer,
	char		*string));

extern int
cma__putint _CMA_PROTOTYPE_ ((
	char		*buffer,
	cma_t_integer	number));

extern int
cma__putint_5 _CMA_PROTOTYPE_ ((
	char		*buffer,
	cma_t_integer	number));

extern int
cma__putint_10 _CMA_PROTOTYPE_ ((
	char		*buffer,
	cma_t_integer	number));

extern int
cma__puthex _CMA_PROTOTYPE_ ((
	char		*buffer,
	cma_t_integer	number));

extern int
cma__puthex_long _CMA_PROTOTYPE_ ((
	char		*buffer,
	cma_t_integer	number));

extern void
cma__puteol _CMA_PROTOTYPE_ ((
	char		*buffer));

extern void
cma__set_eol_routine _CMA_PROTOTYPE_ ((
	cma__t_eol_routine	new_eol_routine,
	cma__t_eol_routine	*prior_eol_routine));

extern cma_t_integer
cma__strlen _CMA_PROTOTYPE_ ((
	char		*s));

extern int
cma__strncmp _CMA_PROTOTYPE_ ((
	char		*str1,
	char		*str2,
	cma_t_integer	length));

extern unsigned long int
cma__strtoul _CMA_PROTOTYPE_ ((
	_CMA_CONST_ char	*string,
	char			**endptr,
	int			in_base));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_UTIL.H */
/*  *15   15-SEP-1992 13:51:02 BUTENHOF "Return length of formatted output" */
/*  *14   31-JUL-1992 15:31:52 BUTENHOF "Add cma__puthex_addr" */
/*  *13   24-MAR-1992 13:46:52 BUTENHOF "Put bugcheck output in file" */
/*  *12   10-MAR-1992 16:28:15 BUTENHOF "Change ftime to gettimeofday" */
/*  *11   19-DEC-1991 13:08:45 BUTENHOF "Change interface to cma__getenv" */
/*  *10   21-AUG-1991 16:45:50 CURTIN "Removed VMS include of stdio.h" */
/*  *9    10-JUN-1991 19:57:49 SCALES "Convert to stream format for ULTRIX build" */
/*  *8    10-JUN-1991 19:22:18 BUTENHOF "Fix the sccs headers" */
/*  *7    10-JUN-1991 18:25:03 SCALES "Add sccs headers for Ultrix" */
/*  *6     2-MAY-1991 13:59:48 BUTENHOF "Add util init" */
/*  *5    12-APR-1991 23:37:32 BUTENHOF "Move trace to cma_defs.h" */
/*  *4     1-APR-1991 18:09:55 BUTENHOF "Add trace function" */
/*  *3     8-MAR-1991 18:49:21 BUTENHOF "Don't need strtok anymore" */
/*  *2    14-DEC-1990 00:56:08 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:55:22 BUTENHOF "Utilities" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_UTIL.H */
