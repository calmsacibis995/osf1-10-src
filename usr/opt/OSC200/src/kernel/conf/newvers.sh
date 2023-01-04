#!/bin/sh -
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
# @(#)$RCSfile: newvers.sh,v $ $Revision: 4.4.16.4 $ (DEC) $Date: 1993/05/08 10:44:37 $ 
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
#
# OSF/1 Release 1.0

#
# Copyright (c) 1980, 1986 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#

#
#
case $# in
	7)
		copyright="$1"; type="$2"; major="$3"; min="$4";variant="";rev="$5"
		;;
	*)
		copyright="$1"; type="$2"; major="$3"; min="$4";variant="$5 ";rev="$6"
		;;
esac

# ULTRIX vs OSF Bourne shell, which is wrong?
if [ `/bin/uname` != OSF1 ]
then
	nl="\\n"
else
	nl="\\\n"
fi
t=`date`
v="${type}${major}.${min}"
CONFIG=`cat vers.config`
if [ -z "${CONFIG}" ]; then
    exit 1
fi
#
# Compute a build number from the local build directory
#
if [ -f buildnumber ]
then
	bn=`cat buildnumber`
	echo `expr $bn + 1` >buildnumber
else
	bn=0
	echo 0 >buildnumber
fi
(
  echo "#ifdef __STDC__" ;
  echo "#define CONST const" ;
  echo "#else" ;
  echo "#define CONST" ;
  echo "#endif" ;
  echo "CONST int  version_major      = ${major};" ;
  echo "CONST int  version_minor      = ${min};" ;
  echo "CONST char version_version[32]  = \"${rev}\";" ;
  echo "CONST char version_release[32]  = \"${type}${major}.${min}\";" ;
  echo "#ifdef REALTIME" ;
  echo "CONST char version[] = \"DEC OSF/1 [RT] ${v}${variant} (Rev. ${rev}); ${t} ${nl}\";" ;
  echo "#else" ;
  echo "CONST char version[] = \"DEC OSF/1 ${v}${variant} (Rev. ${rev}); ${t} ${nl}\";" ;
  echo "#endif" ;
  echo "CONST char copyright[] = \"\\" ;
  sed <$copyright -e '/^#/d' -e 's;[ 	]*$;;' -e '/^$/d' -e 's;$;\\n\\;' ;
  echo "\";";
) > vers.c
if [ -s vers.suffix -o ! -f vers.suffix ]; then
    echo ".${CONFIG}" >vers.suffix
fi
exit 0
