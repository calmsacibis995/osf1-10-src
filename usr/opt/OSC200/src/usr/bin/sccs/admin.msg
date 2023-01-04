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
$ HISTORY

$
$ @(#)$RCSfile: admin.msg,v $ $Revision: 4.2.3.2 $ (OSF) $Date: 1993/08/25 22:13:33 $
$
$ Copyright International Business Machines Corp. 1989
$ Unpublished Work
$ All Rights Reserved
$ Licensed Material - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$       admin.msg 1.8 com/cmd/sccs/hdr,3.1,9021 4/25/91 19:26:34
$
$quote "         define quote character.

$len 
$ DO NOT TRANSLATE the (ad#) at the end of each of these lines.
$ DO NOT TRANSLATE SCCS, SID, admin:, 
$ DO NOT TRANSLATE %c, %s, %1$s, etc...
$ DO NOT TRANSLATE -i, -y, -t, -m, -n, -d, etc..
$set 4        Define initial set#

DNOARG               "\n\
admin: Specify a header flag with the -d flag. (ad1)\n"
FLGNOVAL2            "admin: Specify a value for the %c header flag. (ad2)\n"
UNKNWNFLG            "admin: %c is not a recognized header flag. (ad3)\n\
Usage: admin {-n|-i[Name]} [-f{l{a|Number ...}|\n\
\t{b|cNumber|dSID|fNumber|i|j|mModule|n|qText|tType}} ...]\n\
\t[-fv[Program] -m[MRList]] [-aUser ...] [-rNumber.Number]\n\
\t[-t File] [-y[Comment]] NewFile ...\n\
\tCreates and initializes SCCS files.\n\
\n\
\tUsage: admin [-aUser ...] [-eUser ...] [-f{l{a|Number ...}|\n\
\tb|cNumber|dSID|fNumber|i|j|mModule|n|qText|tType} ...]\n\
\t[-d{l{a|Number ...}|b|c|d|f|i|j|m|n|q|t ...] [-dv|-fv[Program]]\n\
\t[-t[File]] [-h] [-z] File ...\n\
\tReferences existing SCCS files.\n"
FLGTWICE             "admin: Cannot use the %c header flag more than once. (ad4)\n"
FFLGNOARG            "\n\
admin: Specify a header flag with the -f flag. (ad5)\n"
AFLGARG              "\n\
admin: Specify a user name, group name or group number with\n\
\tthe -a flag. (ad8)\n"
AFLGKEYLTRS          "\n\
admin: Cannot use the -a flag more than 10 times\n\
\ton each command line. (ad9)\n"
BADEARG              "\n\
admin: Specify user name, group name or group number to\n\
\tremove. (ad10)\n"
EFLGKEYLTRS          "\n\
admin: Cannot use the -e flag more than 10 times\n\
\ton each command line. (ad11)\n"
CFLGVAL              "admin: The %c header flag cannot have a value\n\
\twhen used with the -d flag. (ad12)\n"
FLGVAL13             "admin: The %c header flag cannot have a value\n\
\twhen used with the -f flag. (ad13)\n"
NODFLTSID            "\n\
admin: Specify an SID number with the d header flag. (ad14)\n"
$ Do not translate SID or d
MTONEFIL             "\n\
admin: Create only one SCCS file at a time with the -i flag. (ad15)\n"
$ Do not translate SCCS or -i.
RONLYIORN            "\n\
admin: Use the -r flag only with the -i flag or the -n flag. (ad16)\n"
TFLGNOARG            "\n\
admin: Specify a file name for the -t flag when creating\n\
\ta new SCCS file. (ad17)\n\
."
NODWITHN             "\n\
admin: Cannot use the -d flag with the -n  or -i flags. (ad18)\n"
FILEEXISTS           "admin: File %s exists. Cannot use the -n flag or\n\
\tthe -i flag with an existing file. (ad19)\n"
FLEXST20             "admin: File %s does not exist. (ad20)\n"
ILLGLDATA            "admin: %1$s contains illegal data on line %2$d.\n\
\t (ad21)\n"
$ %1$s is the file name.  %2$s is the line number. 
FLOORNOTNUM          "admin: Specify a value between 0 and 10000\n\
\twith the f header flag. (ad22)\n"
FLOORRNG             "admin: Specify a value between 0 and 10000\n\
\twith the f header flag. (ad23)\n"
CEILNONNUM           "admin: Specify a value between 0 and 10000\n\
\twith the c header flag. (ad24)\n"
CEILRNG              "admin: Specify a value between 0 and 10000\n\
\twith the c header flag. (ad25)\n"
DIRIKYLTR            "\n\
admin: Specify a file, not a directory, with the -i flag. (ad26)\n"
BADLISTFMT           "\n\
admin: The version and release must be numbers\n\
\tseparated by a comma. (ad27)\n"
ELMNTRNG             "\n\
admin: Cannot lock a version number or a release number\n\
\tgreater than 9999. (ad28)\n"
BDIR                 "admin: Cannot use the -%2$c flag with directory %1$s.\n\
\tSpecify a file, not a directory, with the -i flag or the -t flag. (ad29)\n"
$ %2$c is a flag. %1$s is a directory. 
ILLYORM              "\n\
admin: Use the -y flag or the -m flag only when creating\n\
\tan SCCS file. (ad30)"
BYMSG                "by"
DATETIME             "date and time created"
LINECNTWARN          "\n\
WARNING - The number of lines in this file exceeds\n\
\t99999.  The header section of the s. file will record 99999 in the\n\
\tlines inserted field.\n"
MRLISTSTARTFRDASH	"\n\
admin:  The MRlist   %s   starts from \"-\". This is a WARNING."
$ The source file line count exceeds 5 digits. 
$ The s. file has a 5 digit limit so it will record 99999. 
CEILCONS             "\n\
admin: The ceiling value %1$u is less than the lowest\n\
\trelease(the floor). (ad31)\n"
FLORCONS             "\n\
admin: The floor value %1$u is greater than the highest\n\
\trelease(the ceiling). (ad32)\n"
DEFBADOPTION	"\nFlag -%c is not valid. (cm1)\n\
\tadmin {-n|-i[Name]} [-f{1{a|Number ...}|\n\
\t{b|cNumber|dSID|fNumber|j|mModule|n|qText|tType}} ...]\n\
\t[-fv[Program]] [-m[MRList]] [-aUser ...] [-rNumber.Number]\n\
\t[-t File] [-y[Comment]] File ...\n\
\tCreates and initializes SCCS files.\n\
\n\
\tUsage: admin [-aUser ...] [-eUser ...] [-f{1{a|Number ...}|\n\
\tb|cNumber|dSID|fNumber|i|j|mModule|n|qText|tType} ...]\n\
\t[-d{1{a|Number ...}|b|c|d|f|i|j|m|n|q|t] ...] [-dv|-fv[Program]]\n\
\t[-t[File]] [-h] [-z] File ...\n\
\tReferences existing SCCS files.\n"
MNOARG               "\n\
admin: Specify an MR list with the -m flag.\n"
