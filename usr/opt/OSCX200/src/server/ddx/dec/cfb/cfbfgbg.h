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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbfgbg.h,v 1.1.2.2 92/01/07 12:49:02 Jim_Ludwig Exp $ */
#ifdef MITR5
#include "cfbdec.h"
#endif	/* #ifdef MITR5 */

FGBG    cfbGetFGBG(/* Pixel32 fg, bg */);
void	cfbFreeFGBG(/* FGBG fgbg */);

FGBGMap cfbGetITFGBGMap(/* GCPtr pGC */);
FGBGMap cfbGetOSFGBGMap(/* GCPtr pGC */);
FGBGMap cfbGetTSFGBGMap(/* GCPtr pGC */);
