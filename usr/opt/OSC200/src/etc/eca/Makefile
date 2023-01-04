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
#/etc/eca/Makefile
# builtin_types.dat depends on .h files being exported

DATAFILES	= builtin_types.dat internet_mom.conf snmp_pe.conf
EXPLIB_TARGETS	= export_builtin_types.dat
ILIST		= builtin_types.dat internet_mom.conf snmp_pe.conf
IDIR		= /etc/eca/
IMODE		= 644

CLEANFILES	= builtin_types.dat bit.*

MCC_INTERFACE_DEF = `genloc /export/${target_machine}/usr/include/eca/mcc_interface_def.h`
NCL_INTERFACE_DEF = `genloc /export/${target_machine}/usr/include/eca/ncl_interface_def.h`

builtin_types.dat: builtin_types.proto 
	rm -rf bit.c bit.i builtin_types.dat 
	grep MCC_K_DT_ ${MCC_INTERFACE_DEF} > bit.c 
	cat ${NCL_INTERFACE_DEF} builtin_types.proto >> bit.c
	$(CC) -P bit.c 
	cat -r bit.i > builtin_types.dat 

include ${MAKEFILEPATH}/rules.mk

