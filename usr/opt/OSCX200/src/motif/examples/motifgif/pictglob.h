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
 * @(#)$RCSfile: pictglob.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:06:27 $
 */
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */
#include <stdio.h>
#include <X11/Xlib.h>

#define MAX_ARGS     100

#ifdef VMS
#ifndef MAIN
#define WHERE globalref
#else
#define WHERE globaldef
#endif
#else
#ifndef MAIN
#define WHERE extern
#else
#define WHERE
#endif
#endif

typedef unsigned char byte;

#define CENTERX(f,x,str) ((x)-XTextWidth(f,str,strlen(str))/2)
#define CENTERY(f,y) ((y)-((f->ascent+f->descent)/2)+f->ascent)


/* X stuff */
WHERE Display       *theDisp;
WHERE int           theScreen, dispcells;
WHERE Colormap      theCmap;
WHERE Window        rootW, mainW;
WHERE GC            theGC;
WHERE unsigned long fcol,bcol;
WHERE Font          mfont;
WHERE XFontStruct   *mfinfo;
WHERE Visual        *theVisual;
WHERE XImage        *theImage, *expImage;

/* global vars */
WHERE int            iWIDE,iHIGH,eWIDE,eHIGH,expand,numcols,strip,nostrip;
#ifdef VMS
WHERE unsigned long  ccols[256];
#else
WHERE unsigned long  cols[256];
#endif
WHERE XColor         defs[256];
WHERE char          *cmd;

