$ 
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$ 
$ 
$ OSF/1 Release 1.0

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
$quote "

$ messages for the diff3.sh shell script
$set 1

1 "diff3: File not found.\n\
\tSpecify three existing files.\n"
2 "Usage: diff3 File1 File2 File3\n\
\tCompares three files.\n"
$ Do not translate diff3.
$ messages for the diff3prog.c source file
$set MS_DIFF3

ARGCNT "diff3: Specify three existing files on the command line.\n"
$ Do not translate diff3.
CANTOPEN "diff3: Cannot open %s.\n\
\tCheck path name and permissions.\n"
$ %s is a file.
$ Do not translate diff3.
CHNGCNT "diff3: There are too many differences\n\
\tbetween the three files to use the diff3 command.\n\
\tThere must be less than 200 lines that are different\n\
\tbetween the three specified files.\n"
$ Do not translate diff3.
LOGERR "diff3: There is a logic error in the diff3 program.\n\
\tTurn on the debugging mode to use the diff3 command\n\
\twith the specified files.\n"
$ Do not translate diff3.
UNKFLG "diff3: Flag %c is not a recognized flag.\n\
\tUsage: diff3 [-exEX3] file1 file2 file3\n"
