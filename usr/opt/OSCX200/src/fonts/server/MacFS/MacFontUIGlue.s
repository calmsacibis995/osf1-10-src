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
#
#Copyright 1991 by Apple Computer, Inc, Cupertino, California
#			All Rights Reserved
#
#Permission to use, copy, modify, and distribute this software
#for any purpose and without fee is hereby granted, provided
#that the above copyright notice appear in all copies.
#
#APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
#OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
#PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
#PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
#AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
#QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
#FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
#DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.
#
#THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
#AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
#IMPLIED.
#
	text

	global AsmClikLoop
AsmClikLoop:

#	movem.l	%d1/%d2/%a1,-(%sp)		# d0 and a0 need not be saved
	jsr	GetOldClikLoop			# get the old clikloop
#	movem.l	(%sp)+,%d1/%d2/%a1		# restore the world as it was

	jsr	(%a0)				# and execute old clikloop

#	movem.l	%d1/%d2/%a1,-(%sp)		# d0 and a0 need not be saved
	jsr	CClikLoop			# do our clikloop
#	movem.l	(%sp)+,%d1/%d2/%a1		# restore the world as it was
	move.l	&1,%d0			# clear the zero flag so textedit keeps going
	rts


# A/UX C doesn't now how to generate a Pascal calling sequence for a ROM
# call-back routine.  So this routine takes the place of the VActionProc
# declared in the MacFontUI.c source.  This code, in turn, translates the
# Pascal calling sequence into the C calling sequence and calls the 
# VActionProc in MacFontUI.c, now called CVActionProc
# void		VActionProc (theControl,partCode)
# ControlHandle	theControl;
# short		partCode;

	global	VActionProc
VActionProc:
	move.w	4(%a7),-(%a7)
	clr.w	-(%a7)
	move.l	10(%a7),-(%a7)
	jsr	CVActionProc		# Call the C version
	move.l	8(%a7),%a1
	add.l	&18,%a7
	jmp	(%a1)

# A/UX C doesn't now how to generate a Pascal calling sequence for a ROM
# call-back routine.  So this routine takes the place of the HActionProc
# declared in the MacFontUI.c source.  This code, in turn, translates the
# Pascal calling sequence into the C calling sequence and calls the 
# HActionProc in MacFontUI.c, now called CHActionProc
# void		HActionProc (theControl,partCode)
# ControlHandle	theControl;
# short		partCode;

	global	HActionProc
HActionProc:
	move.w	4(%a7),-(%a7)
	clr.w	-(%a7)
	move.l	10(%a7),-(%a7)
	jsr	CHActionProc		# Call the C version
	move.l	8(%a7),%a1
	add.l	&18,%a7
	jmp	(%a1)

