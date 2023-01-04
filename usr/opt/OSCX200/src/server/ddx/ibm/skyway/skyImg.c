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
 * $XConsortium: skyImg.c,v 1.2 91/07/16 13:15:20 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#include "X.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "windowstr.h"

#include "skyHdwr.h"
#include "skyReg.h"
#include "ibmTrace.h"

/* DMA */
/* move int by int instead of byte by byte */

void
skywayDrawColorImage(x, y, w, h, data, RowIncrement, alu, mask,index)
    int	x ;
    int	y ;
    int w ;
    int h ;
    register unsigned char *data ;
    int		RowIncrement ;
    int	alu ;
    unsigned long int mask ;
    int	index ;
{
	unsigned char *sptr;            /* the source pointer           */
	unsigned char *dptr, *Dptr;     /* the destination pointer      */
	int i, j;

	TRACE(("SkywayDrawColorImage\n"));

 	skywayWaitFifo2(index);
	sptr = (unsigned char *) data ; 
	Dptr = (unsigned char *) ((SKYWAY_VRAM_START[index] | 
		SKYWAY_OFFSET[index]) + (SKYWAY_WIDTH * y) + x ) ;

	 for ( i=0; i<h; i++ ) {

		dptr = Dptr ;

		for( j=0; j<w; j++ )    /* copy one row into memory     */
		{
			switch( alu )
			{
				case 0x0 : *dptr = 0;
					   break;
				case 0x1 : *dptr =  ( *sptr & *dptr );
					   break;
				case 0x2 : *dptr =  ( *sptr & ~*dptr );
					   break;
				case 0x3 : *dptr =  *sptr;
					   break;
				case 0x4 : *dptr =  ( ~*sptr & *dptr );
					   break;
				case 0x5 : *dptr =  *dptr;
					   break;
				case 0x6 : *dptr =  ( *sptr ^ *dptr );
					   break;
				case 0x7 : *dptr =  ( *sptr | *dptr );
					   break;
				case 0x8 : *dptr =  ( ~*sptr & ~*dptr );
					   break;
				case 0x9 : *dptr =  ( ~*sptr ^ *dptr );
					   break;
				case 0xa : *dptr =  ~*dptr;
					   break;
				case 0xb : *dptr =  ( *sptr | ~*dptr );
					   break;
				case 0xc : *dptr =   ~*sptr;
					   break;
				case 0xd : *dptr =  ( ~*sptr | *dptr );
					   break;
				case 0xe : *dptr =  ( ~*sptr | ~*dptr );
					   break;
				case 0xf : *dptr =  0x1;
					   break;
			}

			dptr++;
			sptr++;

		}

		Dptr += SKYWAY_WIDTH ;

		/* skip 32-bit boundary */

		while ( ( (int) sptr % 4 ) != 0 )
			sptr++;
	}

 	skywayWaitFifo2(index);

        return ;
}

void skywayReadColorImage(x, y, w, h, data ,RowIncrement,index)
    int		x ;
    int		y ;
    int		w ;
    register int h ;
    register unsigned char *data ;
    int		RowIncrement ;
    int		index ;
{
	unsigned char *sptr, *Sptr ;
	unsigned char *dptr ;
	int i, j;

	TRACE(("SkywayReadColorImage\n"));

	dptr = data;    /* data is assumed to be at 32-bit boundary     */

	Sptr = (unsigned char *) ((SKYWAY_VRAM_START[index] | 
	       SKYWAY_OFFSET[index]) + (SKYWAY_WIDTH * y) + x ) ;

	for ( i=0; i<h; i++ )
	{
		sptr = Sptr;
		for( j=0; j<w; j++ )    /* copy one row into memory     */
		{
			*dptr++ = *sptr++;
		}

		/* then skip subsequent destination memory up to the
		   next 32-bit boundary                                 */

		while ( ( (int) dptr % 4 ) != 0 )
		{
			dptr++;
		}

		Sptr += SKYWAY_WIDTH ;
	}

    return ;
}

