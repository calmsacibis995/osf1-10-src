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
$ HISTORY
$ 


$quote "
$set MS_STRERR

STRER_USAGE	"usage: strerr [-a sys_admin_mail_name] [-d logdir]\n"
STRER_NOLOG	"couldn't open log device"
STRER_EALREADY	"ERROR: strerr already running\n"
STRER_FMT1	"%s/error.%02d-%02d"
STRER_DAEMON	"daemon"
STRER_MAIL	"/usr/bin/mail %s"
STRER_MKDIR     "strerr: couldn't mkdir errlog directory"
STRER_STAT      "strerr: stat on errlog directory"
