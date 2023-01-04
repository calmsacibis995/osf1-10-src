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
# 
# @(#)$RCSfile: security.mk,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/04/01 20:27:06 $
# 

# ACL_POLICY		ACL_POSIX or ACL_SWARE
# ACL_ARCH		tagged file system.

#
#  'Soft' security level for datafile rules
#

SEC_SLEVEL	= ${SEC_LEVEL?${SEC_LEVEL}:C2}

#
#  Security flags
#

_SEC_C2_DEFS_	= -DSEC_BASE \
		  ${ACL_POLICY?-DSEC_${ACL_POLICY}:} \
		  ${SEC_ARCH?-DSEC_ARCH:}

_SEC_B1_DEFS_	= ${_SEC_C2_DEFS_} \
		  -DSEC_PRIV \
		  -DSEC_MAC_OB

_SET_SEC_DEFS_=${SEC_DEFS?SEC_DEFS:${SEC_LEVEL?SEC_DEFS:}}
${_SET_SEC_DEFS_}=${SEC_DEFS?${SEC_DEFS}:${_SEC_${SEC_LEVEL}_DEFS_}}

SEC_LIBS=${SEC_DEFS?-lsecurity -laud:}
#SEC_LIBS=${SEC_DEFS?-lsecurity:} -laud 

PROGRAMS_C2	= ${PROGRAMS_BASE} \
		  ${ACL_POLICY?${PROGRAMS_ACL} ${PROGRAMS_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${PROGRAMS_ARCH}:}

PROGRAMS_B1	= ${PROGRAMS_C2} \
		  ${PROGRAMS_PRIV} \
		  ${PROGRAMS_MAC}

LIBRARIES_C2    = ${LIBRARIES_BASE} \
		  ${ACL_POLICY?${LIBRARIES_ACL} ${LIBRARIES_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${LIBRARIES_ARCH}:}

LIBRARIES_B1	= ${LIBRARIES_C2} \
		  ${LIBRARIES_PRIV} \
		  ${LIBRARIES_MAC}

OFILES_C2	= ${OFILES_BASE} \
		  ${ACL_POLICY?${OFILES_ACL} ${OFILES_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${OFILES_ARCH}:}

OFILES_B1	= ${OFILES_C2} \
		  ${OFILES_PRIV} \
		  ${OFILES_MAC}

OBJECTS_C2	= ${OBJECTS_BASE} \
		  ${ACL_POLICY?${OBJECTS_ACL} ${OBJECTS_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${OBJECTS_ARCH}:}

OBJECTS_B1	= ${OBJECTS_C2} \
		  ${OBJECTS_PRIV} \
		  ${OBJECTS_MAC}

SCRIPTS_C2	= ${SCRIPTS_BASE} \
		  ${ACL_POLICY?${SCRIPTS_ACL} ${SCRIPTS_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${SCRIPTS_ARCH}:}

SCRIPTS_B1	= ${SCRIPTS_C2} \
		  ${SCRIPTS_PRIV} \
		  ${SCRIPTS_MAC}

DATAFILES_C2    = ${DATAFILES_BASE} \
		  ${ACL_POLICY?${DATAFILES_ACL} ${DATAFILES_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${DATAFILES_ARCH}:}

DATAFILES_B1	= ${DATAFILES_C2} \
		  ${DATAFILES_PRIV} \
		  ${DATAFILES_MAC}

OTHERS_C2	= ${OTHERS_BASE} \
		  ${ACL_POLICY?${OTHERS_ACL} ${OTHERS_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${OTHERS_ARCH}:}

OTHERS_B1	= ${OTHERS_C2} \
		  ${OTHERS_PRIV} \
		  ${OTHERS_MAC}

MANPAGES_C2	= ${MANPAGES_BASE} \
		  ${ACL_POLICY?${MANPAGES_ACL} ${MANPAGES_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${MANPAGES_ARCH}:}

MANPAGES_B1     = ${MANPAGES_C2} \
		  ${MANPAGES_PRIV} \
		  ${MANPAGES_MAC}

DOCUMENTS_C2	= ${DOCUMENTS_BASE} \
		  ${ACL_POLICY?${DOCUMENTS_ACL} ${DOCUMENTS_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${DOCUMENTS_ARCH}:}

DOCUMENTS_B1	= ${DOCUMENTS_C2} \
		  ${DOCUMENTS_PRIV} \
		  ${DOCUMENTS_MAC}

SUBDIRS_C2      = ${SUBDIRS_BASE} \
		  ${ACL_POLICY?${SUBDIRS_ACL} ${SUBDIRS_${ACL_POLICY}}:} \
		  ${SEC_ARCH?${SUBDIRS_ARCH}:}

SUBDIRS_B1      = ${SUBDIRS_C2} \
		  ${SUBDIRS_PRIV} \
		  ${SUBDIRS_MAC}

