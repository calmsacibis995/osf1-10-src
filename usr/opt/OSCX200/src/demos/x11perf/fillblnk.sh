#!/bin/sh
# 
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
# HISTORY
# 
echo '#####EOF#####' | cat $2 - $1 |
awk -F: '\
$1 == "#####EOF#####"	{ filling = 1; currentItem = 1; lastItem = NR; next; }
filling != "1"	{ itemOrder[" " $1] = NR; name[NR] = $1; }
filling == "1"	{ rate[itemOrder[$2]] = $1; }
END	{
	for (i = 1; i < lastItem; i++) {
		if (rate[i] != "") {
			printf ("%s: %s\n", rate[i], name[i]);
		} else {
			printf (" 0 trep @ 0.0 msec (0.0/sec): %s\n", name[i]);
		}
	}
	}'
