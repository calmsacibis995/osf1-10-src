 
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

$ @(#)$RCSfile: installbsd.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:36:45 $

$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ instbsd.msg	1.4  com/cmd/files/installbsd,3.1,9021 3/19/90 16:31:18
$
$ message for the install.sh shell script
$quote "
$
$
$set MS_INSTBSD
$ Do not translate install

1 "install: Cannot specify the -c flag more than once."
2 "install: Specify a destination."
3 "install: There are too many files specified: $*"
$ The files specified follow after the colon.
4 "install: Cannot move the file"
$ It appears that at times this message will be followed by message 5

5 "onto itself."
$ It appears that at times this message will follow message 4.
6 "install: Cannot open the file"
$ This is followed by the file name.
