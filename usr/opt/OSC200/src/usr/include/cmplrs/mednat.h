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
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/cmplrs/mednat.h,v 4.2.4.2 1992/04/30 15:58:24 Ken_Lesniak Exp $ */
#ifndef _CMPLRS_MEDNAT_H
#define _CMPLRS_MEDNAT_H

/*#define MEDNAT	1	/* define MEDNAT flag		*/

#ifdef _MEDNAT

#define		flush(err);		;
#define		flush(stderr);		;
#define		flush(output);		;
#define		flush(dumpfile);	;

#define		maxint		maxlong

#define		lshift(_a,_b)	((_a) << (_b))
#define		rshift(_a,_b)	((_a) >> (_b))

type
    integer = int32;
    double = longreal;
    pointer = extaddr;
    cardinal = card32;

function  max(a,b : integer) : integer; external;
function  min(a,b : integer) : integer; external;
function  argc : integer; external;
procedure argv( i : integer; s : fstring); external;

#define bitand(_a,_b) ((_a) And (_b))
#define bitxor(_a,_b) ((_a) Xor (_b))
#define bitor(_a,_b) ((_a) Or (_b))
#define bitnot(_a) (Not (_a))

#else

#define		stderr			err

#endif

#endif
