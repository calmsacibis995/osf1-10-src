 
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

$ @(#)$RCSfile: spell.msg,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/01/21 14:21:38 $

$ COMPONENT_NAME: (CMDTEXT) Text Formatting Services
$ 
$ FUNCTIONS:
$ 
$ ORIGINS: 3,10,27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ messages for spell
$ 
$ spell.msg	1.4  com/bsd.d/spell.d,3.1,9011B 3/13/90 16:40:07
$ 

$set MS_SPELL
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "

$ messages for spell.sh
1 "spell: The -v choice is not supported on pdp11.\n"
$ Do not translate pdp11.
2 "spell: All but the last options are ignored.\n"
3 "spell: Cannot find or open %s.\n"
$ Do not translate %s, refers to the file.
4 "spell: Cannot identify local spell file.\n"
5 "spell: %s is not a valid flag.\n"
$ Do not translate %s.
6 "Usage: spell [-v] [-b] [-x] [-l] [-i] [+Wordlist] [-d Hashlist] [-s Hashstop] [-h Historylist] [File]\n"
7 "spell: All but the last -i/-l flags are ignored.\n"
$ messages for spellinprg.c
8 "spellin: Cannot find or open the List file %s.\n"
SPIN_ARG "spellin: The parameter count is not correct.\n"
SPIN_WID "spellin: expected code widths = %f\n"
SPIN_ITMS "spellin: %1$ld items, %2$d ignored, %3$d extra, %4$u words occupied\n"
SPIN_BITS "spellin: %1$f table bits/item, %2$f table+index bits\n"

$ messages for spellout.c
ARGCNT     "spellout: The parameter count is not correct.\n"
CANTINIT   "spellout: Cannot initialize the hash table.\n"

$ 002 new messages for spellinprg.c
SPIN_NUM "spellin: number of hash codes = %s; positive number expected\n"
SPIN_BAD "spellin: bad input; octal hash code expected\n"
SPIN_NON "spellin: no hash codes on input\n"

$ 003 new messages for hashcheck.c
HCHK_BAD "hashcheck: bad input format; compressed spelling list expected.\n"
