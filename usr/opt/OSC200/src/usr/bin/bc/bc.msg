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
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ @(#)$RCSfile: bc.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 14:50:41 $
$
$ 
$ HISTORY
$ 
$ OSF/1 1.2
$
$ COMPONENT_NAME: (CMDCALC) calculators
$
$ FUNCTIONS: 
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ bc.msg	1.4  com/cmd/calc,3.1,9013 3/9/90 14:52:38

$quote "
$set MS_BC
BUND "bundle %d elements at 0x%x\n"
ROUT "routput(0x%x)\n"
INVINP "%s on line %d %s"
NOBUNDLE "bundling space exceeded"
$ Do not put a new line character at the end. INVINP will follow it.
NOINFILE "cannot open input file"
$ Do not put a new line character at the end. INVINP will follow it.
NOSTRSPC "string space exceeded"
$ Do not put a new line character at the end. INVINP will follow it.
TOOBIG "program too big"
$ Do not put a new line character at the end. INVINP will follow it.
USAGE "usage: bc [-cl] [files...]\n"
