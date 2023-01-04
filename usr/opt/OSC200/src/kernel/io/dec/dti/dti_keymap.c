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
static char *rcsid = "@(#)$RCSfile: dti_keymap.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/16 07:58:35 $";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * dti_keymap.c
 *
 * Modification history
 *
 * 06-Aug-91 - R. Craig Peterson
 *
 *	Initial version of code.
 *
 */

/*
 * This file contains the standard keyboard translation tables.
 */

/*
 * dti (lk501) to lk201 translation codes
 */

unsigned char dti_to_lk201[256]=
{
    /* Key code */
    0x00,			/* Unused */
    0x01,			/* Unused */
    0x02,			/* Unused */
    0x03,			/* Unused */
    0x04,			/* Unused */
    0x05,			/* Unused */
    0x06,			/* Unused */
    0x07,			/* Unused */
    0x08,			/* Unused */
    0x09,			/* Unused */
    0x0a,			/* Unused */
    0x0b,			/* Unused */
    0x0c,			/* Unused */
    0x0d,			/* Unused */
    0x0e,			/* Unused */
    0x0f,			/* Unused */
    0x10,			/* Unused */
    0x11,			/* Unused */
    0x12,			/* Unused */
    0x13,			/* Unused */
    0x14,			/* Unused */
    0x15,			/* Unused */
    0x16,			/* Unused */
    0x17,			/* Unused */
    0x18,			/* Unused */
    0x19,			/* Unused */
    0x1a,			/* Unused */
    0x1b,			/* Unused */
    0x1c,			/* Unused */
    0x1d,			/* Unused */
    0x1e,			/* Unused */
    0x1f,			/* Unused */
    0x20,			/* Unused */
    0x21,			/* Unused */
    0x22,			/* Unused */
    0x23,			/* Unused */
    0x24,			/* Unused */
    0x25,			/* Unused */
    0x26,			/* Unused */
    0x27,			/* Unused */
    0x28,			/* Unused */
    0x29,			/* Unused */
    0x2a,			/* Unused */
    0x2b,			/* Unused */
    0x2c,			/* Unused */
    0x2d,			/* Unused */
    0x2e,			/* Unused */
    0x2f,			/* Unused */
    0x30,			/* Unused */
    0x31,			/* Unused */
    0x32,			/* Unused */
    0x33,			/* Unused */
    0x34,			/* Unused */
    0x35,			/* Unused */
    0x36,			/* Unused */
    0x37,			/* Unused */
    0x38,			/* Unused */
    0x39,			/* Unused */
    0x3a,			/* Unused */
    0x3b,			/* Unused */
    0x3c,			/* Unused */
    0x3d,			/* Unused */
    0x3e,			/* Unused */
    0x3f,			/* Unused */
    0x40,			/* Unused */
    0x41,			/* Unused */
    0x42,			/* Unused */
    0x43,			/* Unused */
    0x44,			/* Unused */
    0x45,			/* Unused */
    0x46,			/* Unused */
    0x47,			/* Unused */
    0x48,			/* Unused */
    0x49,			/* Unused */
    0x4a,			/* Unused */
    0x4b,			/* Unused */
    0x4c,			/* Unused */
    0x4d,			/* Unused */
    0x4e,			/* Unused */
    0x4f,			/* Unused */
    0x50,			/* Unused */
    0x51,			/* Unused */
    0x52,			/* Unused */
    0x53,			/* Unused */
    0x54,			/* Unused */
    0x55,			/* Unused */
    0x56,			/* F1 */
    0x57,			/* F2 */
    0x58,			/* F3 */
    0x59,			/* F4 */
    0x5a,			/* F5 */
    0x5b,			/* Unused */
    0x5c,			/* Unused */
    0x5d,			/* Unused */
    0x5e,			/* Unused */
    0x5f,			/* Unused */
    0x60,			/* Unused */
    0x61,			/* Unused */
    0x62,			/* Unused */
    0x63,			/* Unused */
    0x64,			/* F6 */
    0x65,			/* F7 */
    0x66,			/* F8 */
    0x67,			/* F9 */
    0x68,			/* F10 */
    0x69,			/* Unused */
    0x6a,			/* Unused */
    0x6b,			/* Unused */
    0x6c,			/* Unused */
    0x6d,			/* Unused */
    0x6e,			/* Unused */
    0x6f,			/* Unused */
    0x70,			/* Unused */
    0x71,			/* F11 */
    0x72,			/* F12 */
    0x73,			/* F13 */
    0x74,			/* F14 */
    0x75,			/* Unused */
    0x76,			/* Unused */
    0x77,			/* Unused */
    0x78,			/* Unused */
    0x79,			/* Unused */
    0x7a,			/* Unused */
    0x7b,			/* Unused */
    0x7c,			/* Help */
    0x7d,			/* Do */
    0x7e,			/* Unused */
    0x7f,			/* Unused */
    0x80,			/* F17 */
    0x81,			/* F18 */
    0x82,			/* F19 */
    0x83,			/* F20 */
    0x84,			/* Unused */
    0x85,			/* Unused */
    0x86,			/* Unused */
    0x87,			/* Unused */
    0x88,			/* Unused */
    0x89,			/* Unused */
    0x8a,			/* Find */
    0x8b,			/* Insert Here */
    0x8c,			/* Remove */
    0x8d,			/* Select */
    0x8e,			/* Prev */
    0x8f,			/* Next */
    0x90,			/* Unused */
    0x91,			/* Unused */
    0x92,			/* KP0 */
    0x93,			/* Unused */
    0x94,			/* KP. */
    0x95,			/* Enter */
    0x96,			/* KP1 */
    0x97,			/* KP2 */
    0x98,			/* KP3 */
    0x99,			/* KP4 */
    0x9a,			/* KP5 */
    0x9b,			/* KP6 */
    0x9c,			/* KP, */
    0x9d,			/* KP7 */
    0x9e,			/* KP8 */
    0x9f,			/* KP9 */
    0xa0,			/* KP- */
    0xa1,			/* PF1 */
    0xa2,			/* PF2 */
    0xa3,			/* PF3 */
    0xa4,			/* PF4 */
    0xa5,			/* Unused */
    0xa6,			/* Unused */
    0xa7,			/* Left arrow */
    0xa8,			/* Right arrow */
    0xa9,			/* Down arrow */
    0xaa,			/* Up arrow */
    0xab,			/* Right Shift */
    0xac,			/* Left Alt */
    0xad,			/* Right Compose */
    0xae,			/* Left Shift */
    0xaf,			/* Ctrl */
    0xb0,			/* Lock */
    0xb1,			/* Left Compose */
    0xb2,			/* Right Alt */
    0xb3,			/* Unused */
    0xb4,			/* Unused */
    0xb5,			/* Unused */
    0xb6,			/* Unused */
    0xb7,			/* Unused */
    0xb8,			/* Unused */
    0xb9,			/* Unused */
    0xba,			/* Unused */
    0xbb,			/* Unused */
    0xbc,			/* <X] */
    0xbd,			/* Return */
    0xbe,			/* Tab */
    0xbf,			/* Unused */
    0xc0,			/* 1 */
    0xc1,			/* Q */
    0xc2,			/* A */
    0xc3,			/* Unused */
    0xc4,			/* Unused */
    0xc5,			/* 2 */
    0xc6,			/* Unused */
    0xc7,			/* S */
    0xc8,			/* Unused */
    0xc9,			/* < */
    0xca,			/* Unused */
    0xcb,			/* 3 */
    0xcc,			/* E */
    0xcd,			/* D */
    0xce,			/* C */
    0xcf,			/* Unused */
    0xd0,			/* 4 */
    0xd1,			/* R */
    0xd2,			/* F */
    0xd3,			/* Unused */
    0xd4,			/* Space Bar */
    0xd5,			/* Unused */
    0xd6,			/* 5 */
    0xd7,			/* T */
    0xd8,			/* G */
    0xd9,			/* B */
    0xda,			/* Unused */
    0xdb,			/* 6 */
    0xdc,			/* Unused */
    0xdd,			/* H */
    0xde,			/* N */
    0xdf,			/* Unused */
    0xe0,			/* 7 */
    0xe1,			/* U */
    0xe2,			/* J */
    0xe3,			/* M */
    0xe4,			/* Unused */
    0xe5,			/* 8 */
    0xe6,			/* I */
    0xe7,			/* K */
    0xe8,			/* , */
    0xe9,			/* Unused */
    0xea,			/* 9 */
    0xeb,			/* O */
    0xec,			/* L */
    0xed,			/* . */
    0xee,			/* Unused */
    0xef,			/* 0 */
    0xf0,			/* P */
    0xf1,			/* Unused */
    0xf2,			/* ; */
    0xf3,			/* / */
    0xf4,			/* Unused */
    0xf5,			/* = */
    0xf6,			/* Unused */
    0xf7,			/* Unused */
    0xf8,			/* Unused */
    0xf9,			/* - */
    0xfa,			/* Unused */
    0xfb,			/* ' */
    0xfc,			/* Unused */
    0xfd,			/* Unused */
    0xfe,			/* Unused */
    0xff			/* Unused */
};
