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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbfillsp.h,v 1.1.2.2 92/01/07 12:49:11 Jim_Ludwig Exp $ */

/* Solid span painters */
#ifdef MITR5
extern void cfbSolidSpansCopy();
extern void cfbSolidSpansXor();
extern void cfbSolidSpansGeneral();
#else	/* MITR5 */
extern void cfbSolidFSCopy();
extern void cfbSolidFSXor();
extern void cfbSolidFSGeneral();
#endif	/* MITR5 */

/* Transparent stipple span painters */
extern void cfbStippleFSCopy();
extern void cfbStippleFSXor();
extern void cfbStippleFSGeneral();

/* Transparent stipple span painters, stipple width 32 */
extern void cfbStippleFS32Copy();
extern void cfbStippleFS32Xor();
extern void cfbStippleFS32General();

/* Opaque stipple span painters */
extern void cfbOpqStippleFSCopy();
extern void cfbOpqStippleFSXor();
extern void cfbOpqStippleFSGeneral();

/* Opaque stipple span painters, stipple width 32 */
extern void cfbOpqStippleFS32Copy();
extern void cfbOpqStippleFS32Xor();
extern void cfbOpqStippleFS32General();

