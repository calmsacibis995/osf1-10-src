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
 *	@(#)$RCSfile: error.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/03/31 21:24:59 $
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

/*
 * COMPONENT_NAME: (CMDPROG) error.h
 *
 * FUNCTIONS: TOOLSTR, cerror, uerror, warning, werror                       
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
# ifndef ERROR_H
# define ERROR_H
# ifndef EXIT
# define EXIT exit
# endif

#ifdef MSG
#include "ctools_msg.h"
#define TOOLSTR(Num, Str) NLcatgets(catd, MS_CTOOLS, Num, Str)
nl_catd catd;
#else
#define TOOLSTR(Num, Str) Str
#endif
#define LINT_MAX_ERRORS 	30 
int max_errors = LINT_MAX_ERRORS;
int nerrors = 0;  /* number of errors */
int nmigchk = 0;  /* number of errors suppressed */
int LintErrRC = 2; /* Return code - may be changed to 0 */
/* -------------------- uerror -------------------- */

/*VARARGS1*/
#ifndef __mips64
uerror( mode, s, a, b ) int mode; char *s; caddr_t a, b; {
#else
uerror( mode, s, a, b ) int mode; char *s; size_t a, b; {
#endif
#ifdef LINT
        /* if migration testing allow ONLY error when the print flag is on */
  if (AL_MIGCHK && (!AL_PRINTIT)) {
    ++nmigchk;
    return;
  }
#endif
	if( mode ){
#if	defined (LINT) || defined (CFLOW)
		if( !ifname || !strcmp(pfname, ifname) )
			fprintf( stdout, TOOLSTR(M_MSG_199, "\"%s\", line %d: error: "), pfname, lineno );
		else
			fprintf( stdout, TOOLSTR(M_MSG_277, "\"%s\", line %d (\"%s\"): error: "), ifname, lineno, pfname );
		fprintf( stdout, s, a, b );
		fprintf( stdout, "\n" );
#else
		fprintf( stderr, TOOLSTR(M_MSG_288, "%s, line %d: error: "), ftitle, lineno );
		fprintf( stderr, s, a, b );
		fprintf( stderr, "\n" );
#endif
		if( ++nerrors > max_errors )
			cerror(TOOLSTR(M_MSG_257, "too many errors"));
	} else {
#ifdef WANSI
		warning( WANSI, s, a, b );
#else
		warning( 1, s, a, b );
#endif
#ifdef LINT
	  AL_PRINTIT = 0;
#endif

	}
}

/* -------------------- werror -------------------- */

/*VARARGS1*/
#ifndef __mips64
werror( mode, s, a, b) int mode; char *s; caddr_t a, b; {
#else
werror( mode, s, a, b ) int mode; char *s; size_t a, b; {
#endif
#ifdef LINT
        /* if migration testing allow ONLY error when the print flag is on */
        if (AL_MIGCHK && (!AL_PRINTIT))
	  return;
#endif
	if( mode ){
#if	defined (LINT) || defined (CFLOW)
		if( !ifname || !strcmp(pfname, ifname) )
			fprintf( stdout, TOOLSTR(M_MSG_199, "\"%s\", line %d: error: "), pfname, lineno );
		else
			fprintf( stdout, TOOLSTR(M_MSG_277, "\"%s\", line %d (\"%s\"): error: "), ifname, lineno, pfname );
		fprintf( stdout, s, a, b );
		fprintf( stdout, "\n" );
#else
		fprintf( stderr, TOOLSTR(M_MSG_288, "%s, line %d: error: "), ftitle, lineno );
		fprintf( stderr, s, a, b );
		fprintf( stderr, "\n" );
#endif
	} else {
#ifdef WANSI
		warning( WANSI, s, a, b );
#else
		warning( 1, s, a, b );
#endif
	}
#ifdef LINT
	  AL_PRINTIT = 0;
#endif
      }

/* -------------------- cerror -------------------- */

/*VARARGS1*/
#ifndef __mips64
cerror( s, a, b, c ) char *s; caddr_t a, b, c ; {
#else
cerror(  s, a, b, c ) char *s; size_t a, b,c ; {
#endif
	/*
	** Compiler error: die
	*/
	if( nerrors && nerrors <= max_errors ){
		/* give the compiler the benefit of the doubt */
		fprintf( stderr, TOOLSTR(M_MSG_288, "%s, line %d: error: "), ftitle, lineno );
		fprintf( stderr, TOOLSTR(M_MSG_289,
			"cannot recover from earlier errors: goodbye!\n" ));
	} else {
#if	defined (LINT) || defined (LINTP2)
		fprintf( stderr, TOOLSTR(M_MSG_290, "%s, line %d: lint error: " ), ftitle, lineno);
#else
#if defined (CFLOW) || defined (CFLOW2)
		fprintf( stderr, TOOLSTR(M_MSG_267, "%s, line %d: cflow error: " ), ftitle, lineno);
#else
#if defined (CXREF)
		fprintf( stderr, TOOLSTR(M_MSG_268, "%s, line %d: cxref error: " ), ftitle, lineno);
#else
		fprintf( stderr, TOOLSTR(M_MSG_291, "%s, line %d: compiler error: " ), ftitle, lineno);
#endif
#endif
#endif
		fprintf( stderr, s, a, b, c );
		fprintf( stderr, "\n" );
	}
#ifdef FORT
	{	extern short debugflag;
		if (debugflag)
			abort();
	}
#endif /*FORT*/
	EXIT( LintErrRC );
}

/* -------------------- warning -------------------- */

/*VARARGS1*/
#ifndef __mips64
warning( mode, s, a, b, c, d, e ) int mode; char *s; caddr_t a, b, c, d, e; {
#else
warning( mode, s, a, b, c, d, e ) int mode; char *s; size_t a, b, c, d, e; {
#endif
#ifdef LINT
        /* if migration testing allow ONLY error when the print flag is on */
        if (AL_MIGCHK && (!AL_PRINTIT))
	  return;
	else 
	  AL_PRINTIT = 0;
#endif
	if( mode ){
#if	defined (LINT) || defined (CFLOW)
		if( !ifname || !strcmp(pfname, ifname) )
			fprintf( stdout, TOOLSTR(M_MSG_292, "\"%s\", line %d: warning: "), pfname, lineno );
		else
			fprintf( stdout, TOOLSTR(M_MSG_293, "\"%s\", line %d (\"%s\"): warning: "), ifname, lineno, pfname );
		fprintf( stdout, s, a, b, c, d, e );
		fprintf( stdout, "\n" );
#else
		fprintf( stderr, TOOLSTR(M_MSG_294, "%s, line %d: warning: "), ftitle, lineno );
		fprintf( stderr, s, a, b );
		fprintf( stderr, "\n" );
#endif
	}
}
#endif /* ERROR_H */






