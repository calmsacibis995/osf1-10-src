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
# HISTORY
#
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/10/21 23:08:46 $
#
#/usr/sbin/mir_dat/Makefile
# Produce "mir.dat" from set of mibs
#
# To Extend mir.dat: Extend (mirc)order-dependent MIB_LIST
#
MIR		= mir.dat
OTHERS		= ${MIR}

ILIST		= ${MIR}
IDIR		= /etc/eca/
IMODE		= 644

MSL_SRC_LIST	= ${MAKETOP}usr/sbin/internet_mom/internet.ms
MIB_LIST	= ${MAKETOP}usr/sbin/fddi_mom/fddi.mib ${MAKETOP}usr/sbin/trn_mom/trn.mib
MSL_LIST	= $(MIB_LIST:.mib=.ms)

MTU		= `genloc /obj/${target_machine}/usr/sbin/mtu/ntv_mtu`
MIRC		= `genloc /obj/${target_machine}/usr/sbin/mirc/ntv_mirc`
MOMGEN		= `genloc /obj/${target_machine}/usr/sbin/momgen/ntv_momgen`

ECA_MIR_BT_FILE	= ${MAKETOP}../../export/${target_machine}/etc/eca/builtin_types.dat

CLEANFILES	= *.ms ${MIR}

include ${MAKEFILEPATH}/rules.mk

${MIR}	: ${MIB_LIST} ${MSL_SRC_LIST}
##	${MTU} ${MIB_LIST} 
	( ECA_MIR_BT_FILE=${ECA_MIR_BT_FILE} ; export ECA_MIR_BT_FILE; ${MIRC} ${MSL_SRC_LIST} ${MSL_LIST} )

