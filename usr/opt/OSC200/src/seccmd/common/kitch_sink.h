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
 *	@(#)$RCSfile: kitch_sink.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:16 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifdef SEC_BASE



/*
	kitch_sink.h - everything we might need anywhere that doesn't
	really fit anywhere else

	mostly generic routines && types that some versions of C
	and attendant libraries may not have or do "properly"
*/

#ifndef MIN
#define MIN(A,B)	(A < B ? A : B)
#endif /* MIN */

#ifndef MAX
#define MAX(A,B)	(A > B ? A : B)
#endif /* MAX */

#ifndef X11
# ifndef Boolean
#  define Boolean int
# endif /* Boolean */
#endif /* X11 */


/*
 * tolower() and toupper() WILL check the case before doing the
 * conversion for System V, OSF, and any ANSI-compliant compiler,
 * at the least. Older compilers which follow K&R literally, will
 * need help.
 */

#ifdef CASE_CONV_HELP
# define ToLower(c)	(islower(c) ? tolower(c) : (c))
# define ToUpper(c)	(isupper(c) ? toupper(c) : (c))
#else /* CASE_CONV_HELP */
# define ToLower(c)	tolower(c)
# define ToUpper(c)	toupper(c)
#endif /* CASE_CONV_HELP */


#define UPD_SECS (1 * 60)

#define SECINHOUR	(3600)
#define SECINDAY	(SECINHOUR * 24)
#define SECINWEEK	(SECINDAY * 7)



#endif /* SEC_BASE */
