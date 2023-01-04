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
static char *rcsid = "@(#)$RCSfile: image.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 17:54:06 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */
#include "pictglob.h"

Resize(w,h)
int w,h;
{
    int  ix,iy,ex,ey;
    byte *ximag,*ilptr,*ipptr,*elptr,*epptr;
    static char *rstr = "Resizing Image.  Please wait...";

    /* warning:  this code'll only run machines where int=32-bits */

    if (w==iWIDE && h==iHIGH) {		/* very special case */
        if (expImage != theImage) {
            if (expImage) XDestroyImage(expImage);
            expImage = theImage;
            eWIDE = iWIDE;  eHIGH = iHIGH;
            }
        }

    else {				/* have to do some work */
        /* if it's a big image, this'll take a while.  mention it */
        if (w*h>(500*500)) {
            XDrawImageString(theDisp,mainW,theGC,CENTERX(mfinfo,w/2,rstr),
                  CENTERY(mfinfo,h/2),rstr, strlen(rstr));
            XFlush(theDisp);
            }

	/* first, kill the old expImage, if one exists */
	if (expImage && expImage != theImage) {
            free(expImage->data);  expImage->data = NULL;
            XDestroyImage(expImage);
            }

        /* create expImage of the appropriate size */
        
        eWIDE = w;  eHIGH = h;
        ximag = (byte *) malloc(w*h);
        expImage = XCreateImage(theDisp,theVisual,8,ZPixmap,0,(char *)ximag,
                        eWIDE,eHIGH,8,eWIDE);

        if (!ximag || !expImage) {
            fprintf(stderr,"ERROR: unable to create a %dx%d image\n",w,h);
            exit(0);
            }

        elptr = epptr = (byte *) expImage->data;

        for (ey=0;  ey<eHIGH;  ey++, elptr+=eWIDE) {
            iy = (iHIGH * ey) / eHIGH;
            epptr = elptr;
            ilptr = (byte *) theImage->data + (iy * iWIDE);
            for (ex=0;  ex<eWIDE;  ex++,epptr++) {
                ix = (iWIDE * ex) / eWIDE;
                ipptr = ilptr + ix;
                *epptr = *ipptr;
                }
            }
        }
}
