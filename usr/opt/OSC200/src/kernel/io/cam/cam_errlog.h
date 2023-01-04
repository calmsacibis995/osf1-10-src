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
 * @(#)$RCSfile: cam_errlog.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 13:40:07 $
 */
#ifndef _CAM_ERRLOG_
#define _CAM_ERRLOG_

/*
 *		"@(#)cam_errlog.h	4.3	(ULTRIX)	11/19/91"
 */

/************************************************************************
 *									*
 *			Copyright (c) 199x by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* ---------------------------------------------------------------------- */

/* cam_error.h		Version 1.07			Nov. 19, 1991 */

/*
    This file contains the CAM error logging macros.

Modification History

	Version	  Date		Who	Reason

	1.00	12/04/90	jag	Created for use in the CAM modules.
	1.01	12/04/90	jag	Added a check for lint in the macro.
	1.02	12/10/90	jag	Added an "_" to the macro name.  Moved
					PARGS to the end of the macro args.
	1.03	08/01/91	dallas	Redefined the Args for cam_error
	1.04	09/13/91	dallas	Fixed CAM_ERROR for lint.
	1.05	11/13/91	jag	Changed the MACRO labels for convention.
					Removed the CAMERRLOG define, each
					source file will have to define it.
	1.06	11/18/91	jag	Added a NULL check around MESGSTR if
					the Macro is disabled.
	1.07	11/19/91	jag	Redid the Macro if CAMERROR is not
					defined.  It is now a printf with
					both the func and msg strings.
*/

/* ---------------------------------------------------------------------- */

/*
This Macro is an attempt to present a consistant error logging
interface to the various modules within the CAM subsystem.  Using this
macro will allow all the places within the modules that need to report
and log error information to use the same macro call and arguments.  It
was decided to allow the individual modules to contain their own error
logging routine specific to that module.  This local error logging
routine is called via the macro through the static, per source file,
"(*local_errorlog)()" pointer.  The reason for this indirection is to
allow the same macro to be used within the modules.  The macro will
always call the local error logging routine though the local pointer.

All the source files will need to have the following declaration of the 
error log routine pointer:

static void (*local_errorlog)();

Within the initiailization code for that module or as part of the
initialized data the pointer is loaded with the local handler address:

extern void sx_errorlog();
static void (*local_errlog)() = sx_errorlog; 

Within common modules it is possible to have the local pointer contain the
error logger handler from the another module. 

Currently the arguments to the macro contain different types of
information.  By convention the first three arguments in the macro are
pre-defined.  The first argument, FUNC, is the function name string.
The second argument, MSGSTR, is the message string from the fuction
that will be sent to the error logger.   This second argument also
unique in that if the macro is "undefined" the message string is
reported to the console.  The third argument, EFLAGS, contains error
flags for the local handler.  The remaining arguments, ARG4 - ARG6, are
local parameters that have meaning between the function and the local
error handler.

A primary reason for the use of this macro is to keep the reported
message string with the module code that reported it to begin with.
*/

#if defined(CAMERRLOG) && !defined(lint)
#   define CAM_ERROR( FUNC, MSGSTR, EFLAGS, ARG4, ARG5, ARG6)            \
    {                                                                    \
	/* VARARGS */                                                    \
	(void)(*local_errlog)( FUNC, MSGSTR, EFLAGS, ARG4, ARG5, ARG6 ); \
    }
#else /*  CAMERRLOG and not lint */ 
#   define CAM_ERROR( FUNC, MSGSTR, EFLAGS, ARG4, ARG5, ARG6 )           \
    {                                                                    \
	/* VARARGS */                                                    \
	printf( "%s: %s\n",                                              \
	    (((FUNC)   != (char *)NULL) ? (FUNC)   : "CAM Subsystem" ),  \
	    (((MSGSTR) != (char *)NULL) ? (MSGSTR) : "Unknown Error" ) );\
    }
#endif /* CAMERRLOG and not lint */

#endif /* _CAM_ERRLOG_ */
