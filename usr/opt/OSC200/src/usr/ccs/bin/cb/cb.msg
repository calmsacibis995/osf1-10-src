$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27; 10
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set MS_CB
$quote "
BADOPEN "cb: Cannot open %s.\n\
\tCheck path name and permissions.\n"
BADOPTION "cb: %c is not a recognized flag.\n\
\tUsage: cb [-s] [-j|-l length] [File] ...\n"
BIGCOMMENT "Cannot read comment lines greater than 1024 characters.\n\
\tReduce the length of the comment.\n"
CURLY "There are more than 20 levels of braces ({}).\n\
\tThese will not be formatted correctly.\n"
DODEEP "There are more than 10 levels of do statements.\n\
\tThese will not be formatted correctly.\n"
$ NOTE:
$ NOTE: The "do" part of the above message refers to C language statements
$ NOTE:
IFDEEP "There are more than 10 levels of if statements.\n\
\tThese will not be formatted correctly.\n"
$ NOTE:
$ NOTE: The "if" part of the above message refers to C language statements
$ NOTE:
TRUNCATING "Cutting off comment line longer than 900 characters.\n"
NONUM	"The -l flag must be followed by a number.\n"
FOLDED "/* The indentation level exceeds the page width. */\n\
/* Resetting to the left page margin. */\n"
$ NOTE:
$ NOTE: Do NOT remove the slashes and asterisks from the "FOLDED" message.
$ NOTE: These messages get ADDED to the output of cb and MUST follow
$ NOTE: C language syntax. (Note: no tab at beginning)
$ NOTE: (Same for message "UNFOLDING")
$ NOTE:
UNFOLDING "/* Restoring indentation level to original setting. */\n"
