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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: _outch.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:43:08 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_outch.c	1.6  com/lib/curses,3.1,8943 10/16/89 22:58:42";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _outch
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "cursesext.h"

#ifdef WCHAR
#include <wchar.h>
#endif /* WCHAR */

int outchcount;

/*
 * NAME:        _outch
 *
 * FUNCTION:
 *
 *      Write out one character to the tty.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is one of the main things
 *      in this level of curses that depends on the outside
 *      environment.
 */

_outch (c)
chtype c;
{
#ifdef WCHAR
	int	clen, i;
	unsigned char	cbuf[5];
	FILE	*out;
	static int out_acschar = -1 ; /* 1 => output ACS, 0 => fallback */

	out = (SP && SP->term_file) ? SP->term_file : stdout;
	outchcount++;

	if( IS_NEXTCHAR( c ) )
	{
		return 0;
	}
        else if( IS_ACSCHAR(c) )
        {
		if (out_acschar == -1)
		{
			/* 
			 * First call, set out_acschar
			 * Set to 1 if smacs[2] = '0' which means mapping
			 * to DEC Special Graphic
			 */
			if ((enter_alt_charset_mode != NULL) &&
			    (enter_alt_charset_mode[2] == '0'))
			    out_acschar = 1 ;
			else
			    out_acschar = 0 ;
		}
		if (out_acschar || ((c & 0x7f0000) == 0)) {
#ifdef DEBUG
# ifndef LONGDEBUG
			if (outf)
				fprintf(outf, "[S0][%x][S1]", c & 0xff);
# else /* LONGDEBUG */
			if (outf) {
				fprintf(outf, "_outch: char 'SO_CODE' term %x file %x=%d\n",
					SP, SP->term_file, fileno(SP->term_file));
				fprintf(outf, "_outch: char '%x' term %x file %x=%d\n",
					c&0xff, SP, SP->term_file, 
					fileno(SP->term_file));
				fprintf(outf, "_outch: char 'S1_CODE' term %x file %x=%d\n",
					SP, SP->term_file, fileno(SP->term_file));
			}
# endif /*LONGDEBUG */
#endif /* DEBUG */
			putc( SO_CODE, out );	
                	putc( c & 0xff, out) ;
			putc( S1_CODE, out );	
		} else {
			/*
			 * Output fallback character which is encoded in
			 * the third byte of that ACS char. 
			 */
#ifdef DEBUG
# ifndef LONGDEBUG
			if (outf)
				fprintf(outf, "[%x]", (c >> 16) & 0x7f);
# else /* LONGDEBUG */
			if (outf)
				fprintf(outf, "_outch: char '%x' term %x file %x=%d\n",
					(c >> 16) & 0x7f, SP, SP->term_file, 
					fileno(SP->term_file));
# endif /*LONGDEBUG */
#endif /* DEBUG */
			putc((c >> 16) & 0x7f, out) ;
		}
		return ACS_WIDTH;	
        }
	else if( (clen = wctomb( cbuf, c&A_CHARTEXT )) > 0 )
	{
#ifdef DEBUG
# ifndef LONGDEBUG
		if (outf) {
			if (clen == 1 && c < ' ')
				fprintf(outf, "^%c", (c+'@')&0177);
			else 
				for( i = 0; i < clen; i++ )
					putc (cbuf[i], outf);
		}
# else /* LONGDEBUG */
		if(outf) for( i = 0; i < clen; i++ )
			fprintf(outf, "_outch[%d]: char '%x' term %x file %x=%d\n",
				i, cbuf[i], SP, SP->term_file, 
				fileno(SP->term_file));
# endif /* LONGDEBUG */
#endif /* DEBUG */
		for( i = 0; i < clen; i++ )
			putc (cbuf[i], out);
	}
	return clen;

#else /* WCHAR */

#ifdef DEBUG
# ifndef LONGDEBUG
	if (outf)
		if (c < ' ')
			fprintf(outf, "^%c", (c+'@')&0177);
		else
			fprintf(outf, "%c", c&0177);
# else /* LONGDEBUG */
	if(outf) fprintf(outf, "_outch: char '%s' term %x file %x=%d\n",
		unctrl(c&0177), SP, SP->term_file, fileno(SP->term_file));
# endif /* LONGDEBUG */
#endif /* DEBUG */

	outchcount++;

	if (SP && SP->term_file)
		putc (c&0177, SP->term_file);
	else
		putc (c&0177, stdout);

#endif /* WCHAR */
}

#ifdef PHASE2
_w_outch( chtype c )
{
	int	clen, i;
	unsigned char	cbuf[5];
	FILE	*out;

	out = (SP && SP->term_file) ? SP->term_file : stdout;
	outchcount++;

        if( IS_NEXTATR( *SP->curatr ) )
	{
		return 0;
	}
        else if( IS_ACSCHAR(c) )
        {
		putc( SO_CODE, out );	
                putc( c & 0xff, out) ;
		putc( S1_CODE, out );	
		return ACS_WIDTH;	
        }
	else if( (clen = wctomb( cbuf, c&A_CHARTEXT )) > 0 )
	{
		for( i = 0; i < clen; i++ )
		{
			putc (cbuf[i], out);
		}
	}
	return clen;
}
#endif
