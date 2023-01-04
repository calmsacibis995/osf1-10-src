#!/usr/bin/sh
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
#	@(#)installbsd.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0



#
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
#
# FUNCTIONS: install
#
# ORIGINS: 26, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# 
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
# install.sh	1.5  com/cmd/files/installbsd,3.1,9021 1/18/90 15:52:57
 
#
# NAME: install (shell script)
#
# FUNCTION: installs a command - BSD version.
#
# EXECUTION ENVIRONMENT: Shell 
#
# NOTES:
# 	Possible flags:
#	-c	moves binary to destination.
#	-m	specifies the mode of the destination file
#	-o	specifies the owner of the destination file
#	-g	specifies the group of the destination file
#	-s 	strippes binary after being installed	
#
# RETURNS: 0 if it is successful; 1 if it is unsuccessful.
#

#
# initializes some shell variables
#
cmd=""
stripbefore=""
stripafter=""
chmod="/bin/chmod -f 755"
chown="/bin/chown -f root"
chgrp="/bin/chgrp -f staff"
# 
# load messages from file
#
if CANTMV=`dspmsg -s 1 installbsd.cat 4 'install: cannot move'`
then :
else CANTMV="install: cannot move"
fi
if ITSELF=`dspmsg -s 1 installbsd.cat 5 'onto itself'`
then :
else ITSELF="onto itself"
fi
if CANTOPEN=`dspmsg -s 1 installbsd.cat 6 'install: cannot open'`
then :
else CANTOPEN="install: cannot open"
fi
#
# parses the arguments
#
while true ; do
	case $1 in
		-s )	if [ $cmd ]
			then	stripafter="/usr/bin/strip"
			else	stripbefore="/usr/bin/strip"
			fi
			shift
			;;
		-c )	if [ $cmd ]
			then
				if [ -x dspmsg ]; then
					echo `dspmsg -s 1 installbsd.cat 1 'install: multiple specifications of -c'`
				else
					echo "install: multiple specifications of -c"
				fi
				exit 1
			fi
			cmd="/usr/bin/cp"
			stripafter=$stripbefore
			stripbefore=""
			shift
			;;
		-m )	chmod="/usr/bin/chmod -f $2"
			shift
			shift
			;;
		-o )	chown="/usr/bin/chown -f $2"
			shift
			shift
			;;
		-g )	chgrp="/usr/bin/chgrp -f $2"
			shift
			shift
			;;
		* )	break
			;;
	esac
done
# if -c flag is not on, then the binary is moved to destination
if [ $cmd ]
then true
else cmd="/usr/bin/mv"
fi
#
# checks errors
#
if [ ! ${2-""} ]
then
	if [ -x dspmsg ]; then
		echo `dspmsg -s 1 installbsd.cat 2 'install: no destination specified'`
	else
		echo "install: no destination specified"
	fi
	exit 1
fi
if [ ${3-""} ]
then
	if [ -x dspmsg ]; then
		echo `dspmsg -s 1 installbsd.cat 3 'install: too many files specified -> $*'`
	else
		echo "install: too many files specified -> $*"
	fi
	exit 1
fi
if [ $1 = $2 -o $2 = . ]; then
	echo "$CANTMV $1 $ITSELF"
	exit 1
fi
if [ '!' -f $1 ]; then
	echo "$CANTOPEN $1"
	exit 1
fi
# if the destination is directory then binary is moved into the destination
# directory with its original file-name.
if [ -d $2 ]
then	file=$2/`basename $1`
else	file=$2
fi

/usr/bin/rm -f $file
if [ $stripbefore ]
then	$stripbefore $1
fi
$cmd $1 $file
if [ $stripafter ]
then	$stripafter $file
fi
$chown $file
$chgrp $file
$chmod $file
