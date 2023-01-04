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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/10/20 23:16:21 $
#
# /usr/sbin/internet_mom/makefile

MOM		= internet
OTHERS		= ${MOM}
MIB		= $(MOM).mib 
MSL		= $(MOM).ms
PARFILE		= $(MOM).par
MIR		= mir.dat
ILIST		= internet_mom
IDIR		= /usr/sbin/

#EXPDIR		= /usr/include/eca/
#EXPINC_TARGETS	= export_${MIB}

MTU		= `genloc /obj/${target_machine}/usr/sbin/mtu/ntv_mtu`
MIRC		= `genloc /obj/${target_machine}/usr/sbin/mirc/ntv_mirc`
MOMGEN		= `genloc /obj/${target_machine}/usr/sbin/momgen/ntv_momgen`
MS_SCRIPT	= `genloc /src/usr/sbin/internet_mom/rfc1213_ms.scr`

ECA_MIR_BT_FILE	= ${MAKETOP}../../export/${target_machine}/etc/eca/builtin_types.dat
CHG_SRC		= change_src.make
MOM_MAKEFILE	= ${MOM}.make
MOM_BUILD	= ./

CLEANFILES	= *.make *.o *.c *.h *~ $(PARFILE) $(MSL) $(MIR)

include ${MAKEFILEPATH}/rules.mk

${MOM}	: ${MIB}
	${MTU} -i ${MIB} 
	sed 's/\<\<copywright owner\>\>/Digital Equipment Corporation/; \
	s/\<\<author\>\>/Geetha M. Brown/; \
	s/\<\<organization\>\>/OSG/; \
	s/\<\<facility\>\>/ZKO3-3/' $(PARFILE) > tmp.$$$$; \
	mv -f tmp.$$$$ $(PARFILE) 
	sed -f ${MS_SCRIPT} ${MSL} > tmp.$$$$; mv -f tmp.$$$$ ${MSL}
	( ECA_MIR_BT_FILE=${ECA_MIR_BT_FILE} ; export ECA_MIR_BT_FILE; ${MIRC} ${MSL} )
	( ECA_MIR_BT_FILE=${ECA_MIR_BT_FILE} ; export ECA_MIR_BT_FILE; \
	${MOMGEN} -i ${PARFILE} -d ${MIR} -t ${MAKETOP}../../export/${target_machine}/usr/include/eca ${MOM_BUILD} )
	${MAKE} -f ${CHG_SRC} "MOM=${MOM}" 
	mv Makefile ${MOM}.make
	${MAKE} -f ${MOM_MAKEFILE} "MOM=${MOM}_mom"
