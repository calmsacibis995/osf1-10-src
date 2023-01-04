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
$ @(#)$RCSfile: apply.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 14:26:28 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDSH) bourne shell and related commands
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.4  com/cmd/sh/apply.msg, 9121320k, bos320 6/15/90 23:27:23


$
$ messages for apply
$
$quote "
$
$set MS_APPLY
$ Do not translate apply

USAGE "usage: apply [-aCharacter] [-Number] Command Argument...\n"
TOOFEWARGS "apply: Specify a parameter after the %s command.\n"
$ %s will be some unix command that the user enters
NOSHELL "apply: Cannot run the environment $SHELL or the shell /usr/bin/sh.\n"
$ Do not translate /usr/bin/sh or $SHELL (it is an environment variable)
NOFORK "apply: The fork function failed.\n"
$ Do not translate fork
