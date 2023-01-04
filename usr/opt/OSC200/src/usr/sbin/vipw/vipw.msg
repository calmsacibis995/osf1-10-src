 
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
$ @(#)$RCSfile: vipw.msg,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:16:23 $
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
$ messages for vipw
$

$set MS_VIPW

$quote "

BUSY           "vipw: password file busy\n"
VIPW           "vipw: "
STAT           "vipw: can't stat temp file, %s unchanged\n"
BADTEMP        "vipw: bad temp file, %s unchanged\n"
REOPEN         "vipw: can't reopen temp file, %s unchanged\n"
MKPASS         "vipw: mkpasswd failed\n"
MANGLED        "vipw: you mangled the temp file, %s unchanged\n"
EDMSG          "Unable to find vi editor using ed.\n"
EDMSG2         "Enter q to exit ed\n"
BADSHELL	"vipw: %s entry has an invalid shell\n"
NOSHELLS	"vipw: Warning, unable to open /etc/shells\n"
ASKMKPW		"Hashed passwd database does not exist, create? [yes]> "
YES		"yes"
