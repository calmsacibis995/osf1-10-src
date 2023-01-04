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
$ @(#)$RCSfile: wc.msg,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/05 14:34:46 $
$
$
$ HISTORY
$
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.11  com/cmd/files/wc.msg, bos320 4/26/91 10:37:38
$
$quote "
$
$ $ Message numbers 755 - 764 reserved for this file.
$
$set MS_WC

USAGE "Usage: wc [-clmw] [File ...]\n"
$ The above is the usage statement.
CANTOPEN "wc: %s : %s.\n"
$ %1$s is the file name. Second %s is the error message from the kernel
EREAD                "wc: read error on %s.\n"
TOTAL " total\n"
$ When the user uses the wc command on more than one file at a time, the
$ word "total" appears to the right of the total characters, words, and lines
$ of all files specified.
$ The leading space is required.
