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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/08/10 20:52:43 $
#
# /usr/sbin/snmp_pe/Makefile
# Makefile for producing and installing Common Agent SNMP PE
# Issue:	i18n

PROGRAMS 		= snmp_pe
ILIST			= snmp_pe
IDIR			= /usr/sbin/


INCFLAGS	        = -I. -I${MAKETOP}../../export/${target_machine}/usr/include/eca
LIBS                    = -lmir -lmoss -lpthreads -lmach -lc_r  

CLEANFILES 		= *.o *.cat *.a 

HFILES 			= asn_def.h snmp_def.h snmppe.h snmppe_interface_def.h snmppe_snmplib.h \
	                  port_cmip.h snmp_service_def.h snmppe_descrip.h snmppe_msg.h

OFILES			= mcc_asn1.o mcc_asn1_dump.o mcc_asn1_extra.o mcc_asn1_objid.o \
			  sck_pei_sstb.o snmp_lib.o snmppe_carecv.o snmppe_casend.o \
			  snmppe_init.o snmppe_main.o snmppe_mir.o snmppe_netio.o snmppe_text.o

include ${MAKEFILEPATH}/rules.mk

