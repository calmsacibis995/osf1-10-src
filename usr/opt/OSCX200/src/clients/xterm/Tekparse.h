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
 *	$XConsortium: Tekparse.h,v 1.4 91/01/06 12:46:47 rws Exp $
 */

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


/* @(#)Tekparse.h	X10/6.6	11/7/86 */

/*
 * The following list of definitions is generated from Tekparse.def using the
 * following command line:
 *
 *     egrep -v '^CASE_' Tekparse.def | \
 *     awk 'BEGIN {n = 0;} {printf "#define %s %d\n", $1, n; n++}' 
 *
 * You you need to change something, change Tekparse.def and regenerate the
 * definitions.  This would have been automatic, but since this doesn't change
 * very often, it isn't worth the makefile hassle.
 */

#define CASE_REPORT 0
#define CASE_VT_MODE 1
#define CASE_SPT_STATE 2
#define CASE_GIN 3
#define CASE_BEL 4
#define CASE_BS 5
#define CASE_PT_STATE 6
#define CASE_PLT_STATE 7
#define CASE_TAB 8
#define CASE_IPL_STATE 9
#define CASE_ALP_STATE 10
#define CASE_UP 11
#define CASE_COPY 12
#define CASE_PAGE 13
#define CASE_BES_STATE 14
#define CASE_BYP_STATE 15
#define CASE_IGNORE 16
#define CASE_ASCII 17
#define CASE_APL 18
#define CASE_CHAR_SIZE 19
#define CASE_BEAM_VEC 20
#define CASE_CURSTATE 21
#define CASE_PENUP 22
#define CASE_PENDOWN 23
#define CASE_IPL_POINT 24
#define CASE_PLT_VEC 25
#define CASE_PT_POINT 26
#define CASE_SPT_POINT 27
#define CASE_CR 28
#define CASE_ESC_STATE 29
#define CASE_LF 30
#define CASE_SP 31
#define CASE_PRINT 32
#define CASE_OSC 33
