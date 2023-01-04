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
$ @(#)$RCSfile: ln.msg,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/06/10 22:56:18 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ Messages for the ln command.
$
$quote "
$set MS_LN

FOPT		"%s exists, specify -f to remove.\n"

DIFFFS          "ln: %s and %s are located on different file systems.\n"

NOPERM		"ln: insufficient permission for %s.\n"

ISDIR		"%s is a directory.\n"

IDENTICAL	"ln: %s and %s are identical.\n"

USAGE 	"usage:\tln [-fs] sourcename [targetname]\n\tln [-fs] sourcename1 sourcename2 [... sourcenameN] targetdirectory\n"
S5USAGE 	"usage:\tln [-fs] targetname [sourcename]\n\tln [-fs] targetname1 targetname2 [... targetnameN] sourcedirectory\n"
QUERY	"%s  is not writeable, mode %o: override %s or %s ? "
