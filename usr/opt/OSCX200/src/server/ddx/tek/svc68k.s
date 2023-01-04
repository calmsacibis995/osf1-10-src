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
/***********************************************************
Copyright 1987 by Tektronix, Beaverton, Oregon,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Tektronix or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

TEKTRONIX DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*	Copyright(c) 1987  Tektronix, Inc.	*/
!
! svc.s --  This file contains the assembly language routines to
!	implement display support on 431x machines.  Includes
!	color, cursor, viewport control.
!
!	$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/svc68k.s,v 1.2 91/12/15 12:42:16 devrcs Exp $";
!

#include "svc.h"
#include <errno.h>

	.data
	.globl	_errno
	.text
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetCursorColor -- Set cursor RGB values
!
! when called:
!	4(sp):	ptr to six shorts for foreground red, green, blue
!			followed by background red, green, blue
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_SetCursorColor
_SetCursorColor:
	movem.l	a0,-(sp)
	move.l	8(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$			!error if null
	move.l	#setCursorColor,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0
	rts
#ifdef not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetCursorColor -- get current cursor RGB values
!
! when called:
!	4(sp):	pointer to six shorts for foreground red, green, blue,
!			followed by background red, green, blue
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		pointer was null
!		display primitive failed
!
	.globl	_GetCursorColor
_GetCursorColor:
	movem.l	a0,-(sp)
	move.l	8(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$
	move.l	#getCursorColor,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0
	rts
#endif not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetColorMap -- Set color map entries
!
! when called:
!	4(sp):	longword which gives size of color map array
!	8(sp):	ptr to an array of color map entries
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_SetColorMap
_SetColorMap:
	movem.l	d1/a0,-(sp)
	move.l	12(sp),d1		!get count
	move.l	16(sp),a0		!get array pointer
	move.l	a0,d0
	beq.s	1$			!error if null
	move.l	#setColorMap,d0		!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,d1/a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetColorMap -- get color map entries
!
! when called:
!	4(sp):	pointer to long to hold size of color map array
!	8(sp):	pointer to array to hold color map
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_GetColorMap
_GetColorMap:
	movem.l	a0/a1,-(sp)		
	move.l	12(sp),a1		!get size pointer
	move.l	a1,d0
	beq.s	1$			!error if arg = null pointer
	move.l	16(sp),a0		!get color map pointer
	move.l	a0,d0
	beq.s	1$			!error if arg = null pointer
	move.l	#getColorMap,d0		!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	move.l	d0,(a1)			!put size in pointer
	clr.l	d0
	movem.l	(sp)+,a0/a1		!restore A0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0/a1		!restore A0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetGrayMap -- Set grayscale map entries
!
! when called:
!	4(sp):	longword which gives size of grayscale map array
!	8(sp):	ptr to an array of grayscale map entries
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_SetGrayMap
_SetGrayMap:
	movem.l	d1/a0,-(sp)
	move.l	12(sp),d1		!get count
	move.l	16(sp),a0		!get array pointer
	move.l	a0,d0
	beq.s	1$			!error if null
	move.l	#setGrayMap,d0		!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,d1/a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/d2
	rts
#ifdef not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetGrayMap -- get grayscale map entries
!
! when called:
!	4(sp):	pointer to long to hold size of grayscale map array
!	8(sp):	pointer to array to hold grayscale map
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_GetGrayMap
_GetGrayMap:
	movem.l	a0/a1,-(sp)		
	move.l	12(sp),a1		!get size pointer
	move.l	a1,d0
	beq.s	1$			!error if arg = null pointer
	move.l	16(sp),a0		!get grayscale map pointer
	move.l	a0,d0
	beq.s	1$			!error if arg = null pointer
	move.l	#getGrayMap,d0		!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	move.l	d0,(a1)			!put size in pointer
	clr.l	d0
	movem.l	(sp)+,a0/a1		!restore A0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0/a1		!restore A0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetColorEntry -- get current value of a single color map entry
!
! when called:
!	4(sp):	pointer to a colormap entry whose index is set.
!			Red, green, blue values of entry are set by routine.
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		color entry pointer was null
!		display primitive failed
!
	.globl	_GetColorEntry
_GetColorEntry:
	movem.l	d1/a0,-(sp)
	move.l	12(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$			!error if arg = null pointer
	move.s	(a0),d1
	move.l	#getColorEntry,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,d1/a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetGrayEntry -- get current value of a single grayscale map entry
!
! when called:
!	4(sp):	pointer to a grayscalemap entry whose index is set.
!			Red, green, blue values of entry are set by routine.
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		grayscale entry pointer was null
!		display primitive failed
!
	.globl	_GetGrayEntry
_GetGrayEntry:
	movem.l	d1/a0,-(sp)
	move.l	12(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$			!error if arg = null pointer
	move.s	(a0),d1
	move.l	#getGrayEntry,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,d1/a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/a0
	rts
#endif not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! LockDisplay -- control access to display registers
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_LockDisplay
_LockDisplay:
	move.l	#lockDisplay,d0		!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0			!indicate no error
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! UnlockDisplay -- allow other processes access to display registers
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_UnlockDisplay
_UnlockDisplay:
	move.l	#unlockDisplay,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0			!indicate no error
	rts
#ifdef not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! LockMap -- control access to color or grayscale map
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_LockMap
_LockMap:
	move.l	#lockMap,d0		!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0			!indicate no error
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! UnlockMap -- allow other processes access to color or grayscale map
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_UnlockMap
_UnlockMap:
	move.l	#unlockMap,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0			!indicate no error
	rts
#endif not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetCursorSourceAndMask -- set cursor forms and offset
!
! when called:
!	4(sp):	ptr to a cursor form structure.
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_SetCursorSourceAndMask
_SetCursorSourceAndMask:
	movem.l	a0,-(sp)
	move.l	8(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$			!error if null
	move.l	#setCursorSourceAndMask,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0
	rts
#ifdef not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetCursorSourceAndMask -- get current forms and offset
!
! when called:
!	4(sp):	pointer to a cursor form structure! both pointers within
!			the structure must be non-null
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		pointer was null
!
	.globl	_GetCursorSourceAndMask
_GetCursorSourceAndMask:
	movem.l	a0,-(sp)
	move.l	8(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$			!error if arg = null pointer
	move.l	4(a0),d0		! check for null pointers in struct
	beq.s	1$
	move.l	8(a0),d0
	beq.s	1$
	move.l	#getCursorSourceAndMask,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!shouldn't happen
	clr.l	d0
	movem.l	(sp)+,a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetCrosshairCursor -- Set crosshair cursor values
!
! when called:
!	4(sp):	ptr to crosshair cursor structure
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_SetCrosshairCursor
_SetCrosshairCursor:
	movem.l	a0,-(sp)
	move.l	8(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$			!error if null
	move.l	#setCrosshairCursor,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetCrosshairCursor -- get current crosshair cursor values
!
! when called:
!	4(sp):	pointer to crosshair cursor structure
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		pointer was null
!
	.globl	_GetCrosshairCursor
_GetCrosshairCursor:
	movem.l	a0,-(sp)
	move.l	8(sp),a0		!get pointer
	move.l	a0,d0
	beq.s	1$			!error if arg = null pointer
	move.l	#getCrosshairCursor,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!shouldn't happen
	clr.l	d0
	movem.l	(sp)+,a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (primitive failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0
	rts
#endif not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetCursorMode -- set which cursor is displayed
!
! when called:
!	4(sp):	cursor mode to set
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		display primitive failed
!
	.globl	_SetCursorMode
_SetCursorMode:
	movem.l	d1,-(sp)
	move.l	8(sp),d1		!get mode
	move.l	#setCursorMode,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	1$			!exit if error
	clr.l	d0
	movem.l	(sp)+,d1
	rts

1$:	move.l	d0,_errno		!load error (primitive failed)
	move.l	#-1,d0
	movem.l	(sp)+,d1
	rts
#ifdef not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetCursorMode -- get current cursor type
!
! when called:
!	no arguments
!
! upon return:
!	d0:	>0 : contains mode
!
! possible errors:
!		none
!
	.globl	_GetCursorMode
_GetCursorMode:
	move.l	#getCursorMode,d0	!put function number in d0
	trap	#DisplayTrapNumber
	rts
#endif not_needed
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetCursorPoint -- Display cursor at x,y.
!	If CursorTrack is True, this is the same as SetMPosition.
!
! when called:
!	4(sp):	ptr to long consisting of x in ms word, y in ls word.
!		(should use "struct POINT !" from graphics.h)
!		Both x and y must valid screen numbers
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		argument pointer was null
!		graphics not initialized
!		x or y outside range of screen
!		display primitive failed
!
	.globl	_SetCursorPoint
_SetCursorPoint:
	movem.l	d1/a0,-(sp)
	move.l	12(sp),a0		!get argument pointer
	move.l	a0,d0			!is pointer valid
	beq.s	1$			! --no, null pointer, error
	move.l	(a0),d1
	move.l	#setCursorPoint,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	movem.l	(sp)+,d1/a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (setCursorPoint failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetCursorBounds -- change the current viewport
!
! when called:
!	4(sp):	pointer to two shorts which describe upper left bound
!	8(sp):	pointer to two shorts which describe lower right bound
!	(should use vsCursor * or struct POINT *).
! 
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_SetCursorBounds
_SetCursorBounds:
	movem.l	d1/d2/a0,-(sp)
	move.l	16(sp),a0		!get ul pointer
	move.l	a0,d0			! and check if null
	beq.s	2$
	move.l	(a0),d1			!load ul x
	move.l	20(sp),a0		!get lr pointer
	move.l	a0,d0			! and check if null
	beq.s	2$
	move.l	(a0),d2			!load lr x
	move.l	#setMouseBounds,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	1$			!exit if error
	clr.l	d0
	movem.l	(sp)+,d1/d2/a0
	rts

1$:	move.l	d0,_errno		!load error (primitive failed)
	bra.s	3$
2$:	move.l	#EINVAL,_errno
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/d2/a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetCursorSpeed -- set amount cursor moves for every mouse movement
!
! when called:
!	4(sp):	ptr to three shorts (threshold, multiplier, divisor)
!
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		argument pointer was null
!		display primitive failed
!
	.globl	_SetCursorSpeed
_SetCursorSpeed:
	movem.l	a0,-(sp)
	move.l	8(sp),a0		!get argument pointer
	move.l	a0,d0			!is pointer valid
	beq.s	1$			! --no, null pointer, error

	move.l	#setCursorSpeed,d0	!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	2$			!exit if error
	clr.l	d0
	movem.l	(sp)+,a0
	rts

1$:	move.l	#EINVAL,_errno		!load error (null pointer)
	bra.s	3$
2$:	move.l	d0,_errno		!load error (setCursorPoint failed)
3$:	move.l	#-1,d0
	movem.l	(sp)+,a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! CursorEnable -- turn the cursor on and start tracking the mouse
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		primitive failure
!
	.globl	_CursorEnable
_CursorEnable:
	move.l	#cursorOn,d0		!turn on cursor
	trap	#DisplayTrapNumber
	move.l	#cursorLink,d0		!track cursor with mouse
	trap	#DisplayTrapNumber
	clr.l	d0			!indicate no error
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! CursorDisable -- turn the cursor off and stop tracking the mouse
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		primitive failure
!
	.globl	_CursorDisable
_CursorDisable:
	move.l	#cursorOff,d0		!turn off cursor
	trap	#DisplayTrapNumber
	move.l	#cursorUnlink,d0	!don't track cursor with mouse
	trap	#DisplayTrapNumber
	clr.l	d0			!indicate no error
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! MousePanOn
!
! when called:
!	none
!
! upon return:
!	d0:	== 0 : sucessful
!
! possible errors:
!		display primitive failed
!
	.globl	_MousePanOn
_MousePanOn:
	move.l	#cursorPanOn,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! DisplayOn --  make the display visible
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_DisplayOn
_DisplayOn:
	move.l	#displayOn,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! DisplayOff --  blank the display
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_DisplayOff
_DisplayOff:
	move.l	#displayOff,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! InvertVideo -- set pixel value of 1 to be white, 0 to black
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_InvertVideo
_InvertVideo:
	move.l	#whiteOnBlack,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! TimeoutOn --  enables automatic screen blanking (screen saver)
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_TimeoutOn
_TimeoutOn:
	move.l	#timeoutOn,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! TimeoutOff --  disables automatic screen blanking (screen saver)
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_TimeoutOff
_TimeoutOff:
	move.l	#timeoutOff,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! GetViewport -- change the current viewport
!
! when called:
!	4(sp):	pointer to long which can hold x in ms word, y in ls.
!	(should use vsCursor * or struct POINT *).
! 
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_GetViewport
_GetViewport:
	movem.l	d1/a0,-(sp)
	move.l	12(sp),a0		!get arg pointer
	move.l	a0,d0			! and check if null
	beq.s	2$
	move.l	#getViewport,d0		!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	1$			!exit if error
	move.l	d0,(a0)			!retrieve x, y pair
	clr.l	d0			!clear return code
	movem.l	(sp)+,d1/a0
	rts

1$:	move.l	d0,_errno		!load error (primitive failed)
	bra.s	3$
2$:	move.l	#EINVAL,_errno
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/a0
	rts

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! SetViewport -- change the current viewport
!
! when called:
!	4(sp):	pointer to long consisting of x in ms word, y in ls.
!	(should use vsCursor * or struct POINT *).
! 
! upon return:
!	d0:	==0 : successful
!		<0  : error, error condition code in _errno
!
! possible errors:
!		null pointer
!		display primitive failed
!
	.globl	_SetViewport
_SetViewport:
	movem.l	d1/a0,-(sp)
	move.l	12(sp),a0		!get arg pointer
	move.l	a0,d0			! and check if null
	beq.s	2$
	move.l	(a0),d1			!load x, y pair
	move.l	#setViewport,d0		!put function number in d0
	trap	#DisplayTrapNumber
	bcs.s	1$			!exit if error
	clr.l	d0			!clear return code
	movem.l	(sp)+,d1/a0
	rts

1$:	move.l	d0,_errno		!load error (primitive failed)
	bra.s	3$
2$:	move.l	#EINVAL,_errno
3$:	move.l	#-1,d0
	movem.l	(sp)+,d1/a0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! JoyPanOn --  enable joydisk panning
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_JoyPanOn
_JoyPanOn:
	move.l	#joyPanOn,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! JoyPanOff --  disable joydisk panning
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.globl	_JoyPanOff
_JoyPanOff:
	move.l	#joyPanOff,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0
	rts

!------------------------------------------------------------------------------!
!
! ProtectDestination -- tell the OS that the user will be drawing or reading
!	directly on the screen in the defined "destination" rectangle.
!	There are only two simultaneously protected rectangles:
!	the "source" rectangle and the "destination" rectangle.
!	It should therefore get the cursor out of the way.
!
! when called: ProtectDestination (Xmin, Ymin, Xmax, Ymax)
!	2 longs for x,y upper left corner.
!	2 longs for x,y lower right corner.
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!	None.
!
	.base	0
RegD1:   .zerol 1
return_addr: .zerol 1
Xmin:    .zerol 1
Ymin:    .zerol 1
Xmax:    .zerol 1
Ymax:    .zerol 1

	.text
	.globl	_ProtectDestination
_ProtectDestination:
	movem.l	d2,-(sp) ! protect registers (D0/D1/A0/A1 c assumes are crashed)

	move.l	Xmin(sp),d1		! fetch Xmin
	Swap	d1			! put into XY form.
	move.l	Ymin(sp),d2		!
	move.w	d2,d1			! now I have XYmin.

	move.l	Xmax(sp),d2		! fetch Xmax
	Swap	d2			! put into XY form.
	move.l	Ymax(sp),d0		!
	move.w	d0,d2			! now I have XYmax.

	move.l	#setDest,d0		!put function number in d0, d1, d2
	trap	#DisplayTrapNumber
	bcs.s	07$			!exit if error

02$:	clr.l	d0			!indicate no error
99$:	movem.l	(sp)+,d2		!restore
	rts

!
!---- this should not be reached ----
!
07$:	move.l	d0,_errno		!load error (primitive failed)
	move.l	#-1,d0			!
	Bra.s	99$
!------------------------------------------------------------------------------!
!
! ProtectSource -- tell the OS that the user will be drawing or reading
!	directly on the screen in the defined "source" rectangle.
!	There are only two simultaneously protected rectangles:
!	the "source" rectangle and the "destination" rectangle.
!	It should therefore get the cursor out of the way.
!
! when called: ProtectSource (Xmin, Ymin, Xmax, Ymax)
!	2 longs for x,y upper left corner.
!	2 longs for x,y lower right corner.
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!	None.
!
	.base	0
RegD1:   .zerol 1
return_addr: .zerol 1
Xmin:    .zerol 1
Ymin:    .zerol 1
Xmax:    .zerol 1
Ymax:    .zerol 1

	.text
	.globl	_ProtectSource
_ProtectSource:
	movem.l	d2,-(sp) ! protect registers (D0/D1/A0/A1 c assumes are crashed)

	move.l	Xmin(sp),d1		! fetch Xmin
	Swap	d1			! put into XY form.
	move.l	Ymin(sp),d2		!
	move.w	d2,d1			! now I have XYmin.

	move.l	Xmax(sp),d2		! fetch Xmax
	Swap	d2			! put into XY form.
	move.l	Ymax(sp),d0		!
	move.w	d0,d2			! now I have XYmax.

	move.l	#setSource,d0		!put function number in d0, d1, d2
	trap	#DisplayTrapNumber
	bcs.s	07$			!exit if error

02$:	clr.l	d0			!indicate no error
99$:	movem.l	(sp)+,d2		!restore
	rts

!
!---- this should not be reached ----
!
07$:	move.l	d0,_errno		!load error (primitive failed)
	move.l	#-1,d0			!
	Bra.s	99$

!------------------------------------------------------------------------------!
!
! ProtectCursorCompleted -- tell the OS that the cursor may be replaced in
!	either the "source" rectangle protected by the ProtectSource()
!	routine or the "destination" rectangle protected by the
!	ProtectDestination() routine.
!
! when called:
!	none
!
! upon return:
!	d0:	==0 : successful
!
! possible errors:
!		none
!
	.text
	.globl	_ProtectCursorCompleted
_ProtectCursorCompleted:
	move.l	#updateComplete,d0	!put function number in d0
	trap	#DisplayTrapNumber
	clr.l	d0			!indicate no error
	rts


