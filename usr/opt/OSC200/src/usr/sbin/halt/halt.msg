 
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
$ RESTRICTED RIGHTS LEGEND
$ Use, Duplication or Disclosure by the Government is subject to
$ restrictions as set forth in paragraph (b)(3)(B) of the rights in
$ Technical Data and Computer Software clause in DAR 7-104.9(a).
$ 
$ @(#)$RCSfile: halt.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 93/02/16 18:04:03 $
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
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

$
$ messages for halt
$

$set MS_HALT

$quote "

NO_ROOT "NOT super-user\n"
USAGE	"usage: halt [-y][-q][-l][-n]\n"
DIALUP	"halt: dangerous on a dialup; use ``halt -y'' if you are really sure\n"
IDLE	"halt: can't idle init\n"
LOGHLT	"halted by %s"
CAUTION	"CAUTION: some process(es) wouldn't die\n"
UTMPZ	"Couldn't open /var/adm/utmp\n"
HALTOVER	"....Halt completed....\n"

$set MS_HALT_SEC

AUTH	"%s: need sysadmin authorization\n"
PRIV	"%s: insufficient privileges\n"
