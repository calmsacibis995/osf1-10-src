
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

$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$quote "
$set MS_SWAPON

USAGE		"usage: %s [-avp] [-l size] [-h size] filename\n"
NO_AUTH		"%s: need mount authorization.\n"
PRIV		"%s: insufficient privileges.\n")
NO_DEV		"%s: %s: device not configured.\n"
NOT_SUPPORTED	"%s: warning: options 'p', 'l', and 'h' not supported.\n"
IN_USE		"%s: %s: device already in use.\n"
VERBOSE1        "%s: adding %s as swap device.\n"
VERBOSE2	"%s: adding %s as %spaging file: low water mark %u, high water mark %u\n"
OVERLAP		"%s: New swap area would overlap mounted filesystem(s)\n\tor another active swap area\n"
UMOUNT_REQ	"%s: Unmount required before swapping on %s (start %d end %d)\n"
START_END	"\t%s %s %s (start %d end %d)\n"

$ Be sure to leave the trailing space
$
PREFERED	"prefered "
