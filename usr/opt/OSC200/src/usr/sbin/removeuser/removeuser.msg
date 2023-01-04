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
$ HISTORY
$ 
$ @(#)$RCSfile: removeuser.msg,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/04 09:32:00 $
$
$quote "
$set MS_REMOVEUSER
1	"\nNIS in use. Refer to the network documentation for adding users.\n"
2	"\nEnter a login name to be removed or <Return> to exit: "
3	"User (%1$s) was not found in the /etc/passwd file.\n"
4	"This is the entry for (%1$s) in the /etc/passwd file:\n\n %2$s\n"
5	"\nIs this the entry you want to delete (y/n)? "
6	"\nUser (%1$s) was not removed.\n"
7	"Working ..."
8	"Rebuilding the password database..."
9	"\nEntry for (%1$s) removed."
10	"\nDo you want to remove the home directory, all subdirectories, \nfiles and mail for (%1$s) (y/n)? "
11	"The files for (%1$s) will be lost if not backed up.\n"
12	"Are you sure you want to remove these files (y/n)? "
13	"\nRemoving %1$s\n"
14	"\n%1$s not removed.\n"
15	"\nError, the /etc/group or /etc/passwd file is busy.\n"
16	"\nCould not copy the /etc/passwd file to /etc/ptmp/passwd."
17	"\nCould not copy the /etc/group file to /etc/ptmp/group."
18	"\nNo user removed.\n"
19	"\nSearching relevant directories and files for user (%1$s) ...\n"
20	"- Entries containing the home directory %1$s\n  for this user were found in the /etc/exports file.\n"
21	"- Entries for (%1$s) were found in the /var/adm/sendmail/aliases file.\n"
22	"- Entries for (%1$s) were found in the /usr/lib/mh/MailAliases file.\n"
23	"- Entries for (%1$s) were found in the /var/spool/cron/crontabs directory.\n"
24	"- Entries for (%1$s) were found in the /var/spool/cron/atjobs directory.\n"
25	"You may want to remove the entries for (%1$s) from these directories/files.\n"
26	"None found.\n"
$ The following 2 entries are regular expressions for 1 letter yes/no responses.
$ The regular expression format must be maintained during any translation.
27	"[yY]*"
28	"[nN]*"
29	"\nFinished removing user account for (%1$s).\n"
