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
$ @(#)$RCSfile: split.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 16:55:40 $
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
$ split.msg	1.9  com/cmd/files,3.1,9021 3/20/90 17:50:55
$
$set MS_SPLIT
$quote "

USAGE "Usage: split [[-b n[k|m] ] | [-l lines]] [-a suffix_length] [file [name]]\n"
INPOPNER "split: Cannot open input file.\n"
OUTNMSIZ "split: Output file cannot exceed %d characters.\n"
$ %d is the maximum number of characters in the output file.
ABRTSPLT "split: Split is terminated. Exceeded the maximum number of output files.\n"
OUTCRTERR "split: unable to write to %s.\n"
BADBYTES  "split: Bad number of bytes.\n"
BADLENGTH "split: Bad suffix length.\n"
BADLINES  "split: Bad number of lines\n"
