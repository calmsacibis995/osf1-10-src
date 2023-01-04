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
static char	*sccsid = "@(#)$RCSfile: input.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:04 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *
 *	UNIX debugger
 *
 */

#include <i386/kdb/defs.h>

INT		mkfault;
char		line[LINSIZ];
char            oldline[LINSIZ];
INT		infile;
char		*lp;
char		peekc,lastc = EOR;
INT		eof;
short		echo = 1;

/* input routines */

eol(c)
char	c;
{
	return(c==EOR || c==';');
}

rdc()
{
	do {
		readchar();
	}
	while( lastc==' ' || lastc=='\t' );
	return(lastc);
}

char erasec[] = {
	'\b', ' ', '\b'};

char *
editchar(_line_, _lp_)
char *_line_;
register char *_lp_;
{
	/* !echo means literalnext */
	if (echo == 0) {
		char printable;
		if (isprint(*_lp_)) {
			write(1, erasec + 1, sizeof(erasec) - 1);
			write(1, _lp_, 1);
		} else {
			printable = *_lp_ ^ 0100;
			write(1, "^", 1);
			write(1, &printable, 1);
		}
		echo = 1;
		return(++_lp_);
	}
	switch (*_lp_)
	{
	case 0177:
	case 'H'&077:
		if (_lp_ > _line_)
		{
			if (*_lp_ == 0177)
                                write(1, erasec, sizeof(erasec));
                        else
                                write(1, erasec + 1, sizeof(erasec) - 1);
			_lp_--;
		}
		break;
	case 'U'&077:
		while (_lp_ > _line_)
		{
			write(1, erasec, sizeof(erasec));
			_lp_--;	/* back up over this character */
		}
		break;
	case 'R'&077:
		write(1, "^R\n", 3);
		while (_lp_ > _line_) {
			if (!isprint(*_line_)) {
				char printable = *_line_ ^ 0100;
				write(1, "^", 1);
				write(1, &printable, 1);
			} else {
				write(1, _line_, 1);
			}
			_line_++;
		}
		/* _lp_ has not changed - ^R will be overwritten on next call */
		break;
	case 'V'&077:
		write(1, "^\b", 2);
		echo = 0;
		/* _lp_ has not changed - ^V will be overwritten on next call */
		break;
	case '\n':
		_lp_++;
		break;
	default:
		/* Pass through all non-control characters */
		/* swallow all unknown (non-quoted) control characters */
		if (isprint(*_lp_)) _lp_++;
	}
	return(_lp_);
}

readchar()
{
	if ( eof ) {
		lastc=0;
	} 
	else {
		if ( lp==0 ) {
			strncpy(oldline, line, sizeof oldline);
			lp=line;
			do {
				eof = read(infile,lp,1)==0;
				if ( mkfault ) {
					error(0);
				}
				lp = editchar(line, lp);
			}
			while( eof==0 && (lp == line || lp[-1]!=EOR) ) ;
			*lp=0;
			lp=line;
		}
		if ( lastc = peekc ) {
			peekc=0;
		} else if ( lastc = *lp ) {
			lp++;
		}
	}
	return(lastc);
}

nextchar()
{
	if ( eol(rdc()) ) {
		lp--;
		return(0);
	} else {
		return(lastc);
	}
}

quotchar()
{
	if ( readchar()=='\\' ) {
		return(readchar());
	} else if ( lastc=='\'' ) {
		return(0);
	} else {
		return(lastc);
	}
}

getformat(deformat)
string_t		deformat;
{
	register string_t	fptr;
	register BOOL	quote;
	fptr=deformat;
	quote=FALSE;
	while ( (quote ? readchar()!=EOR : !eol(readchar())) ) {
		if ( (*fptr++ = lastc)=='"' ) {
			quote = ~quote;
		}
	}
	lp--;
	if ( fptr!=deformat ) {
		*fptr++ = '\0';
	}
}


re_input()
{
	strncpy(line, oldline, sizeof line);
	lp = line;
	eof = 0;
	peekc = ';';
}
