 
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

$ @(#)$RCSfile: install.msg,v $ $Revision: 4.3.3.2 $ (OSF) $Date: 1992/01/29 17:46:18 $

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
$ install.msg	1.4  com/cmd/files,3.1,9021 3/19/90 16:22:45
$
$ 09/13/91 - Tom Peterson
$	- changed symbolic names to numbers according to OSF/1 1.0.2
$	- left symbolic names and corresponding numbers as comments
$	for reference
$
$quote "
$
$set MS_INSTALL

$ USAGE = 1	OPTIONS = 2	INVAL_C = 3	FLAG = 4	INVAL_3C = 5
$ INVAL_F = 6	INVAL_3F = 7	INVAL_I = 8	INVAL_3I = 9	INVAL_N = 11
$ INVAL_3N = 12	INVAL_O = 13	INSTALLIT = 14	EXISTS = 15	MOVED = 16
$ OLDSTUFF = 17	INSTALL_CP = 18	SLASHED = 19	FAILURE = 20	INSTALL_AS = 21
$ MODE = 22	GROUP = 23	OWNER = 24	STRIPPED = 25	OLDER = 26
$ CANTFIND = 27	DEFAULTED = 28	PERMS = 29
$
$
1	"Usage: install [-c DirectoryA] [-f DirectoryB] [-i] [-M] [-m Mode] [-u Owner]\n\
\t       [-g Group] [-S] [-n DirectoryC] [-o] [-s] File [DirectoryX ...]"

2	"install: The -c, -f, and -n flags must be followed by a directory.\n\
\t The -m, -u, -g flags must be followed by a mode, owner, or group."

3	"install: The -c flag is not valid with the"
$ This message is followed by a flag, and then by FLAG.

4	"flag."

5	"install: The -c flag requires at least 3 parameters."

6	"install: The -f flag is not valid with the"
$ This message is followed by a flag, and then by FLAG.

7	"install: The -f flag requires at least 3 parameters."

8	"install: The -i flag is not valid with the"
$ This message is followed by a flag, and then by FLAG.

9	"install: The -i flag requires at least 3 parameters."

10	"install: The -m flag is not valid with the"
$ This message is followed by a flag, and then by FLAG..

11	"install: The -n flag is not valid with the"
$ This message is followed by a flag, and then by FLAG.

12	"install: The -n option requires at least 3 parameters."

13	"install: The -o flag is not valid with the"
$ This message is followed by a flag, and then by FLAG.

14	"install: "
$ Do not translate

15	"already exists in"
$ This message prints out as follows:
$ install: filename already exists in directoryname
$ The program is placing the file and directory names in the line.

16	"moved to"
$ This message prints out as follows:
$ install: filename moved to directoryname/OLDfilename
$ The program is placing the file and directory names in the line.

17	"/OLD"
$ Do not translate

18	"install: cp"
$ Do not translate

19	"/"
$ Do not translate

20	": The command failed."
$ This message is printed out with INSTALL_CP.
$ install: cp dirname/OLDfilename dirname/filename: The command failed.
$ The program is placing the file and directory names in the line.

21	"is installed as"
$ This message prints out as follows:
$ filename is installed as dirname/filename
$ The program is placing the file and directory names in the line.

22	"chmod"
$ Do not translate

23	"chgrp"
$ Do not translate

24	"chown"
$ Do not translate

25	"strip"
$ Do not translate

26	"The old"
$ This is followed by filename, then MOVED, then another filename

27	"was not found."
$ This message is formed as follows:
$ install: filename was not found.

28	"by default."
$ This message is used with INSTALL_AS as follows:
$ filename is installed as dirname/filename by default.

29	"chmod: Use absolute mode to specify the desired permission settings."
