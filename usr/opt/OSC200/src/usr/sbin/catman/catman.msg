 
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
$ HISTORY
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$ 
$ @(#)$RCSfile: catman.msg,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/11/24 10:25:42 $
$ 
$ 
$ 	COMPONENT_NAME: (CMDMAN) commands that allow users to read online
$ 	                         documentation
$ 
$ 	FUNCTIONS: none
$ 
$ 	ORIGINS: 27
$ 
$ 	IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ 	combined with the aggregated modules for this product)
$ 	OBJECT CODE ONLY SOURCE MATERIALS
$ 	(C) COPYRIGHT International Business Machines Corp. 1989
$ 	All Rights Reserved
$ 
$ 	US Government Users Restricted Rights - Use, duplication or
$ 	disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ 	catman.msg\t1.5  com/cmd/man,3.1,9021 3/4/90 17:16:49
$ 
$ Maintenance History
$ -------------------
$
$ 001	gws	(OSF_QAR 1812)
$	inserted "-Tlp" following "nroff" in NROFF message, to be consistent
$	with catman.c, and because NROFF is the real command string when
$	LANG is defined.
$
$	added trailing "Do not translate catman" after STATUS message
$
$ 002	gws 11/04/91	(OSF_QAR 1695)
$ 	changed NROFF message, so that manpages sources are now preprocessed
$	through the tbl(1) and neqn(1) commands, and post-processed through the
$	col(1) command with -p option, when the LANG environent variable is
$	defined, because NROFF is the real command string.
$
$	If the nroff step has an error, no output file is created.
$
$	Positional formats (%digit$) are removed from the NROFF message too.
$
$ 003	gws 11/27/91	(OSF_QARs 2217, 2253)
$	changed NROFF message to remove postprocessing of manpages through
$	  "col -p", because col(1) command counts the Escape codes as text
$	  that "-p" passes through, so col miscalculates no. of tabs to
$	  generate when it converts spaces into tabs by default.  Also, col is
$	  NOT required to make tables vertically alligned, when the "-Tlp"
$	  device is associated with a device that has reverse-linefeed AND
$	  the "-Tlp" device is also the default terminal display device for
$	  the system.
$	add the -h option to the nroff call in NROFF message, because this
$	  generates tabs, and the "-Tlp" default device needs the tabs to step
$	  over left columns in multicolumn tables containing text blocks,
$	  else the left columns get wiped out.  This also makes catman
$	  consistent with the man command, which was already using nroff -h
$	  for creating cat? files.
$

$quote "         define quote character.
$ messages for catman
$len
$set MS_CATMAN        Define initial set#


MISSPATH             "catman: -M option missing path\n"
$ Do not move the %s from its present position.

USAGE                "Usage: catman [-p] [-n] [-w] [-M path] [sections]\n"
MANDIR               "catman: %s"
$ Do not translate.

CDMANDIR             "cd %s\n"
$ Do not translate.

OPENDIR              "catman: opendir: %s"
$ Do not translate.

MKDIR                "mkdir %s\n"
$ Do not translate.

MKDIRCAT             "catman: mkdir: %s"
$ Do not translate.

CAT                  "catman: %s"
$ Do not translate.

NOTDIR               "catman: %s: not a directory\n"
$ Do not translate catman.

LNCAT                "ln -s %1$s %2$s\n"
$ Do not translate.

SYMLINK              "catman: symlink: %s"
$ Do not translate.

$								  start * 002 *
$				       added nroff "-h"; removed col -p * 003 *
NROFF                "tbl %s | neqn | nroff -Tlp -man -h - > %s \
|| (stat=$? ; rm %s ; exit $stat)"
$ Do not translate.				   * -Tlp 001 * * 002 * * 003 *

BADMAN               "catman: nroff: %1$s: The exit status is %2$d\n"
$ Do not translate nroff, catman.

STATUS               "catman: %s: The exit statis is %d\n"
$ Do not translate catman.

$ start * 004 *
PERMISSION               "catman: Check write permissions or su NFS permissions for %s: \n"
$ Do not translate catman.
$ end  * 004 *
