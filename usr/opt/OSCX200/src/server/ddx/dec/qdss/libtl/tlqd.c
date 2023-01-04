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
/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 *  QzQd.c
 *
 *  written by Brian Kelleher; Sept. 1986
 *
 *  distinguish between 8bit grey scale, 8bit
 *  pseudo color, and 24bit real color.
 */

#include	"Ultrix2.0inc.h"
#include	<vaxuba/qdioctl.h>
#include	"tl.h"

#if	NPLANES!=24
#define QD_ID_8_PLANES 0x40
#define QD_ID_4_PLANES 0x20
#endif

extern int Vaxstar;

/*
 *  Determine what machine we're running on.
 */
int QzQdGetNumPlanes()
{
    short config;
    int numplanes;

    ioctl(fd_qdss, QD_RDCONFIG, &config);

#if NPLANES==24
    if (config & (1<<4))
	numplanes = 8;
    else
	numplanes = 24;
#else
    if (Vaxstar)
       numplanes = config;
    else {
       if ( (config & QD_ID_8_PLANES) == 0)
	   numplanes = 8;
       else if ( (config & QD_ID_4_PLANES) == 0)
	   numplanes = 4;
       else
	   numplanes = 24;
    }
#endif

    return(numplanes);
}
