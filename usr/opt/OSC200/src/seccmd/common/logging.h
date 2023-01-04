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
 *	@(#)$RCSfile: logging.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:21 $
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
#ifndef _LOGGING_H
#define _LOGGING_H



/*
 * logging.h - logging macros
 */

#ifndef FILE
#include <stdio.h>
#endif /* FILE */

#ifndef GLOBAL
#include "gl_defs.h"
#endif /* GLOBAL */

GLOBAL char	*_SWlogfn INIT1 (".SWlog");	/* logging file name */
GLOBAL FILE	*_SWlogfp INIT1 (NULL);		/* logging file pointer */
GLOBAL int	_SWlogging INIT1 (1);		/* logging is on? y=1, n=0 */
GLOBAL int	_SWloglevel INIT1 (2);		/* logging level: 0 - n */

/*
 * recommended logging levels:
 *   major routines:
 *   1  - enter & exit function
 *   2  - function arguments
 *   3  - major decision points
 *   4  - major variable tracing
 *   5  - fine detail information
 *   library & convenience routines:
 *   6  - enter & exit function
 *   7  - function arguments
 *   8  - major decision points
 *   9  - major variable tracing
 *   10 - fine detail information
 */

#define FUNCLLVL	1
#define ARGLLVL		2
#define DECLLVL		3
#define VARLLVL		4
#define DETLLVL		5

#define LIBFUNCLLVL	6
#define LIBARGLLVL	7
#define LIBDECLLVL	8
#define LIBVARLLVL	9
#define LIBDETLLVL	10

#define MaxSWLogLevel	LIBDETLLVL

/*
 * Usage:
 *    OPENLOG();
 *    SWLOG(loglevel, format, arg1, arg2, arg3);
 *    CLOSELOG();
 *
 *    ENTERFUNC(F); -		entering function F logging
 *    EXITFUNC(F); -		exiting function F logging
 *    ERRFUNC(F, R); -		error exiting function F (reason = R) logging
 *    DUMPARGS(F,A1,A2,A3); -	dump designated args
 *    DUMPDECP(F,A1,A2,A3); -	dump decision point data
 *    DUMPVARS(F,A1,A2,A3); -	dump designated vars
 *    DUMPDETI(F,A1,A2,A3); -	dump detail information
 *
 *    the same macros names with an L in the middle are for library
 *    versions of the macros.
 */

#ifdef LOGGING

#define OPENLOG() {if (_SWlogging) {if (!(_SWlogfp=fopen(_SWlogfn,"wa")))\
	{fprintf(stderr,"Could not open log file.\n"); _SWlogging=0; sleep(2);}\
	else{long T,time();time(&T);\
		fprintf (_SWlogfp,"\n>>> aif <<< %s\nlog level=[%d]\n",\
			ctime(&T), _SWloglevel);}}}

#define SWLOG(L,F,P1,P2,P3) {if (_SWlogging && L<=_SWloglevel)\
	{fprintf (_SWlogfp, "%-2.2d: ", L);\
		fprintf (_SWlogfp, F, P1, P2, P3);\
		fprintf (_SWlogfp, "\n");\
		fflush (_SWlogfp);}}

#define CLOSELOG() {if (_SWlogging) fclose (_SWlogfp);}

#define ENTERFUNC(FN) {SWLOG(FUNCLLVL, "Entering %s", FN, NULL, NULL);}

#define EXITFUNC(F) {SWLOG(FUNCLLVL, "Exiting %s", F, NULL, NULL);}

#define ERRFUNC(F,R) {SWLOG(FUNCLLVL, "Exiting %s (Reason = %s)", F, R, NULL);}

#define DUMPARGS(F,A1,A2,A3) {SWLOG(ARGLLVL, F, A1, A2, A3);}

#define DUMPDECP(F,A1,A2,A3) {SWLOG(DECLLVL, F, A1, A2, A3);}

#define DUMPVARS(F,A1,A2,A3) {SWLOG(VARLLVL, F, A1, A2, A3);}

#define DUMPDETI(F,A1,A2,A3) {SWLOG(DETLLVL, F, A1, A2, A3);}


#define ENTERLFUNC(FN) {SWLOG(LIBFUNCLLVL, "Entering %s", FN, NULL, NULL);}

#define EXITLFUNC(F) {SWLOG(LIBFUNCLLVL, "Exiting %s", F, NULL, NULL);}

#define ERRLFUNC(F,R) {SWLOG(LIBFUNCLLVL,"Exiting %s (Reason = %s)",F,R,NULL);}

#define DUMPLARGS(F,A1,A2,A3) {SWLOG(LIBARGLLVL, F, A1, A2, A3);}

#define DUMPLDECP(F,A1,A2,A3) {SWLOG(LIBDECLLVL, F, A1, A2, A3);}

#define DUMPLVARS(F,A1,A2,A3) {SWLOG(LIBVARLLVL, F, A1, A2, A3);}

#define DUMPLDETI(F,A1,A2,A3) {SWLOG(LIBDETLLVL, F, A1, A2, A3);}

#else /*  LOGGING */

#define OPENLOG()
#define SWLOG(L,F,P1,P2,P3)
#define CLOSELOG()
#define ENTERFUNC(F)
#define EXITFUNC(F)
#define ERRFUNC(F,R)
#define DUMPARGS(F,A1,A2,A3)
#define DUMPDECP(F,A1,A2,A3)
#define DUMPVARS(F,A1,A2,A3)
#define DUMPDETI(F,A1,A2,A3)
#define ENTERLFUNC(F)
#define EXITLFUNC(F)
#define ERRLFUNC(F,R)
#define DUMPLARGS(F,A1,A2,A3)
#define DUMPLDECP(F,A1,A2,A3)
#define DUMPLVARS(F,A1,A2,A3)
#define DUMPLDETI(F,A1,A2,A3)

#endif /*  LOGGING */

#endif /* _LOGGING_H */
#endif /* SEC_BASE */
