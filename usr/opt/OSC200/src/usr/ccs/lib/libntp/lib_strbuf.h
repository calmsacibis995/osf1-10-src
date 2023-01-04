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
 *	@(#)$RCSfile: lib_strbuf.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:40:56 $
 */
/*
 */

/*
 * lib_strbuf.h - definitions for routines which use the common string buffers
 */

/*
 * Sizes of things
 */
#define	LIB_NUMBUFS	20
#define	LIB_BUFLENGTH	80

/*
 * Macro to get a pointer to the next buffer
 */
#define	LIB_GETBUF(buf) \
	do { \
		buf = &lib_stringbuf[lib_nextbuf][0]; \
		if (++lib_nextbuf >= LIB_NUMBUFS) \
			lib_nextbuf = 0; \
	} while (0)

extern char lib_stringbuf[LIB_NUMBUFS][LIB_BUFLENGTH];
extern int lib_nextbuf;
