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
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: runcat.sh,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 16:56:51 $
#
#
# HISTORY
#
# OSF/1 1.2
#
# COMPONENT_NAME: CMDMSG
#
#  FUNCTIONS: runcat.sh
# 
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# 1.10  com/cmd/msg/runcat.sh, bos, bos320 10/17/90 16:59:40


#
#  NAME: runcat
#
#  FUNCTION: Produce message header file and message catalog
#
#  NOTES: Produces message header file and catalog from message definition 
#         files.
#
#  RETURNS: Return code from last command.
#           1 - error encountered.
#

MK=1

if [ "$#" -lt 2 -o "$#" -gt 3 ]
then
     if [ -x /usr/bin/dspmsg ];
	then /usr/bin/dspmsg -s 5 msgfac.cat 1 "usage: runcat catname srcfile [catfile]\n";
        else echo "usage: runcat catname srcfile [catfile]";
     fi
     exit 1
fi

# verify that source file exists and contains messages (or is stdin)

if [ "$2" != "-" ]; then
	if [ -r $2 ]; then
		if `grep '^\$set' $2 >/dev/null`; then true;
		else
	     		if [ -x /usr/bin/dspmsg ]; then
				/usr/bin/dspmsg -s 5 msgfac.cat 2 "No \$set in source\n";
       	        	else echo "No \$set in source";
     	     		fi
     	     		exit 1;
		fi
	else
     		if [ -x /usr/bin/dspmsg ]; then
			/usr/bin/dspmsg -s 5 msgfac.cat 3 "Can't open %s\n" $2;
        	else echo "Can't open $2"
     		fi
     		exit 1;
	fi
fi
# set name of catalog file

if [ "$#" -eq 2 ]
	then CATFILE="$1.cat"
	else CATFILE="$3"
fi

# run mkcatdefs only if at least one line starts with character

if [ "$2" != "-" ]; then
	if `grep '^[A-Za-z]' $2 >/dev/null`; then MK=1; else MK=0; fi
	if [ $MK -eq 0 ]; then
		if `grep '^\$set *[A-Za-z]' $2 >/dev/null`; then MK=1; fi;
	fi
fi

if [ $MK -eq 1 ]
	then
	    	mkcatdefs $1 $2 | gencat $CATFILE
	else
		gencat $CATFILE $2;

fi
