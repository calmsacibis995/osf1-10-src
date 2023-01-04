 
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

$
$ @(#)$RCSfile: bsd.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/08/03 21:19:04 $
$
$ IBM CONFIDENTIAL
$ Copyright International Business Machines Corp. 1989
$ Unpublished Work
$ All Rights Reserved
$ Licensed Material - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$    " bsd.msg 1.5 com/cmd/sccs/hdr,3.1,9021 3/24/90 16:01:27 "
$
$quote "         define quote character.

$ Do not translate %s, %d, %1$s, etc..
$ Do not translate sccs:
$len 
$set 14        Define initial set#

$ This is the message catalog for the sccs command. 
NOUSER     "sccs: User %s does not exist.\n"
$ %s is a user name. 
$ Do not translate PROJECTDIR. 
NOSOURCE   "sccs: Project %s has no /source or /src subdirectory.\n"
$ Project %s is the user named with the PROJECTDIR variable. 
SCCSUSAGE  "usage: sccs [-dPath] [-pPath] [-r] {SCCSCommand|SCCSPseudoCommand} [SCCSFlag|SCCSFlags] [File]\n"
$ %s is the sccs command 
$ Do not translate sccs. 
UNKNOWNOPT "usage: sccs [-dPath] [-pPath] [-r] {SCCSCommand|SCCSPseudoCommand} [SCCSFlag|SCCSFlags] [File]\n"
$ %s is a flag. 
$ Do not translate sccs. 
UNKNOWNCMD  "sccs: %s is not an SCCS command.\n"
$ %s is a command that is not recognized. 
NEEDR4FIX   "sccs: Specify the -r flag with the fix command.\n"
$ Do not translate -r. 
$ Do not translate fix. 
CANNOTEXEC  "sccs: Cannot run %s.\n"
$ %s is a program. 
CANNOTFORK  "sccs: Cannot create another process.\n"
NOMEM       "sccs: There is not enough memory available.\n"
$ Do not translate sccs. 
PATHERR     "sccs: Cannot use .. or full path names.\n"
$ Do not translate .. 
TOOMANYARGS "sccs: Specify only one directory.\n"
CANTOPEN    "sccs: Cannot open directory %s.\n"
$ %s is a directory. 
BEDIT       "sccs: %s is being edited: "
$ %s is a file. 
$ Because this can be an informational message or an error message we cannot 
$ show recovery information. 
$ Following the colon (:) is the version number and date. 
NOEDIT     "sccs: Nothing is being edited.\n"
$ This message shows that no trunks or branches of the SCCS file tree are being 
$ edited. 
NOEDITTRNK "sccs: No trunk of the SCCS file tree is being edited.\n"
NOEDITBY   "sccs: Nothing is being edited by %s.\n"
$ %s is a user name. 
NOEDITTRNKBY  "sccs: No trunk of the SCCS file tree is being edited by %s.\n"
$ %s is a user name. 
BADFNAME      "sccs: File name %s is not in correct SCCS format.\n"
$ %s is a file name. 
NOBEDIT       "sccs: File %s is not being edited.\n"
$ %s is a file name. 
$ Do not translate unedit. 
CANTCREATE    "sccs: Cannot create %s.\n"
$ %s is a file name. 
CANTREMOVE    "sccs: Cannot remove %s.\n"
$ %s is a file. 
CANTREOPEN    "sccs: Cannot reopen %s.\n"
$ to reopen is to open again
REMOVED       "sccs: Temporary file %s is removed.\n"
$ %s is a temporary file. 
$ This is an informational message used by the unedit pseudocommand only. It 
$ removes the temporary file. 
NOEDITBYYOU "sccs: Only the user who is editing the file can unedit the file.\n"
$ Do not translate unedit
DODIFFPIPE  "sccs: The pipe system call failed.\n"
$ Do not translate pipe, dodiff. 
DODIFFFORK  "sccs: Cannot create another process.\n"
$ Do not translate dodiff
DODIFFMAGIC "sccs: The close system call or the dup system call failed.\n"
$ Do not translate close, dup, dodiff. 
UNKNOWNUID  "sccs: %d is not an existing user number.\n"
FNAMSTOOBIG "sccs: The file name are too large: %1$s %2$s.\n"
$ %1$s is a file. %2$s is a file. 
SCCSUSAGE2	"Usage: sccs [-r] [-dpathname] [-ppathname] command [flags] [file ...]\n"
$ do not translate sccs
ARGRQD		"ERROR:  Flag -%c requires an argumnet\n\
\tUsage: sccs [-r] [-d pathname] [-p pathname]\
 command [flags] [file ...]\n"
$ do not translate sccs

