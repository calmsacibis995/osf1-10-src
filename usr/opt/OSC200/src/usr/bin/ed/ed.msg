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
$ @(#)$RCSfile: ed.msg,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/09/03 03:29:38 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ @(#)ed.msg	1.5  com/cmd/edit,3.1,9013 3/2/90 11:52:21
$ /*
$  * COMPONENT_NAME: (CMDEDIT) ed.msg
$  *
$  * FUNCTIONS: none
$  *
$  *
$  * (C) COPYRIGHT International Business Machines Corp. 1984, 1989
$  * All Rights Reserved
$  *
$  * US Government Users Restricted Rights - Use, duplication or
$  * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$  *
$  */
$quote "
$set MS_ED
$len 

$     Do not translate %s, %d, etc. These will be replaced by file, numbers, etc   
$     The term "regular expression" is a Unix technical term.   
1 "ed: Cannot create a pipe."
2 "ed: Use the w subcommand to write and save the file."
3 "ed: The mark name must be a lower case alphabetic character."
4 "ed: Cannot find or open the input file."
5 "ed: A tab specification is not formatted correctly."
6 "ed: There is no subcommand to undo."
$      "undo" is a specific command within the ed editor   
7 "red: Cannot run the specified subcommand with a restricted shell."
8 "ed: Cannot create the output file."
9 "ed: Cannot write to the output file due to lack of filesystem space."
10 "ed: Cannot find or open the output file."
11 "ed: Cannot link to the output file."
12 "ed: Specify a range end point that is less than 256."
13 "ed: The specified subcommand does not exist."
14 "ed: The search string is not found."
15 "-"
16 "ed: The specified line does not exist."
17 "ed: The character or characters between \\{ and \\} must be numeric."
18 "ed: The first address cannot exceed the second in an address pair."
19 "ed: The specified subcommand does not require an address."
20 "ed: The global subcommand requires a pattern."
$ There is a "global" subcommand in the ed editor (it is not an adjective here) 
21 "ed: The subcommand contains added characters that are not valid."
22 "ed: The specified subcommand requires a file name."
23 "ed: Use a space between the subcommand and the file name."
24 "ed: Cannot create a process at this time."
25 "ed: File names cannot be longer than %i characters."
26 "ed: Specify a \\digit between 1 and 9 that is\n\
\tnot greater than the number of subpatterns."
27 "ed: An interrupt signal was received. Returning to command mode."
28 "ed: The editor ignores lines longer than 512 characters."
29 "ed: The input file contains characters which are not valid."
30 "ed: Cannot write to the specified file."
31 "ed: There is not enough memory available for the append."
32 "ed: The temporary file buffer cannot contain more than 2167 blocks.\n\
\tCannot edit the entire file."
33 "ed: Cannot read to or write from the temporary file buffer."
34 "ed: Cannot use multiple global subcommands."
$      There is a "global" subcommand in the ed editor.   
35 "ed: The global subcommand list cannot exceed 256 characters."
36 "ed: There is no matching pattern."
37 "ed: A delimiter is not correct or is missing."
38 "-"
39 "ed: The replacement string cannot be longer than 512 characters."
40 "ed: The move destination must be numeric and cannot\n\
\tbe one of the lines that is moved."
41 "-"
42 "ed: There is no remembered search string."
43 "ed: There is a missing \\( or \\)."
44 "ed: Specify \\( no more than 9 times."
45 "ed: Specify no more than 2 numbers between \\{ and \\}."
46 "ed: An opening \\{ must have a closing \\}."
47 "ed: The first number cannot exceed the second number\n\
\tbetween \\{ and \\}."
48 "ed: The substitute is not complete."
$      Message 48 does not exist so do not translate.   
49 "ed: Newline was not expected."
$      Message 49 does not exist so do not translate.   
50 "ed: There is a missing [ or ]."
51 "ed: The regular expression is too large."
52 "ed: There is a regular expression error.\n"
53 "ed: The G and V subcommands require a subcommand."
54 "ed: The a, i, or c subcommands cannot be used\n\
\twith the G and V subcommands."
55 "ed: An end-of-line character was expected."
56 "ed: There is no remembered replacement string."
57 "ed: There is no remembered subcommand."
58 "ed: File names cannot begin with a ! character."
59 "ed: A simultaneous edit is being performed on the file."
60 "ed: The specified command cannot be used when the -y flag is set."
61 "ed: The X subcommand replaced the x subcommand."
62 "ed: Warning: An attempt to write the file may destroy it.\n\
\tThe file contains characters which are not valid."
63 "ed: Warning: Quitting may cause the loss of data\n\
\tin the temporary buffer."
64 "ed: File not written. It was truncated. Try write to another file."
M_USAGE  "USAGE: ed [-p string] [-s] [file]\n"
M_EDPARG "ed: The -p flag requires a parameter.\n"
M_EDFILE "ed: A file name cannot exceed 64 characters in length."
M_EDNULLS " null characters were replaced by the \\0 character."
$      Please leave the space at the beginning -- this follows a number.   
M_EDINCREAD "ed: \007The attempted read was interrupted\n\
\tand may not be complete.\007"
$ The \007 needs to be left literal.  (It "beeps" the terminal)   
M_EDLINE "ed: Input file lines cannot exceed 512 characters.\n\
\tLine number "
$      Leave the space at the end.   
M_EDTOOLONG "ed: Cannot write a line greater than 512 characters."
M_EDTAB "ed: Cannot write a line greater than 512 characters.\n"
M_EDKEY "Enter the file encryption key: "
M_EDGENKEY "ed: crypt: Cannot generate the encryption key."
$ NOTE:   
$ NOTE: These last two are NOT part of the present "ed" command   
$ NOTE: but may come back if the programmers can figure out a way   
$ NOTE: to do encryption (Do not translate "crypt:")   
$ NOTE:   
