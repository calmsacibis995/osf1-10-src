 
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

$ @(#)$RCSfile: rdist.msg,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/10/13 14:20:52 $

$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ rdist.msg	1.5  com/cmd/arch/rdist,3.1,9021 3/21/90 21:32:23
$
$quote "
$set MS_RDIST
$       GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate rdist -- this is the command name.
$ Do not translate %o %s %d %1$s %2$s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message.
$ The program replaces %o,%1$o,etc with an octal number.
$ The program replaces %d,%1$d,etc with a decimal number.
$ the program replaces %.2f with a fixed-point number.
$ the program replaces %u with an unsigned integer.
$ The program replaces %.1f%% with a percentage.
$ The program replaces %s,%1$s,etc with a string of
$ characters.

ARGTOO  "rdist: The parameter list is too long.\n"

CANTLN  "rdist: The host system %s cannot link %s to %s.\n"

CANTOPEN "rdist: Cannot open %s.\n"

CHOWNF  "rdist: %s: chown or chmod failed on file %s: %s"
$ Do not translate chown, chmod
$ %s's are the system where problem occurred,
$ the file or directory name, and an appropriate error message.

CLEANOP "rdist: Options are not delimited.\n"

COMPDATES  "rdist: Comparing dates: %s\n"
$ %s is a filename. This is information only.

CONNFAIL  "rdist: Connection failed; rdist version\n\
\tnumbers do not match. (local: %d, remote: %d)\n"
$ %d is the version numbers

CONNFAIL2  "rdist: Connection failed; rdist version numbers do not match.\n"

CONTRR  "rdist: The host program expected a control record.\n"

DATENOTE  "rdist: update: note: %s\n"
$ This is log information only.
$ %s is a file or directory name.

DATING  "rdist: updating: %s\n"
$ This is log information only.
$ %s is a file or directory name.

DESTTOO "rdist: Too many destination hosts were specified.\n"

DIRTOO  "rdist: %s:%s: has too many directory levels.\n"

EEEEE   "rdist: The host program has too many end of directory records.\n"

FILESIZE  "rdist: The size of the file %s has changed.\n"

HARDOLD "rdist: Hardlink oldname name is not delimited.\n"

HARDOPT "rdist: Hardlink options are not delimited.\n"

ILLCOM  "rdist: %d is not a valid command type.\n"
$ %d is the type of command used.

INPUTL  "rdist: The input line is too long.\n"

INST    "rdist: installing: %s\n"
$ This is log information only.

LINEER  "rdist: line %d: %s\n"
$ This prints out the line number (%d) with a message(%s).
$ Leave this as it is.

LINEW   "rdist: line %d: Warning: "
$ %d is the line number.
$ It appears as though this is never seen by any user.

LNLOST  "rdist: There is not enough memory available now.\n"

LOSTCON "rdist: The connection has been lost.\n"

MAILFAIL  "rdist: %s failed to open a pipe to the mailing program.\n"

MISSBRC "rdist: There is a missing } character in the distfile.\n"

MISSBRK "rdist: There is a missing ] character in the distfile.\n"

MISSLN  "rdist: %s is missing links.\n"
$ %s is the file with the missing links.

MISSQU  "rdist: There is a missing closing \" character in the distfile.\n"

MTDEL   "rdist: update: A file modification time was not delimited.\n"

NEEDIN  "rdist: Need to install: %s\n"
$ This is log information only.

NEEDUP  "rdist: Need to update: %s\n"
$ This is log information only.

NEWFILE "rdist: new: %s\n"

NODOLLAR  "rdist: No variable name was specified with the $ character.\n"

NOMEM   "rdist: There is not enough memory available now.\n"

NOGROUP_MSG "rdist: %s: No name for group %d.\n"

NOFILE1 "rdist: %s is not a file or directory.\n"
$ %s is the bad file or directory entry.

NOLOGIN "rdist: %s:%s: login name not known; clearing setuid.\n"
$ Do not translate setuid
$ %s's are the host system and the owner.

NOPAR   "rdist: %s:%s: %s (no parent)\n"

NOPASS  "rdist: %s: No password entry for uid %d.\n"
$ %s is the target file or directory.

NOTEONLY        "rdist: Note: %s\n"
$ This is information only. Do not translate.

NOTIFY1 "rdist: notify "
$ This is for user info only. Do not translate.

NOTPL2  "rdist: %s:%s is not a plain file.\n"

NOTPLAIN "rdist: %s is not a plain file.\n"

NOTREG  "rdist: %s:%s: is not a regular file.\n"

NOUPDATE "rdist: There are no files to be updated.\n"
$ This is information only.

ONENAME "rdist: Only one name is allowed.\n"

OPTDEL  "rdist: The host program options are not delimited.\n"

OVERR   "rdist: The -n option overrides the -v option.\n"

PATHTOO "rdist: The path name is too long.\n"

RECDEL  "rdist: recvf: Mode is not delimited.\n"

RECGROUP "rdist: recvf: Group name is not delimited.\n"

RECMT   "rdist: recvf: Modification time is not delimited.\n"

RECOWN  "rdist: recvf: Owner name is not delimited.\n"

RECSIZE "rdist: recvf: Size is not delimited.\n"

RECVF   "rdist: recvf: Options are not delimited.\n"

REDEFINE  "rdist: The variable, %s, has been redefined.\n"

REMED   "rdist: removed %s\n"
$ %s is the file or directory name.
$ This is an information message.

REMOTEN "rdist: Warning: %s: remote copy is newer than local.\n"
$ This is log information only.
$ %s is a file or directory name.

REMOVE1 "rdist: need to remove: %s\n"
$ %s is the file or directory name.
$ User get this message with the verify option.
$ It lets the user know the specified file is an extraneous one.

RENOTLO "rdist: %s: Remote mode %o does not match local mode %o.\n"

RMCHKR  "rdist: rmchk: Received an unexpected response: %s\n"
$ %s is the unexpected response to rmchk.

SHRET   "rdist: Shell has returned a bad status: %d.\n"

SIZEDEL "rdist: update: File size not delimited.\n"

SPECI   "rdist: special \"%s\"\n"
$ This is log information only.
$ %s is the name of a special file.

STRTOO  "rdist: The command string is too long.\n"

TOOLONG "rdist: The name is too long: %s/%s\n"

TOOLONG1  "rdist: The name is too long: %s:%s/%s\n"

UNDEFINE   "rdist: The variable %s is not defined.\n"

UNGRP   "rdist: The system %1$s does not recognize group %1$s.\n"
$ %s is the host system and the group name.

UNKCOM  "rdist: %s is not recognized by the host program.\n"
$ %s is the command.
$ $ The command %s is not recognized.

UNKNOWNS  "rdist: The requested service is not known.\n"

UNLNK   "rdist: %s:%s: %s (unlink)\n"
$ Do not translate unlink
$ %s's are the host system, the target file or directory,
$ and an appropriate error message.

UNMATCH "rdist: There is an unmatched { character in the distfile.\n"

UNRESPON  "rdist: update: Response is not expected: %s\n"

UPDATEH "rdist: Updating the host %s.\n"
$ %s is the hostname.

UPDTED  "rdist: updated %s\n"
$ This is an information message only.
$ %s is a file or directory name.

USAGE   "Usage: rdist [-bhinqRvwyD] [-f DistFile] [-d var=value] [-m Host] [File ...]\n"

USAGE2  "\tor: rdist [-bhinqRvwyD] -c Source [...] [Login@]Machine[:Dest]\n"

USERN   "rdist: %s is not a valid user name.\n"

USERUNK ": user name not known."

USERUNK1  "rdist: %s is not a known user name.\n"

UTIMEF  "rdist: %s:utimes failed %s: %s\n"
$ Do not translate utimes.
$ %s's are the host system, the file or directory name,
$ and an error message. Do not move them around.

WHOARE  "rdist: %s not found.\n"
$ %s is the user name.

HLNOMEM	"%s:can't link %s to %s (out of memory).\n"
