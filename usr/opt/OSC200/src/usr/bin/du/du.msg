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
$ @(#)$RCSfile: du.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 19:03:11 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$












$ FUNCTIONS: 
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ du.msg	1.5  com/cmd/fs/progs,3.1,9013 3/6/90 13:02:25"
$
$quote "
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate du -- this is the command name.
$ Do not translate %s.
$set MS_DU
$
USAGE "usage: du [-a|-s] [-klrx] [name ...]\n"
$
CANTFIND "du: Cannot find the current directory.\n"
$
NAMETOOLONG "du: %s\nThe specified path name exceeded 255 bytes.\n"
$
$ DO NOT move the %s from where it is. It will be a very long name, and
$ will "wrap".
TOODEEP	"du: %s\nThe generated path name exceeded 255 bytes.\n"

NODOTDOT "du: Cannot change directory to ../%s \n"
$ Do not translate "..". This is a part of the path.

NOMEM	"Out of memory"
