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
$ HISTORY
$
$ @(#)$RCSfile: ctags.msg,v $ $Revision: 1.1.2.2 $ (OSF) $Date: 1993/08/25 22:26:25 $


$set MS_CTAGS
$quote "

$ messages for the ctags command
$

$ 	catgs.c message
$
USAGE		"Usage: ctags [-BFadtuwv] [-x | -f tagsfile] file ..."

$	print.c message
$
NOSPACE		"ctags: no more space.\n"

$	tree.c messages
$
TOOMANY		"ctags: too many entries to sort\n"
NOSPACE2	"ctags: out of space.\n"
DUPENTRY   "Duplicate entry in file %s, line %d: %s\nSecond entry ignored\n"
DUPENTRY2	"Duplicate entry in files %s and %s: %s (Warning only)\n"

