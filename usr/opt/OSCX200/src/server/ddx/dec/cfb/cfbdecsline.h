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
#ifdef MITR5
/* Solid line painters, single clip rectangle */
extern void cfb8LineSS1RectCopy();
extern void cfb8LineSS1RectXor();
extern void cfb8LineSS1RectGeneral();

/* Solid segment painters, single clip rectangle */
extern void cfb8SegmentSS1RectCopy();
extern void cfb8SegmentSS1RectXor();
extern void cfb8SegmentSS1RectGeneral();

#else
/* Solid line painters, single clip rectangle */
extern void cfbLineS1Copy();
extern void cfbLineS1Xor();
extern void cfbLineS1General();

/* Solid segment painters, single clip rectangle */
extern void cfbSegS1Copy();
extern void cfbSegS1Xor();
extern void cfbSegS1General();

#endif

