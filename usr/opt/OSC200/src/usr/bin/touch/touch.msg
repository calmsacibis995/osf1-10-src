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
$ @(#)$RCSfile: touch.msg,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/09/03 03:41:21 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ touch.msg	1.6  com/cmd/scan,3.1,9013 3/4/90 17:36:01
$
$quote "         define quote character.
$len
$set MS_TOUCH        Define initial set#
$ Do not translate touch, %s.

TUSAGE               "Usage: touch [-acfm] [-r ref_file |-t [[CC]YY]MMDDhhmm[.SS]] file ...\n       touch [-acfm] [MMDDhhmm[yy]] file ...\n"
$       Do not translate touch.
$       Do not translate the flags.
$       mm is represents a  2-digit number for month.
$       dd is represents a  2-digit number for day.
$       hh is represents a  2-digit number for hour.
$       mm is represents a  2-digit number for minute.
$       yy is represents a  2-digit number for year.
$       MM - The month of the year (01-12).
$       DD - The day of the month (01-31).
$       hh - The hour of the day (00-23).
$       mm - The minute of the hour (00-59)
$       CC - The first two digits of the year (the century).
$       YY - The second two digits of the year.
$       SS - the second of the minute (00-61).
TCONV                "touch: The access or modification time format is: [mmddhhmm[yy]].\n"
$       mm is represents a  2-digit number for month.
$       dd is represents a  2-digit number for day.
$       hh is represents a  2-digit number for hour.
$       mm is represents a  2-digit number for minute.
$       yy is represents a  2-digit number for year.
TCREATE              "touch: Cannot create %s.\n"
TSTAT                "touch: Cannot get status on %s.\n"
TMALLOC		     "touch: No memory.\n"
TCHTIME              "touch: Cannot change the modification time on %s.\n"
SPECTIME             "touch: The specified time format is: [[[CC]YY]MMDDhhmm[.SS]].\n"
$       MM - The month of the year (01-12).
$       DD - The day of the month (01-31).
$       hh - The hour of the day (00-23).
$       mm - The minute of the hour (00-59)
$       CC - The first two digits of the year (the century).
$       YY - The second two digits of the year.
$       SS - the second of the minute (00-61).
