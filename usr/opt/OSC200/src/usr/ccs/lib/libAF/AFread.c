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
static char	sccsid[] = "@(#)$RCSfile: AFread.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/12/09 22:05:01 $";
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
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

/*
 * NAME:	AFread
 * FUNCTION: 	Read the next entry from an Attribute File.  Parse and
 *	     	fill in the current attribute table and current buffer.
 *
 * RETURN VALUE DESCRIPTION: 
 */

/*
 * Kim Peterson, 4-Apr-1991
 *    Added checks for syntax errors on EOF
 */


#include <stdio.h>
#include <AFdefs.h>

#if defined(NLS) || defined(KJI)
#include <NLchar.h>
#endif


/*
 *	Character to denote the start of a comment line
 */
#define	COMMENTCHAR	'#'

/*
 *	Character types
 */
#define SP 	0			/* space		*/
#define LF 	1			/* line feed		*/
#define CO 	2			/* colon	(':')	*/
#define EQ 	3			/* equal	('=')	*/
#define QO 	4			/* quote	('"')	*/
#define SL	5			/* slash	('\')	*/
#define CM 	6			/* comma	(',')	*/
#define OD 	7			/* octal digit		*/
#define ID 	8			/* other alphanumeric	*/
#define NTYPES 	9

/*
 *	Map of input characters to character types
 */
static char typetab[128] = {       
	SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,LF,LF,LF,SP,SP,SP,
	SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,
	SP,ID,QO,ID,ID,ID,ID,ID,ID,ID,ID,ID,CM,ID,ID,ID,
	OD,OD,OD,OD,OD,OD,OD,OD,ID,ID,CO,ID,ID,EQ,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,SL,ID,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,SP
};

/*
 *	Parse states
 */
#define BOR 	(NTYPES*0)		/* skip to beginning of record */
#define SKR 	(NTYPES*1)		/* skip to key */
#define KEY 	(NTYPES*2)		/* accumulate key */
#define SKK 	(NTYPES*3)		/* skip to next key */
#define SKC 	(NTYPES*4)		/* skip to colon */
#define SKA 	(NTYPES*5)		/* skip to attribute name */
#define ATR 	(NTYPES*6)		/* accumulate name */
#define SKE 	(NTYPES*7)		/* skip to equals */
#define SKV 	(NTYPES*8)		/* skip to value */
#define VAL 	(NTYPES*9)		/* accumulate value */
#define QUO 	(NTYPES*10)		/* accumulate quoted value */
#define SLS 	(NTYPES*11)		/* last char was backslash */
#define OD1 	(NTYPES*12)		/* accumulate octal digits */
#define OD2 	(NTYPES*13)		/* accumulate octal digits */
#define OD3 	(NTYPES*14)		/* accumulate octal digits */
#define SKL 	(NTYPES*15)		/* skip to next value */
#define NSTATES 16			/* # of states - not incl FIN */
#define FIN 	(NTYPES*16)		/* finished state */

/*
 *	Actions
 */
#define E  	0100000		/* error */
#define O  	0040000		/* octal digit */
#define D  	0020000		/* end octal spec */
#define B  	0010000		/* check for backslashed control character */
#define N  	0004000		/* terminate attr name, start value */
#define V  	0002000		/* terminate string */
#define L  	0001000		/* terminate string, start attr name */
#define S  	0000400		/* store character */
#define SMASK 	0377		/* next state */

/*
 *	Transition state table
 */
static unsigned short action[NSTATES*NTYPES] = {
/*
 * SP(0)     LF(1)   CO(2)   EQ(3)   QO(4)   SL(5)   CM(6)   OD(7)   ID(8)
 */
  0|BOR,    0|BOR,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  S|KEY,  S|KEY,/*BOR*/
  0|SKR,    0|BOR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,/*SKR*/
  V|SKC,    E|SKR,V|L|SKA,  E|SKR,  E|SKR,  E|SKR,  V|SKK,  S|KEY,  S|KEY,/*KEY*/
  0|SKK,    L|FIN,  L|SKA,  E|SKR,  E|SKR,  E|SKR,  0|SKK,  S|KEY,  S|KEY,/*SKK*/
  0|SKC,    E|SKR,  L|SKA,  E|SKR,  E|SKR,  E|SKR,  0|SKK,  E|SKR,  E|SKR,/*SKC*/

  0|SKA,    0|SKL,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  S|ATR,  S|ATR,/*SKA*/
  N|SKE,N|V|L|FIN,  E|SKR,  N|SKV,  E|SKR,  E|SKR,  E|SKR,  S|ATR,  S|ATR,/*ATR*/

  0|SKE,  V|L|FIN,  E|SKR,  0|SKV,  E|SKR,  E|SKR,  E|SKR,V|L|SKA,V|L|SKA,/*SKE*/

  0|SKV,    E|SKR,  E|SKR,  E|SKR,  0|QUO,  E|SKR,  E|SKV,  S|VAL,  S|VAL,/*SKV*/
  S|VAL,  V|L|SKL,  E|SKR,  S|VAL,  E|SKR,  E|SKR,  V|SKV,  S|VAL,  S|VAL,/*VAL*/

  S|QUO,    E|SKR,  S|QUO,  S|QUO,  0|VAL,  0|SLS,  S|QUO,  S|QUO,  S|QUO,/*QUO*/
  S|QUO,    E|SKR,  S|QUO,  S|QUO,  S|QUO,  S|QUO,  S|QUO,  D|OD1,B|S|QUO,/*SLS*/
O|S|QUO,    E|SKR,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,  D|OD2,O|S|QUO,/*OD1*/
O|S|QUO,    E|SKR,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,  D|OD3,O|S|QUO,/*OD2*/
O|S|QUO,    E|SKR,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,/*OD3*/

  0|SKL,    V|FIN,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  S|ATR,  S|ATR /*SKL*/
};



int
AFread(af)
	AFILE_t af;
{
	register char *p;
	register ATTR_t a;
	register int t;
	register int c;
#if defined(NLS) || defined(KJI)
	register int c2; /* second byte of a two byte character */
#endif
	int peekc, num, state;

	/*
	 *	INITIALIZE:
	 */
	af->AF_errs = AF_OK; 			/* Initialize entry errors */
	a = af->AF_cent.EN_catr;		/* Attribute list pointer */
	p = af->AF_cent.EN_cbuf;		/* Character buffer pointer */
	*p = '\0';				/* Init 1st usable char buffer */
	af->AF_cent.EN_name = p;		/* Init entry name */
	a->AT_name =  NULL;			/* Init 1st attribute entry */
	a->AT_value = NULL;
	a->AT_nvalue = NULL;

	for (peekc = '\n', num = 0, state = BOR; state != FIN; state = t&SMASK)
	{   
	    /* 
	     *	INPUT:	Get next character
	     */
	    if ((c = peekc) == EOF)
		c = getc(af->AF_iop);
	    peekc = EOF;
	    if (c == EOF)
            {
		/* If EOF in BOR state then return(0) (End of file). 
		 * Else should behave as if we got LF.
		 */
		if (state != BOR) {
		    c = '\n'; 
	        }
		else
		    return(0);
            }

	    /* 
	     *	INPUT:	Remove comment lines from input stream
	     *
	     */
	    if (c == '\n') {
		if ((c=getc(af->AF_iop)) == COMMENTCHAR)
			while ((c = getc(af->AF_iop)) != '\n' && c != EOF)
			;
		peekc = c;
		c = '\n';
	    }

	    /* 
	     *	STATE:	Determine input character type
	     */
#if defined(NLS) || defined(KJI)
	    if (c & 0200) {
		/* if this is a two byte character, note the second byte for
		   later processing. Getting the second byte now elminates
		   the possibility of confusing the second byte for a
		   special character like backslash. */
		if (NCisshift (c))
		    c2 = getc (af->AF_iop);
		t = ID; /* any two byte or high bit character is treated as a
			   ID character. */
	    } else
		t = typetab [c & 0177];
#else
	    t = (!(c & 0200)) ? typetab[c & 0177] : ID;
#endif
		
	    /* 
	     *	STATE:	Determine new state and transition action(s)
	     */
	    t = action[t + state];
	
#ifdef DEBUG
    debuginfo(state, typetab[c & 0177], c, t, af->AF_cent.EN_cbuf, p);
#endif

	    /* 
	     *	ACTION:	Error
	     *		- No action
	     */
	    if (t&E) {
		af->AF_errs |= AF_SYNTAX;
	    }

	    /* 
	     *	ACTION:	Continue octal specification
	     *		- Accumulate octal representation
	     */
	    if (t&D) {   
		num = (num << 3) + (c - '0');
	    }

	    /* 
	     *	ACTION:	End octal specification
	     *		- Convert special representation characters
	     */
	    if (t&O) {   
		peekc = c;
		c = num;
		num = 0;
	    }

	    /* 
	     *	ACTION:	Map special backslash
	     *		- Convert special representation characters
	     */
	    if (t&B) {   
		switch (c) { 
		  case 'b':
		    c = '\b';  break;
		  case 'f':
		    c = '\f';  break;
		  case 'n':
		    c = '\n';  break;
		  case 'r':
		    c = '\r';  break;
		  case 't':
		    c = '\t';  break;
		}
	    }

	    /* 
	     *	ACTION:	Terminate attribute name
	     *		- Terminate string
	     *		- Set AT_value to first character position
	     */
	    if (t&N) {   
		*p++ = '\0';
		a->AT_value = p;
		a->AT_nvalue = NULL;
	    }

	    /* 
	     *	ACTION:	Terminate attribute value
	     *		- Terminate string
	     */
	    if (t&V) {   
		*p++ = '\0';
	    }

	    /* 
	     *	ACTION:	Terminate attribute list
	     *		- Doubly terminate string
	     *		- Increment attribute list pointer, unless 1st use
	     *		- Set AT_name to first character position
	     *		- Set AT_value to AT_name to insure a valid 
	     *			pointer is assigned
	     */
	    if (t&L) {   
		*p++ = '\0';
		if( a->AT_name )
			a++;
		a->AT_name = p;
		a->AT_value = p;
		a->AT_nvalue = NULL;
	    }

	    /* 
	     *	ACTION:	Store char
	     *		- Copy charecter into character buffer
	     *		- If NLS 2-byte character, copy both characters
	     */
	    if (t&S) {   
		*p++ = c;
#if defined(NLS) || defined(KJI)
		if (NCisshift (c) && c2 != EOF)
		    *p++ = c2;
#endif
	    }

	    /* 
	     *	CHECK: End of character buffer reached
	     *		- skip to EOL or EOF
	     *		- set entry error flag
	     *		- break to return
	     */
	    if ((p - af->AF_cent.EN_cbuf) >= (af->AF_maxsiz -2) ) {
		while ((c = getc(af->AF_iop)) != '\n' && c != EOF)
		    ;
		af->AF_errs |= AF_ERRCBUF;
		break;
	    }
	    /* 
	     *	CHECK: End of attribute list reached
	     *		- skip to EOL or EOF
	     *		- set entry error flag
	     *		- break to return
	     */
	    if ((a - af->AF_cent.EN_catr) >= (af->AF_maxatr -1) ) {
		while ((c = getc(af->AF_iop)) != '\n' && c != EOF)
		    ;
		af->AF_errs |= AF_ERRCATR;
		break;
	    }


	} /* End for(;;) */

	/* 
	 *	RETURN:	
	 *		- Terminiate last attribute entry
	 *		- Terminiate string
	 *		- Push start of next record back onto input stream
	 */
	a->AT_name = NULL;
	a->AT_value = NULL;
	a->AT_nvalue = NULL;
	*p = '\0';			/* terminate record */
	if (peekc != EOF)
	    ungetc(peekc,af->AF_iop);
	return(1);
}

#ifdef DEBUG
char *
whatstate(int i)
{
	char * s;
	switch (i) {
	case BOR: s = " BOR"; break;
	case SKR: s = " SKR"; break;
	case KEY: s = " KEY"; break;
	case SKK: s = " SKK"; break;
	case SKC: s = " SKC"; break;
	case SKA: s = " SKA"; break;
	case ATR: s = " ATR"; break;
	case SKE: s = " SKE"; break;
	case SKV: s = " SKV"; break;
	case VAL: s = " VAL"; break;
	case QUO: s = " QUO"; break;
	case SLS: s = " SLS"; break;
	case OD1: s = " OD1"; break;
	case OD2: s = " OD2"; break;
	case OD3: s = " OD3"; break;
	case SKL: s = " SKL"; break;
	case FIN: s = " FIN"; break;
	default:  s = " ???"; break;
	}
	return(s);
}
char *
whattype(int i)
{
	char *	s;
	switch (i) {
	case SP: s = "SP"; break;
	case LF: s = "LF"; break;
	case CO: s = "CO"; break;
	case EQ: s = "EQ"; break;
	case QO: s = "QO"; break;
	case SL: s = "SL"; break;
	case CM: s = "CM"; break;
	case OD: s = "OD"; break;
	case ID: s = "ID"; break;
	default: s = "??"; break;
	}
	return(s);
}

int
debuginfo(int state, int type, int c, int action, char * buf, char * last)
{
	int	i;

	fprintf(stderr,"%s %s (%c)%d: %s ",
		whatstate(state), 
		whattype(type), 
		c,
		c,
		whatstate(action & SMASK));
	for(i=0; i < 60 && buf < last; i++, buf++)
		if ( *buf == '\0' )
			fprintf(stderr, "^");
		else if ( *buf == '\n' )
			fprintf(stderr, "\\n");
		else
			fprintf(stderr, "%c", *buf);
	fprintf(stderr, "\n");
}
    

#endif
