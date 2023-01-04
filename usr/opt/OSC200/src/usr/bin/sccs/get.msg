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
$ @OSF_COPYRIGHT@
$ 
$ 
$ HISTORY

$
$ @(#)$RCSfile: get.msg,v $ $Revision: 4.2.3.2 $ (OSF) $Date: 1993/08/03 21:13:48 $
$
$ Copyright International Business Machines Corp. 1989
$ Unpublished Work
$ All Rights Reserved
$ Licensed Material - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$    get.msg 1.6  com/cmd/sccs/hdr/get.msg, 9121320k, bos320 4/25/91 19:26:56
$
$quote "         define quote character.

$ Do not translate get:
$ Do not translate the paranthesis (ge#) at the end of each line.
$len 
$set 7        Define initial set#

ENOTWITHM            "\n\
get: Cannot use the -e flag with the -m flag. (ge3)\n"
$ Do not translate -e, -m
WRITFILEEXST         "get: A writable version of %s exists. (ge4)\n"
ALRDYINC             "\n\
get: The delta %s is already included. (ge9)\n"
$ %s is a delta. 
ALRDYEXCLD           "\n\
get: The delta %s is already excluded. (ge10)\n"
$ %s is a delta. 
INTRNERR11           "\n\
get: There is an internal software error. (ge11)\n"
BNGEDT               "get: Another user is editing %s. (ge17)\n"
WARNMSG              "WARNING: The file %s is being edited. (ge18)\n"
SERNOTOOLRG          "\n\
get: The specified serial number does not exist. (ge19)\n"
$ you can do a get -a to specify delta serial number 
$ instead of SID number.  This is not documented in commands reference. 
$ By the way, the serial number overrides the SID number 
CMRTOOLNG            "\n\
get: The CMR list is too long."
$ Do not translate this message. The user will never see it. 
ULINES               "%u lines\n"
$ %u is the number of lines. 
BADSIDCALC           "\n\
get: There is a bad SID calculated in newsid()."
$ Do not translate this message. The user will never see it. 
NEWDELTA             "new delta %s\n"
$ %s is the new delta. 
CKCASSINOCMR         "\n\
get: Background CASSI get with no CMRs.\n"
$ Do not translate this message. The user will never see it. 
BKCASSIINVCMR        "\n\
get: Background CASSI get with invalid CMR.\n"
$ Do not translate this message. The user will never see it. 
COMMACMR             "Input Comma Separated List of CMRs: "
$ Do not translate this message. The user will never see it. 
TOOMANYCMR           "get: Too many CMRs.\n"
$ Do not translate this message. The user will never see it. 
ONEVALCMR            "get: Must enter at least one valid CMR.\n"
$ Do not translate this message. The user will never see it. 
REENTCMR             "get: Re-enter invalid CMRs, or press return.\n"
$ Do not translate this message. The user will never see it. 
GETUSAGE	" Specify the file to process.  (cm3)\n\
\tUsage: get [-Lbegkmnpst] [-c Cutoff] [-i List]\n\
\t\t[-r SID] [-w String] [-x List] [-l | -lp] file... | -\n"
GETOPTION	" Flag -%c is not valid. (cm1)\n\
\tUsage: get [-Lbegkmnpst] [-c Cutoff] [-i List]\n\
\t\t[-r SID] [-w String] [-x List] [-l | -lp] file... | -\n"
ARGRQD		" Flag -%c requires an argumnet\n\
\tUsage: get [-Lbegkmnpst] [-c Cutoff] [-i List]\n\
\t\t[-r SID] [-w String] [-x List] [-l[p]] file... | -\n"
