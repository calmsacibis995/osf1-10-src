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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/12/17 21:07:10 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: LintPass2, main
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * main.c	1.4  com/cmd/prog/lint/pass2,3.1,9013 3/5/90 16:14:30"; 
 */
#include "lint_msg.h"
#define         MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;

#include "mfile1.h"
#include "error.h"
#include "lint2.h"

main(argc, argv)
int argc;
register char *argv[];
{
	register int i, c;
	register char *cp;
	register int r;
	int fdef = 0;
	catd = catopen(MF_LINT, NL_CAT_LOCALE);

	for( i = 0; i <= MXDBGFLG; i++ ){
#ifdef LINT
	        alphalevel[i] =0;
#endif
		devdebug[i] = 0;
		warnlevel[i] = 0;
	};

	for( i=1; i<argc; ++i ){
		if( *(cp=argv[i]) == '-' )
		while( *++cp ){
		    switch( *cp ){
		    case 'p': /* extreme portability */
			    pflag++;
			    break;

		    case 'X': /* debug toggles */
			    /*
			     *** Fii:
			     **   Xdollar is passed to lint for free. 
			     *** gaf:   
			     **   set the flag, but the flag is not yet used
			     **   in lint2.
			     */
			    if (strcmp(cp, Xdollar) == 0) {
				lintXdlr++;
				cp += strlen(cp);
			    } else while(*++cp) {
			      switch(*cp) {
			      case '2':	/* General debug */
				debug++;
				break;
			      case '3':	/* Fred (fii) Debug in reader.c */
				fdebug++;
				break;
			      }
			    }
			    --cp;
			    break;

		    case 'w': /* toggle warning level */
			    while( *++cp )
				if( *cp == 'A' )
					for( r = 0; r <= MXDBGFLG; r++ )
						warnlevel[r] = !warnlevel[r];
				else if( ( *cp >= 'a' && *cp <= 'z' ) ||
						( *cp >= 'A' && *cp <= 'Z' ) )
					warnlevel[*cp] = !warnlevel[*cp];
			    --cp;
			    break;

#ifdef LINT
		    case 'Q': /* -Q The Migration check flag */
			   /* First set up full checking */
			   for( r = 0; r <= MXDBGFLG; r++ )
				alphalevel[r] = !alphalevel[r];
			   /* Supress those desired */
			    while( *++cp )
				if( ( *cp >= 'a' && *cp <= 'z' ) ||
						( *cp >= 'A' && *cp <= 'Z' ) )
				alphalevel[*cp] = !alphalevel[*cp];
			    AL_PRINTIT = 0; /* Initially nothing prints */
			   --cp;
			   break;
#endif
		    case 'M': /* HCR development toggles */
			    while( *++cp )
				if( ( *cp >= 'a' && *cp <= 'z') ||
						( *cp >= 'A' && *cp <= 'Z' ) )
					devdebug[*cp] = !devdebug[*cp];
			    --cp;
			    break;
		    }
		}

		else {
			/* First-time initializations. */
			if (fdef == 0) {
				/* check for ANSI mode */
				if ( devdebug[ANSI_MODE] ) {
					/* set all other ANSI flags on */
					devdebug[ANSI_PARSE]	=
						!devdebug[ANSI_PARSE];
					devdebug[COMPATIBLE]	=
						!devdebug[COMPATIBLE];
					devdebug[PROMOTION]	=
						!devdebug[PROMOTION];
					devdebug[REFDEF]	=
						!devdebug[REFDEF];
					devdebug[SCOPING]	=
						!devdebug[SCOPING];
					devdebug[STRUCTBUG]	=
						!devdebug[STRUCTBUG];
					devdebug[TYPING]	=
						!devdebug[TYPING];
				}
				InitTypes();
				fdef = 1;
				curSym = &theSym;
			}
			fname = argv[i];
			OpenFile();
			LintPass2();
			CloseFile();
		}
	}
	CheckSymbols();
}

LintPass2()
{
	int stat;

	/* Read and process all symbol records. */
	while (1) {
		switch (InHeader()) {
		case LINTEOF:
			/* EOF delimiter. */
			if (markerEOF)
				return;
			markerEOF = 1;
			continue;

		case LINTADD:
			/* Append to function usage, symbol should exist. */
			InUsage();
			prevSym = LookupSymbol(curSym, LOOK);
			if (prevSym)
				AddFtnUsage(prevSym, curSym);
			else
				cerror(MSGSTR(M_MSG_260,
					"i/o sequence error on file %s"),
					curPFname);

			break;

		case LINTSYM:
			/* Fetch symbol. */
			InSymbol();
			prevSym = LookupSymbol(curSym, LOOK);

			/* Cross-check old symbols, otherwise
			 * insert new symbols. */
			if (prevSym) {
				switch (stat = CheckSymbol()) {
				case STORE:
					(void) LookupSymbol(curSym, stat);
					break;
				case CHANGE:
					ChangeSymbol(prevSym, curSym);
					break;
				case REJECT:
					/* Remember bad function calls. */
					if (ISFTN(curSym->type))
						FtnRefSymbol(prevSym, curSym);
#ifndef	DEBUG
					if (debug)
						PrintSymbol("REJECTING",
							curSym);
#endif
					break;
				default:
					cerror(MSGSTR(M_MSG_261,
			"unknown action for symbol in file %s"), curPFname);
				}
			}
			else
				(void) LookupSymbol(curSym, STORE);
			break;

		default:
			cerror(MSGSTR(M_MSG_262,
				"unknown record directive in file %s"),
				curPFname);
		}
	}
}
