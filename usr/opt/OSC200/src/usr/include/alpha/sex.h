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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/include/alpha/sex.h,v 1.1.4.2 1993/12/15 22:12:59 Thomas_Peterson Exp $ */

/*
 * This file contains macro constant names for byte sex flags, the macros for
 * byte swapping words and half words, and the external declarations for the
 * routines in libsex.a which change the sex of structures that appear
 * in object files.
 */

/*
 * Byte sex constants
 */
#define BIGENDIAN	0
#define LITTLEENDIAN	1
#define UNKNOWNENDIAN	2

/*
 * Byte swaps for word and half words.
 */
#if defined(__mips64) || defined(__alpha)
#define swap_word(a) ( ((a) << 24) | \
		      (((a) << 8) & 0x00ff0000) | \
		      (((a) >> 8) & 0x0000ff00) | \
	((unsigned int)(a) >>24) )
#else
#define swap_word(a) ( ((a) << 24) | \
		      (((a) << 8) & 0x00ff0000) | \
		      (((a) >> 8) & 0x0000ff00) | \
	((unsigned long)(a) >>24) )
#endif

#define swap_half(a) ( ((a & 0xff) << 8) | ((unsigned short)(a) >> 8) )

#ifdef __cplusplus
extern "C" {
#endif

extern
int
gethostsex();

extern
void
swap_filehdr();

extern
void
swap_aouthdr();

extern
void
swap_scnhdr();

extern
void
swap_hdr();

extern
void
swap_fd();

extern
void
swap_fi();

extern
void
swap_sym();

extern
void
swap_ext();

extern
void
swap_pd();

extern
void
swap_opt();

extern
void
swap_aux();

extern
void
swap_line();

extern
void
swap_reloc();

extern
void
swap_ranlib();

extern
void
swap_gpt();

#ifdef __cplusplus
}
#endif
