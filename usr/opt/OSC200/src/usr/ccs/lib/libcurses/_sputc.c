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
static char rcsid[] = "@(#)$RCSfile: _sputc.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:52:47 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_sputc.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:01:51";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _sputc
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"

#ifdef WCHAR
#include <wchar.h>
#endif

/*
 * NAME:        _sputc
 */

#ifdef DEBUG
_sputc(c, f)
chtype c;
FILE *f;
{
	int	so;
#ifdef WCHAR
	int	clen, i;
	unsigned char cbuf[5];

	so = c & A_ATTRIBUTES;
	c &= A_CHARTEXT;

	if( IS_NEXTCHAR( c ) ){
		fprintf( f, "NN" );
		return;
	}
	if( so ){
		putc( '<', f);
		fprintf( f, "%o,", so );
	}

        if( (clen = wctomb( cbuf, c )) > 1 )
        {
                for( i = 0; i < clen; i++ )
                {
                        putc( cbuf[i], f );
                }
		fprintf( f, "(%02x)", c );
        }else{
		putc( cbuf[0], f );
	}
	if( so )
		putc( '>', f );
#else
	so = c & A_ATTRIBUTES;
	c &= 0177;
	if (so) {
		putc('<', f);
		fprintf(f, "%o,", so);
	}
	putc(c, f);
	if (so)
		putc('>', f);
#endif
}
#endif
