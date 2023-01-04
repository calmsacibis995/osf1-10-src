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
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/cmplrs/f_errno.h,v 4.2.4.2 1992/04/30 15:57:41 Ken_Lesniak Exp $ */
/*
 *
 * f77 I/O error definitions
 */

#include	<errno.h>

extern int errno;

#define F_ER		100	/* base offset of f77 error numbers */

#define F_ERFMT		100	/* error in format */
#define F_ERUNIT	101	/* illegal unit number */
#define F_ERNOFIO	102	/* formatted io not allowed */
#define F_ERNOUIO	103	/* unformatted io not allowed */
#define F_ERNODIO	104	/* direct io not allowed */
#define F_ERNOSIO	105	/* sequential io not allowed */
#define F_ERNOBKSP	106	/* can't backspace file */
#define F_ERNFILE	107	/* null file name */
#define F_ERSTAT	108	/* can't stat file */
#define F_ERNCON	109	/* unit not connected */
#define F_EREREC	110	/* off end of record */
#define F_ERTRUNC	111	/* truncation failed */
#define F_ERLIO		112	/* incomprehensible list input */
#define F_ERSPACE	113	/* out of free space */
#define F_ERNOPEN	114	/* unit not connected */
#define F_ERRDCHR	115	/* read unexpected character */
#define F_ERLOGIF	116	/* blank logical input field */
#define F_ERBVN		117	/* bad variable name */
#define F_ERNLN		118	/* bad namelist name */
#define F_ERVNL		119	/* variable not in namelist */
#define F_ERNER		120	/* no end record */
#define F_ERVCI		121	/* variable count incorrect */
#define F_ERNREP	122	/* negative repeat count */
#define F_ERILLOP	123	/* illegal operation for channel or device */
#define F_ERBREC	124	/* off beginning of record */
#define F_ERREPT	125	/* no * after repeat count */
#define F_ERNEWF	126	/* 'new' file exists */
#define F_EROLDF	127	/* can't find 'old' file */
#define F_ERSYS		128	/* unknown system error */
#define F_ERSEEK	129	/* requires seek ability */
#define F_ERARG		130	/* illegal argument */
