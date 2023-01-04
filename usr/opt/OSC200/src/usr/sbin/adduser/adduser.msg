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
$ @(#)$RCSfile: adduser.msg,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/09/30 19:29:53 $
$ 
$ 
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$ 
$ 
$quote "
$set MS_ADDUSER
1 "Please su to root first.\n"
2 "\nNIS in use. Refer to the network documentation for adding users.\n"
3 "\nEnter a login name for the new user (for example, john): "
4 "Error, login name cannot be longer than 8 characters.\n"
5 "User (%1$s) is already in the /etc/passwd file.\n"
6 "\nThe /etc/passwd file may be corrupt.\n"
7 "Exiting %1$s.  New entry not created.\n"
8 "Enter a UID for (%1$s) [%2$s]: "
9 "Bad value for UID, must be an integer\n"
10 "UID must be less than %1$s\n"
11 "There is another account with UID %1$s. Use UID %1$s ([y]/n)? "
12 "Enter a full name for (%1$s): "
13 "Error, name can not contain colons.\n"
14 "Enter a login group for (%1$s) [%2$s]: "
15 "\nThe group %1$s was not found.  The existing groups are:\n\n"
16 "\nDo you want to add group %1$s to the /etc/group file ([y]/n)? "
17 "\nAdding group %1$s to the /etc/group file...\n"
18 "\nEnter another group that (%1$s) should be a member of.\n(<Return> only if none): "
19 "\nUser (%1$s) is already a member of group %2$s.\n"
20 "Enter a parent directory for (%1$s) [%2$s]: "
21 "%1$s was not found, do you want to create it ([y]/n)? "
22 "Unable to create %1$s.\n"
23 "Error, you must answer either y or n.\n"
24 "The shells are:\n\n"
25 "Enter a login shell for (%1$s) [%2$s]: "
26 "Rebuilding the password database...\n"
27 "Creating home directory...\n\n"
28 "Unable to cd to parent directory %1$s\n"
29 "%1$s/%2$s already exists.\n"
30 "You must enter a new password for (%1$s).\n"
31 "Warning, the password for (%1$s) was not set.\n"
32 "\nAdding new user...\n"
33 "\nA UID greater than the maximum allowed (%1$s) was found.\n"
$ The following 2 entries contain regular expressions for yes and no responses.
$ The regular expression format must be maintained during any translation.
34 "[yY]*"
35 "[nN]*"
36 "The /etc/passwd file is busy.  Try again later.\n"
37 "Unable to create temporary group file, user (%1$s) not added to group %2$s.\n"
38 "Finished adding user account for (%1$s).\n"
39 "The hashed password database does not exist.  Do you want to create it ([y]/n)? "
40 "Invoking ed editor ......\n\n"
$set 2
1 "An auth database entry already exists for (%1$s)\n"
2 "Enter maximum password lifetime in days [%1$s]: "
3 "Enter minimum password lifetime in days [%1$s]: "
4 "Will machine generated passwords be required (y/[n])? "
5 "Do you wish to edit the auth file entry for this user (y/[n])? "
