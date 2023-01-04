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
$ @(#)$RCSfile: pr.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 21:06:03 $
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
$ 1.7  com/cmd/files/pr.msg, cmdfiles, bos320, 9126320 6/3/91 12:57:34
$
$quote "
$
$set MS_PR

PAGE            "Page"
BADOPT          "The specified flag is not correct."
EMPTYF          "The file %s is empty."
NOCORE          "There is not enough memory available now."
PBOVRFL         "Internal error. A page-buffer overflow has occurred."
TOOMANYF        "Too many files are specified."
WDTHSMALL       "The width specified with the -w flag is too small."
CANTOPEN	"File %s cannot be opened."
$ The file cannot be opened for reading. Either the file does not exist or
$ the user does not have read permission for the file.
USAGE           "Usage: pr [-adfFmprt] [-e[Character][Gap]]\n\
\t\t   [-h \"Header\"] [-i[Character][Gap]] [-l Lines]\n\
\t\t   [-n[Character][Width]] [-o Offset] [-s[Character]] [-w Width]\n\
\t\t   [-Column] [+Page] [File...]\n"
$
DATE_FMT	"%b %e %H:%M %Y"
$ This is a strftime() format string and should be 'translated' to
$ provide the correct date format in the locale
BAD_OPT		"pr: illegal option -- %c\n"
ARG_REQ		"pr: option requires an argument -- %c\n"
$ printed when option parsing goes wrong
$ do not translate pr
