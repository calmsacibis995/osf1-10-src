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
# 
# @(#)$RCSfile: explain.sh,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/12/22 10:19:04 $
# 
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#
# COMPONENT_NAME: (CMDTEXT) Text Formatting Services
#
# FUNCTIONS:
#
# ORIGINS: 26,27,28
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# explain.sh	1.6 3/3/90 14:39:16

#	explain.sh	4.5	(Berkeley)	83/05/27
#
# 09/13/91 - Tom Peterson
#	- changed symbolic names to numbers according to OSF/1 1.0.2
#	- also changed explain.msg accordingly
#
D=/usr/lbin/explain.d
while	dspmsg explain.cat 1 'phrase?'
	read x
do
	case $x in
	[a-z]*)	sed -n /"$x"'.*	/s/\(.*\)	\(.*\)/use "\2" for "\1"/p' $D
	esac
done
