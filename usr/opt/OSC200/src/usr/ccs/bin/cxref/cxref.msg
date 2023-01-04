 
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
$ @(#)$RCSfile: cxref.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:02:28 $
$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$len
$quote "         define quote character.

$set MS_CXREF        Define initial set#
$ The message numbers have been assigned a range of 150 - 199
$ Do not translate cxref:, %s, %d, %c. 

M_MSG_1        "cxref: Block nesting is not correct."
$ Block is a C language program technical term. 
M_MSG_2        "cxref: There cannot be more than 50 macro files.\n"
$ Do not translate this message, it currently is not used. 
M_MSG_3        "cxref: There cannot be more than 50 preprocessor parameters.\n"
M_MSG_4        "cxref: m4 failed with status %s.\n"
$ Do not translate this message, it currently is not used. 
M_MSG_5        "cxref: Assembler failed with status %d.\n"
$ Do not translate this message, it currently is not used. 
M_MSG_6        "cxref: The command /lib/cpp failed on %s.\n"
$ Do not translate /lib/cpp 
$ %s is the file name. 
M_MSG_7        "cxref: Cannot process %s.\n"
$ %s is the file name. 
M_MSG_8        "cxref"
$ Do not translate. 
M_MSG_9        "cxref: Internal software error.\n"
M_MSG_10       "cxref: Internal software error.\n"
M_MSG_11       "cxref: The sort command failed with status %d.\n"
$ Do not translate sort. 
M_MSG_12       "cxref: Cannot open %s.\n"
M_MSG_13       "Usage: cxref [-cst] [-o File] [-w[Number]]\n\
\t[[-DName[=Definition]] [-UName] [-IDirectory]] ...\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"
M_MSG_14       "cxref: %c is not a recognized flag. It is ignored.\n"
M_MSG_15       "cxref: Cannot recover from error in %s."
$ %s is a command name. 
M_MSG_16       "SYMBOL\t\tFILE\t\t\tFUNCTION   LINE\n"
$ These are headings for the C language cross reference listing.  
$ They have technical meaning to the C programmer. It would be  
$ best to leave them as they are. If you do translate, the columns should 
$ be preserved. 
