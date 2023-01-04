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
awk '
/^     1/ && READY == 0 {	printf ("    1   ");
		for (i = 2; i < NF; i++)
			printf ("          %2d        ", i);
		printf ("   Operation\n");
		next;
	}
/^---/	{ 	printf ("--------");
		for (i = 2; i <= NF; i++)
			printf ("   -----------------");
		printf ("\n");
		READY=1; next;
 	}
READY==1 {
		base=$1;
		printf ("%8.1f", base);
		for (i = 2; i < '$1'; i++) {
			if (base == 0)
				printf ("   %8.1f         ", $i);
			else {
				rate=$i/base;
				if (rate < .1)
					printf ("   %8.1f (%6.3f)", $i, rate);
				else if (rate < 1000)
					printf ("   %8.1f (%6.2f)", $i, rate);
				else
					printf ("   %8.1f (%6.0f)", $i, rate);
			}
		}
		printf ("   ");
		for (; i <= NF; i++)
		{
			printf ("%s ", $i);
		}
		printf ("\n");
		next;
	   }
	   { print $0; }
'
