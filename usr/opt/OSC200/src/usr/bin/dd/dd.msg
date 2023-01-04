 
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
$ (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0.1

$ @(#)$RCSfile: dd.msg,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/18 14:43:17 $

$ @(#)dd.msg	1.5  com/cmd/arch,3.1,9021 3/21/90 21:24:57
$
$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_DD
$ Do not translate dd

RECSIN "dd:  %1$u+%2$u records in.\n"
RECSOUT "dd:  %1$u+%2$u records out.\n"

TRUNRECS "dd:  %u are truncated records.\n"

NEXTVOL "dd:  Insert next volume and press RETURN key. [q]"

EOM "dd: \007Reached end of medium; Please wait for file closing\n"

NOPROMPT "dd:  Cannot prompt (can't open %s): "

EREAD "dd:  read error"
$ Do not translate read

EWRITE "dd:  write error"
$ Do not translate write

ECLOSE "dd:  close error"

ESPARSE "dd:  seek error"

ESKIP "dd:  skip error"

EARG "dd:  The command parameter %s is not correct.\n\
\tUsage: dd [if=InputFile] [of=OutputFile] [cbs=Number] [fskip=Number]\n\
\t          [skip=Number] [seek=Number] [count=Number] [ibs=Number]\n\
\t          [obs=Number] [conv=Parameter[, ...]]\n"

EBLKSIZ "dd:  Block sizes cannot be zero.\n"

ENOMRY "dd:  There is not enough memory available now.\n"

EBUFNOMRY "dd:  There is not enough memory available for internal dd buffers.\n"

ESTOMP "dd:  Can't copy file onto itself.\n"

EPART	"dd: Unable to write complete record.\n"

ETHREAD "dd: Unable to create thread.\n"

ETAPE  "dd: Error in repositioning the tape backward.\n"

ESEEK  "dd: Unable to perform seek.\n"

ESYNCINIT "dd: Unable to initialize synchronization object.\n"

ESYNCLOCK "dd: Unable to lock synchronization object.\n"

ESYNCTRYLOCK "dd: Unable to trylock synchronization object.\n"

ESYNCUNLOCK "dd: Unable to unlock synchronization object.\n"

ECANCEL "dd: Unable to cancel thread.\n"

EINVCHAR "dd: Invalid character: %c.\n"

