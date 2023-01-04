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
$ @(#)$RCSfile: sh.msg,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/17 18:17:53 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2
$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for the shell.
$
$quote  "

$set MS_SH

$	informational messages

M_ATLINE "at line "
M_BUILT "%s is a shell builtin.\n"
$ %s is substituted with the name of a command.
M_BADSH "Bad BIN_SH value %s.\n"
$ %s is substituted with the value of BAD_SH which is not understood.
M_SHMISS "%s specified but %s is missing.\n"
$ %s is substituted with the value of an environment variable.
$ %s is substituted with the pathname of an alternate shell .
M_COREDUMP ": A memory image file is created as \"core\".\n"
M_ENDOFFILE "end of file"
M_FUNC "%s is a function.\n"
$ %s is substituted with the name of a function.
M_HPR "Hits\tCost\tCommand\n"
M_IS "%s is %s.\n"
$ The first %s is substituted with the name of a command that is hashed to the
$ second %s which is substituted with the name of command
M_ISH "%s is the hashed version of %s.\n"
$ The first %s is substituted with the name of a command that is hashed to the
$ second %s which is substituted with the name of command
M_MAILMSG "You have mail.\n"
M_NL "Specify either a newline or a ;"
M_NOTFOUND "not found"
M_SYNMSG "Command usage error "
M_UNEXPECTED " unexpected.\n"
M_WTFAILED "is read only."
$ There is no ulimit for this resource.
M_UNLIMITED "unlimited"
$ this message is part of the messagess that the shell displays when it finds
$ a syntax error.


$	error messages

M_ARGEXP "Specify a command parameter.\n"
M_ARGLIST "The argument list is greater than the system imposed limit.\n"
M_BADCREATE "Cannot create the specified file.\n"
M_BADEXEC "Cannot execute the command as specified.\n"
M_BADEXPORT "Cannot export functions.\n"
M_BADHASH "The specified hash options are not correct.\n\
\tUsage: hash [-r] [name . . .]\n"
M_BADFILE "Specify a known file number.\n"
M_BADNUM "Specify a correct number.\n"
M_BADOP "The specified operator is not known.\n"
M_BADOPEN "Cannot open the file.\n\
\tCheck filename or permissions.\n"
M_BADOPT "The specified options are not correct.\n\
\tUsage: sh|rsh [-i|-r][+|-][-a|-e|-f|-h|-k|-n|-t|-u|-v|-x][File]\n\
\t[Parameters][-c|-cmdstrCommandString|-s]\n"
M_BADPARAM "Cannot specify a null or unset parameter.\n"
M_BADPERM "Execute permission denied.\n\
\tCheck permissions.\n"
M_BADRETURN "Cannot perform a return when not in function mode.\n"
M_BADSHIFT "Cannot make the specified change.\n"
M_BADSUB "Specify a substitution correctly.\n"
M_BADTRAP "Cannot perform a trap as specified.\n"
M_ULIMITEXCEED  "Requested ulimit exceeds current hard limit."
M_ULIMITUSAGE "The specified ulimit is not correct.\n\
Usage: ulimit [-H | -S] [-a | -c | -d | -f | -h | -m | -n | -s | -t] [limit]\n"
M_ULIMITBAD "Bad ulimit value."
M_ULIMITNOTSU "Only superuser can set hard limit."
M_ULIMITRESOURCE "Failure getting or setting a resource value."
M_ULIMITHARD "Current soft limit exceeds requested hard limit."
M_BADUNSET "Specify a correct unset.\n"
M_BADINLIB "loader install procedure failed\n"
M_BADRMLIB "loader removal procedure failed\n"
M_NO_ARGS "requires argument.\n"
M_CD_ARGS "Too many parameters.\n\
\tUsage: cd [directory]"
M_LONGPWD "sh error: The maximum length for a full path name is\n\
\t512 characters.\n"
M_NOFORK "No more processes available.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
M_NOHOME "Cannot locate the home directory.\n\
\tCheck path name and permissions.\n"
M_NOSPACE "There is not enough memory available now.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
M_NOSWAP "No more swap space available.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
M_NOTID "The specified data is not an identifier.\n"
M_NOTF "Cannot find %s.\n"
M_PERM "Permission denied.\n\
\tCheck permissions.\n"
M_PIPERR "The pipe system call failed.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
M_PWDOPEN "pwd: Cannot open .. \n\
\tCheck path name and permissions.\n"
M_PWDREAD "pwd: The read system call failed in  ..\n\
\tTry again later or\n\
\tuse local problem reporting procedures."
M_RBRACKET "There is a missing ] .\n"
M_RESTRICTED "Operation is not allowed in Restricted Shell.\n\
\tCheck permissions.\n"
M_RPAREN "There is a missing ) .\n"
M_STATDOT "pwd: The stat system call failed at . \n\
\tCheck path name and permissions.\n"
M_STATDOTDOT "pwd: The stat system call failed at .. \n\
\tCheck path name and permissions.\n"
M_TRAP "Cannot trap 11.\n"
M_TXTBSY "The text is in use.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
M_UNKNOWN "Specify a builtin that is known.\n"
M_UNSET "The parameter is not set.\n"
M_SYSCONF "sh: sysconf error"
M_NOMEM "sh: no memory"
M_BADUMASK "umask: Improper mask"
