#!/sbin/sh 
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
# @(#)$RCSfile: security.sh,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/08/14 15:55:16 $
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#
# This script allows for security to be configured based on the value of
# SECURITY variable which is found in the /etc/rc.config file.  This script
# is called twice once at system boot by an inittab entry and then again
# when the system goes multiuser.
#


PATH=/tcb/bin:/sbin
RCCONFIG=/etc/rc.config
MATRIX_CONF=/etc/sia/matrix.conf
NAME=""

BASE_MATRIX_CONF=/etc/sia/bsd_matrix.conf
ENHANCED_MATRIX_CONF=/etc/sia/OSFC2_matrix.conf

# sanity check for missing /etc/rc.config file
if [ ! -f ${RCCONFIG} ]
then
	echo "Error: The ${RCCONFIG} file is missing"
	exit 1
else
	. ${RCCONFIG}
fi

# sanity check for c2security option

case ${SECURITY} in
	BASE|ENHANCED)
		;;
	*)
		echo "security configuration set to default (BASE)."
		SECURITY=BASE	
		;;
esac

#
# Sanity check to ensure security should be enabled.
#

if [ ! -d /tcb ]
then
	SECURITY=BASE	
fi

eval NAME=\$\{${SECURITY}_MATRIX_CONF\}

#
# Check to see if the appropriate conf file exist
#

if [ -f ${NAME} ]	
then
	rm -f ${MATRIX_CONF}			# remove the old link
	ln -s ${NAME} ${MATRIX_CONF}		# link to the conf file
else
	echo "matrix.conf unchanged."
fi
