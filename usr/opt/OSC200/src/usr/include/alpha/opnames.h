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
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/alpha/opnames.h,v 1.1.2.2 1992/04/01 15:41:32 Ken_Lesniak Exp $ */

#if !defined(__alpha) && !defined(__mips64)
extern char *op_name[64];
extern char *spec_name[64];
extern char *bcond_name[32];
extern char *cop1func_name[64];
extern char *bc_name[32];
extern char *c0func_name[64];
extern char *c0reg_name[32];
#else /* defined(__alpha) */
extern char *op_name[64];
extern enum { PAL, OP, FOP, MEM, MEM1, FMEM, JSR, BRA, FBRA, ILL } format[64];
extern char *pal_name[192];
extern char *vfop_name[64];
extern char *ifop_name[64];
extern char *gfop_name[64];
extern char *fmod_name[32];
extern char vmod_form[64];
extern char imod_form[64];
#endif /* defined(__alpha) */
