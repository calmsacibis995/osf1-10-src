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
#ifndef TESTINIT
#ifndef LINT
#ifdef RCS_ID
#endif 
#endif 

#ifndef LIBTEST
#include "LibTest.h"
#ifndef LIBTEST
#define LIBTEST
#endif
#endif
#ifndef _X11_XLIB
#include <X11/Xlib.h>
#ifndef _X11_XLIB
#define _X11_XLIB
#endif
#endif


#ifdef AUTOHEADER
#else
#endif

#ifndef	GLOBAL
#  define	GLOBAL
#endif

extern Display	*pDpy;
extern GC	gc;


extern void
TestInit();
#ifdef XDEBUG
#endif 
#ifdef NEED_COLORMAP
#endif 
#ifdef NEED_WINDOW
#endif 
#ifdef NEED_GC
#endif 
#ifdef NEED_FONTS
#endif 
extern void
TestCleanup();
#ifdef NEED_GC
#endif 
#ifdef NEED_WINDOW
#endif 
#ifdef NEED_FONTS
#endif 
#ifndef TESTINIT
#define TESTINIT
#endif
#endif
