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
$ @(#)$RCSfile: cp.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 16:15:55 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ Messages for the cp command.
$quote "
$set MS_CP

RANDR		"cp: the -R and -r options are mutually exclusive.\n"
SPACE		"%s: out of space.\n"
SAME		"%s: %s and %s are identical (not copied).\n"
ISDIR		"%s: %s is a directory (not copied).\n"
NOTDIR		"%s: %s: not a directory.\n"
OVER		"overwrite %s? "
CANTREAD	"%s: can't read directory %s.\n"
TOLNG		"%s: %s: name too long.\n"
TOLNG2		"%s: %s/%s: name too long.\n"
ACL		"%s: cannot propagate ACL for %s\n"
USAGE		"usage: cp [-fhip] [--] source_file destination_file\n   or: cp [-fhip] [--] source_file ...  destination_directory\n   or: cp [-fhip] [-R | -r] [--]\n		  [source_file | source_directory] ... destination_directory\n"
