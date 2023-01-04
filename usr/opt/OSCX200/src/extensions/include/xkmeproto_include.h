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
/***********************************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts.
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

************************************************************************/
/*
**++
**  FACILITY:
**
**	common struct
**
**  ABSTRACT:
**
**	It contains protocol definition for the keyboard management extension.
**      (KME).
**	The KME extension name string and all relevant protocol definition for
**	I18N keyboard mode-switching are defined here.
**
**  ENVIRONMENT:
**
**	Ultrix	
**
**  MODIFICATION HISTORY:
**
**--
**/
#ifndef XKMEPROTO_INCLUDE_H
#define XKMEPROTO_INCLUDE_H

#define KMEXTENSIONNAME	    "Keyboard-Management-Extension"

/* constant returned by XKME routines indicating that the target server
 * version does not honor the request */
#define KMEBADREQUEST 2

/* Opcodes to set and reset keyboard mode-switch */
#define X_KMEDoKBModeSwitch	10

/* command mode constants for switching the keyboard mode */
#define LockDownModeSwitch	1
#define UnlockModeSwitch	2


/* reply constants */
#define KBModeSwitchSuccess	0   /* kb mode successfully switched */
#define KBModeSwitchFailure	1   /* kb mode failed to switch */
#define KBModeSwitchInvalidCmd  2   /* invalid kb mode-switch command */
#define KBModeSwitchNoop        3   /* Keyboard is already in the request mode*/


/* 
 * X_KMEDoKBModeSwitch request packet def
 */
typedef struct {
    CARD8 reqType;
    CARD8 minor_opcode;
    CARD16 length B16;
    CARD32 mode B32;	    /* LockDownModeSwitch or UnlockModeSwitch */
} xKMEDoKBModeSwitchReq; 

/*
 * X_KMEDoKBModeSwitch reply packet definition 
 */
typedef struct {
    BYTE type;
    BYTE data1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 status B32;	    /* 0 = SUCCESS, 1 = FAILURE */
    CARD32 pad1   B32;
    CARD32 pad2  B32;
    CARD32 pad3  B32;
    CARD32 pad4  B32;
    CARD32 pad5  B32;
}xKMEDoKBModeSwitchRep;

#define sz_xKMEDoKBModeSwitchReq (sizeof(xKMEDoKBModeSwitchReq))
#define sz_xKMEDoKBModeSwitchRep (sizeof(xKMEDoKBModeSwitchRep))

#endif /* XKMEPROTO_INCLUDE_H */
