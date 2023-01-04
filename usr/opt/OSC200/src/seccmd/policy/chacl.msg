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
$
$ Copyright (c) 1988-91 SecureWare, Inc.
$   All rights reserved.
$
$ @(#)chacl.msg	1.3 17:01:30 9/3/91 SecureWare
$
$quote "
$set MS_CHACL

CHUSAGE		"Usage:  chacl [-a] directoryname(s)\n\tchacl [ -x | -c ] [-p] [-b] [-d] [-f file1] [-r entries]\n\t\t[-R file2] [-u entries] [-U file3] filename(s).\n"
MALLOCERR	"%s: Memory allocation error.\n"
ACLREAD		"%s: Cannot read ACL of file \"%s\".\n"
STATERR		"%s: Failed stat of file \"%s\".\n"

DELDEFERR	"%s: Error deleting default acl for \"%s\".\n"
DELEXTERR	"%s: Error deleting extended entries for \"%s\".\n"
UPDTMASKERR	"%s: Error updating mask for \"%s\".\n"
UNUSERR		"%s: Contents of file \"%s\" unusable.\n"
UPDTACLERR	"%s: Error updating ACL for \"%s\""
UNUSENTRY	"%s: Entries are unusable.\n"
DELBASE		"%s: Cannot remove base entry for \"%s\".\n"
DELENTERR	"%s: Error deleting entry for \"%s\""
DELMASKERR	"%s: Cannot delete mask entry for \"%s\".\n"
RELATIVEERR	"%s: Cannot accept relative permissions.\n"
MASKCALCERR	"%s: Invalid mask calculated.\n"
ADDWARN		"%s: Warning: changing \"%s\" to be more permissive.\n"
ADDPERMERR	"%s: Cannot change \"%s\" to be more permissive.\n"
READERR		"%s: Cannot read \"%s\""
ADDMASKERR	"%s: Cannot specify mask and -c or -x flags.\n"


