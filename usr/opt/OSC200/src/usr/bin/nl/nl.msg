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
$ @(#)$RCSfile: nl.msg,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/09/24 03:00:43 $
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
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.9  com/cmd/files/nl.msg, cmdfiles, bos320, 9125320 5/29/91 19:29:36
$
$quote "
$
$set MS_NL
$ Do not translate nl

HEADER1 "Header: "
$ Do not translate
BODY1 "Body: "
$ Do not translate
FOOTER1 "Footer: "
$ Do not translate
CANTOPEN "nl: Cannot open %s.\n"
REGERR1 "nl: Regular expression error code %d.\n"
REGERR2 "\tThe regular expression is not valid.\n"
OPTINVAL "nl: %s is not a recognized flag.\n\
\tUsage: nl [-b Type] [-d Delimiter1Delimiter2] [-f Type] [-h Type]\n\
\t[-i Number] [-l Number] [-n Format] [-p] [-s Separator]\n\
\t[-v Number] [-w Number] [File]\n"
SEPTOOLONG "nl: SEPARATOR TOO LONG - PROCESSING TERMINATED\n"
NOMEM "nl: Out of memory.\n"
INVDELIM1 "nl: Invalid 1st delimiter.\n"
INVDELIM2 "nl: Invalid 2nd delimiter.\n"
MISSARG "nl: The option '%c' requires a parameter.\n"
