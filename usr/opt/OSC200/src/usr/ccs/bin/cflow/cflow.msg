 
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
$ @(#)$RCSfile: cflow.msg,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/29 17:58:20 $
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

$set MS_DAG        Define initial set#
$
$ Do not translate cflow:, %s, %c.  
SKIP         "cflow: File %s must have a .o, .s, .i, .c, .l, or .y extension.\n"
$ Do not translate .o, .s, .i, .c, .l, .y  
USAGE    "Usage: cflow [-dNumber] [-i_] [-ip] [-ix] [-r] [-MA]\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"
DBOPT    "cflow: %c is not a recognized flag or is not used correctly.\n"
$ %c is a flag.  
TMCHS    "cflow: There is not enough memory available to add more characters.\n"
TMNDS    "cflow: There is not enough memory available to add more nodes.\n"
TMLNKS   "cflow: There is not enough memory available to add more links.\n"
