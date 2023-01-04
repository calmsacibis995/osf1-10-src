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
 * @(#)$RCSfile: MuteKeysym.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/10/15 22:44:39 $
 */
/*
 * compilation of known Dead Key Keysyms from 
 * Dec, Sun, IBM, HP
 */

/*		--- AIXmutesym.h ---
	AIX private mute character-accent symbols
*/

#ifndef	XK_dead_acute

/* dead key keysyms */
#define	XK_dead_circumflex	0x1800005e
#define	XK_dead_grave		0x18000060
#define	XK_dead_tilde		0x1800007e
#define	XK_dead_diaeresis	0x180000a8
#define	XK_dead_macron		0x180000af
#define	XK_dead_degree		0x180000b0
#define	XK_dead_acute		0x180000b4
#define	XK_dead_cedilla		0x180000b8
#define	XK_dead_breve		0x180001a2
#define	XK_dead_ogonek		0x180001b2
#define	XK_dead_caron		0x180001b7
#define	XK_dead_doubleacute	0x180001bd
#define	XK_dead_abovedot	0x180001ff

#endif	/* XK_dead_acute */
/*		--- DECmutesym.h ---
	DEC private mute character-accent symbols
*/

#ifndef DXK_acute_accent

/*
 * DEC private keysyms
 * (29th bit set)
 */

/* two-key compose sequence initiators, chosen to map to Latin1 characters */

#define DXK_ring_accent         0x1000FEB0
#define DXK_circumflex_accent   0x1000FE5E
#define DXK_cedilla_accent      0x1000FE2C
#define DXK_acute_accent        0x1000FE27
#define DXK_grave_accent        0x1000FE60
#define DXK_tilde               0x1000FE7E
#define DXK_diaeresis           0x1000FE22

#endif /* DXK_acute_accent */
/*		--- HPmutesym.h ---
	HP private mute character-accent symbols
*/

#ifndef XK_mute_acute

#define XK_mute_acute           0x100000a8
#define XK_mute_grave           0x100000a9
#define XK_mute_asciicircum     0x100000aa
#define XK_mute_diaeresis       0x100000ab
#define XK_mute_asciitilde      0x100000ac

#endif   /* XK_mute_acute */
/*		--- SUNmutesym.h ---
	SUN private mute character-accent symbols
*/

#ifndef SunXK_FA_Acute

/*
 * Floating Accent
 */

#define	SunXK_FA_Grave		0x1005FF00
#define	SunXK_FA_Circum		0x1005FF01
#define	SunXK_FA_Tilde		0x1005FF02
#define	SunXK_FA_Acute		0x1005FF03
#define	SunXK_FA_Diaeresis	0x1005FF04
#define	SunXK_FA_Cedilla	0x1005FF05

#endif   /* SunXK_FA_Acute */
