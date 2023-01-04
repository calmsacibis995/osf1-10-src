 
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
$ @(#)$RCSfile: co.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:50:21 $
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
$       co.msg 1.5 com/cmd/sccs/hdr,3.1,9021 3/24/90 16:02:06
$
$quote "         define quote character.

$ Do not translate , SCCS, SID, MR
$ Do not translate %s, %c, %1$s, %2$d, etc...
$ Do not translate the parenthesis (co#), or (ut#) at the end of each line.
$len 
$set 3        Define initial set#

NOTSCCS              "\n\
Specify an SCCS file with this command. (co1)\n"
MRTHNONELNK          "\n\
SCCS files can have only one link or name. (co3)\n"
FRMTERR              "There is a format error at line %u. (co4)\n"
PRMTREOF             "\n\
Missing newline at the end of the file. (co5)\n"
CORUPTFILE           "\n\
The file is damaged. (co6)\n"
PATHTOOLNG           "\n\
The maximum path name size is 510 characters. (co7)\n"
INVSID               "\n\
Specify a valid SID number. (co8)\n"
$ Do not translate sact, SID, p-file
CLKWRNG              "\n\
The creation date of the newest delta in the SCCS file\n\
\tis later than the current time. (co10)\n"
CLKMAYWRNG           "WARNING: The difference between the current date and the creation date of\n\
\tthe newest delta in the SCCS file is greater than 1 year. (co11)\n"
BR                   "The 2 limits of a delta range must be in ascending order. (co12)\n"
DLS                  "The delta list syntax is not correct. (co13)\n"
NOTAUTH              "\n\
You are not authorized to make deltas. (co14)\n"
RLSLTFLR             "Release %1$u is less than lowest allowed release %2$u\n\
\t(the floor). (co15)\n"
$ Release %1$u is the specified release. 
$ Release %2$u is the lowest allowed release. 
RLSGTCLNG            "Release %1$u is greater than highest allowed release %2$u\n\
\t(the ceiling). (co16)\n"
$ Release %1$u is the specified release. 
$ Release %2$u is the highest allowed release. 
BADPFILE             "\n\
The p-file is damaged. (co17)\n"
$ Do not translate p-file.
CANTFORK             "\n\
Cannot create another process. (co20)\n"
TOOMANY              "\n\
The maximum number of MRs allowed is 64. (co21)\n"
$ Do not translate MRs
FILLCKD              "The SCCS file is locked against editing. (co23)\n"
RLSLCKD              "Release %d is locked against editing. (co23)\n"
CANTEXEC             "Cannot execute %s. (co50)\n"
OTOFSPC              "\n\
There is not enough memory available. (ut9)\n"
FFREE                "\n\
An attempt to free memory failed.\n"
INEXCNFLT            "An include or exclude conflict %1$s at line %u. (%2$s)\n"
$ %1$s will be replaced by the word "begins" or by the word "ends" from the 
$  28 or the BEGINS message. 
$ %u is a line number. 
$ %2$s will be replaced by "(co25)" or "(co26)". 
TYPTERM              "Type %s, terminated with an End of File character\n\
\tor a blank line.\n"
$ %s is replaced by the "comments" message from the delta.msg message catalog. 
$ (MCOMMENTS). 
INCLUDED             "Included:\n"
EXCLUDED             "Excluded:\n"
CORRUPT              "\n\
The file is damaged. (co6)\n"
ENDS                 "ends"
$ Note to translator: Please translate this message literally. 
$ This one-word message will replace %1$s in the INEXCNFLT message. 
BEGINS               "begins"
$ Note to translator: Please translate this message literally. 
$ This one-word message will replace %1$s in the INEXCNFLT message. 
LCNTWARN             "WARNING: The number of lines inserted, deleted or unchanged\n\
\texceeds 99999.  The header section of the s. file will record 99999\n\
\tin the appropriate field.\n"
$ The 99999 line limit in one of the statistics field has been exceeded. 
$ The command will succeed, but the field will contain the value 99999. 
