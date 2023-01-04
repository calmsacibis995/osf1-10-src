$
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ @(#)$RCSfile: od.msg,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:48:35 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$

$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ od.msg	1.9  com/cmd/scan,3.1,9013 3/4/90 17:34:34
$
$quote "         define quote character.
$len
$set MS_OD        Define initial set#
$	Do not translate od, %s, %c.

BADFLAG	"%s: -%c is not a recognized flag.\n"
$       %s MUST stay at the start of the message.
$       Do not translate %s and -%c.
USAGE	"usage: %s [-bCcdfhiloQs[n]vw[n]x] [file] [+]offset[.|b|B] [+]label[.|b|B]\n"
USAGE1	"       %s [-v] [-A address_base] [-j skip] [-N count] [-t type_string] [file...]\n"
$ $ Check the usage statement.
$       Do not translate %s and all the flags.
AOPT  "-A option only accepts the following:  d, o, n, and x\n"
INVNUM "Invalid number of bytes to interpret\n"
DOPT  "d may only be followed with C, S, I, L, 1, 2, or 4\n"
FOPT  "f may only be followed with F, D, L, 4, or 8\n"
OOPT  "o may only be followed with C, S, I, L, 1, 2, or 4\n"
UOPT  "u may only be followed with C, S, I, L, 1, 2, or 4\n"
XOPT  "x may only be followed with C, S, I, L, 1, 2, or 4\n"
INVOFF "Invalid offset\n"
TOOMANY	"Too many conversions specified\n"
TOOBIG	"-%c value too large\n"
$ %c is either 'w' or 's' - option value overflowed numeric
$ representation.
