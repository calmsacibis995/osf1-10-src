#!/bin/sh -x
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
# @(#)$RCSfile: lat_links.sh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1994/02/15 20:01:12 $
# 

#  lat_links.sh
#
#  This script establishes links between the installation binary LAT files
#  and the source kit. It is invoked with the syntax:  $ lat_links
#  from the directory: src/kernel. This script can only be run after the
#  main build has finished.
#  
#  


for fi in ` awk '/optional lat Binary/ {print $1}' /usr/sys/conf/files | \
		 sed -e 's/dec\/lat\///' -e 's/\.c/\.o/' `
	do
		ln -s /usr/sys/BINARY/$fi ../../obj/alpha/kernel/BINARY/$fi
		ln -s /usr/sys/BINARY.rt/$fi ../../obj/alpha/kernel/BINARY.rt/$fi
	done
