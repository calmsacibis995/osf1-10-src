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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbpntarea.h,v 1.1.2.2 92/01/07 12:50:59 Jim_Ludwig Exp $ */
/* Solid rectangle painters */
extern void cfbSolidFillAreaCopy();
extern void cfbSolidFillAreaXor();
extern void cfbSolidFillAreaGeneral();

/* Transparent stipple rectangle painters */
extern void cfbTSFillAreaCopy();
extern void cfbTSFillAreaXor();
extern void cfbTSFillAreaGeneral();

/* Transparent stipple rectangle painters, width = 32 */
extern void cfbTSFillArea32Copy();
extern void cfbTSFillArea32Xor();
extern void cfbTSFillArea32General();

/* Opaque stipple rectangle painters */
extern void cfbOSFillAreaCopy();
extern void cfbOSFillAreaXor();
extern void cfbOSFillAreaGeneral();

/* Opaque stipple rectangle painters, width = 32 */
extern void cfbOSFillArea32Copy();
extern void cfbOSFillArea32Xor();
extern void cfbOSFillArea32General();

/* Opaque stipple CopyPlane painters */
extern void cfbOSPlaneCopy();
extern void cfbOSPlaneXor();
extern void cfbOSPlaneGeneral();

