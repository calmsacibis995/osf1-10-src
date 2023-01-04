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
# @(#)$RCSfile: options.sh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1994/02/14 16:53:44 $
# 
#
#  This script processes the kernel configuration files in
#  src/kernel/conf/alpha to remove references to LAT and presto
#  which are not supplied in the basic source kit. The syntax
#  is: $ options.sh [remove] [restore] 
#  
#  A reference to the file containing specifications for 
#  transfer of LAT include files is also removed.
#  
#  This script must be run prior to executing the source kit
#  build to prevent Makefiles being created by config(8)
#  referencing these subsets. Before executing paths to the
#  tools executables must be satisfied. 

case "$1"
in

	"remove")


			for fi in conf/alpha/[A-Z0-9]*[A-Z0-9] conf/alpha/[A-Z0-9]*[A-Z0-9].rt
			do
			ls $fi
			sed -e '/^options[ 	]*LAT/s/^/#/'  -e '/^pseudo-device[ 	]*presto/s/^/#/' $fi > $fi.tmp
			mv $fi.tmp $fi
			done
		
			sed -e '/^SUBDIRS/s/lat//' include/dec/Makefile > include/dec/Makefile.tmp
			mv -f include/dec/Makefile.tmp include/dec/Makefile;
			;;
	"restore")
			for fi in conf/alpha/[A-Z0-9]*[A-Z0-9] conf/alpha/[A-Z0-9]*[A-Z0-9].rt
			do
			ls $fi
			sed -e '/^#options[ 	]*LAT/s/#//'  -e '/^#pseudo-device[ 	]*presto/s/#//' $fi > $fi.tmp
			mv $fi.tmp $fi
			done
		
			grep 'SUBDIRS.*lat' include/dec/Makefile 

			if [  $? !=  0 ]; then
			sed -e '/^SUBDIRS.*\=[ 	]*/s/SUBDIRS[ 	]*\=/&	lat/' include/dec/Makefile > include/dec/Makefile.tmp
			mv -f include/dec/Makefile.tmp include/dec/Makefile;
			fi
			;;

		*)
			echo "Usage: options [remove] [restore]";
			;;

esac
