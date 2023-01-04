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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/08/13 13:50:19 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# ---------------------------------
# 001	29-May-1991	GWS
#	termcap.src is now multiple files, split up into boilerplate sections,
#	the Specials section, manufacturer Supported and manufacturer
#	Unsupported sections.  The sections are grouped into ULTRIX Supported
#	and Unsupported sections.  The default for an ULTRIX build is to create
#	termcap.src from only the ULTRIX Supported files.  An "Unsupported"
#	build rule may be added here in the future to create termcap.src from
#	both the Supported and Unsupported files.
#
#	SCRIPTS is replaced by DATAFILES since "termcap" is a data file, rather
#	than an executable shell script.  "scripts.mk" is also replaced by
#	"datafiles.mk".
#

HEAD			= termcap.copyright termcap.header \
			  termcap.s_Specials.src

TAIL			= termcap.END

SUPPORTED_DIGITAL	= termcap.d_Digital.src
SUPPORTED_XTERM		= termcap.v_MIT_Supported.src

UNSUPPORTED_DIGITAL	= termcap.d_Digital_Unsupported.src
UNSUPPORTED_XTERM	= termcap.v_MIT_Unsupported.src

UNSUPPORTED_OTHERS	= termcap.A_Daisy.src termcap.B_ATT.src \
			  termcap.C_ControlData.src termcap.D_Datamedia.src \
			  termcap.H_Hazeltine.src termcap.I_IBM.src \
			  termcap.M_Misc.src termcap.N_AnnArbor.src \
			  termcap.P_PCkermit.src termcap.T_Teletype.src \
			  termcap.V_Visual.src termcap.X_Tektronix.src \
			  termcap.Z_Misc.src termcap.a_ADDS.src \
			  termcap.b_Beehive.src termcap.c_Concept.src \
			  termcap.h_HP.src termcap.i_Infoton.src \
			  termcap.k_Heathkit.src \
			  termcap.l_LearSiegler_ADM.src \
			  termcap.m_Microterm.src termcap.p_PerkinElmer.src \
			  termcap.q_HomeMade.src termcap.t_TI.src \
			  termcap.v_Televideo.src termcap.w_WYSE.src \
			  termcap.y_Teleray.src termcap.z_Misc.src

SUPPORTED		= ${HEAD} \
			  ${SUPPORTED_DIGITAL} \
			  ${SUPPORTED_XTERM} \
			  ${TAIL}

UNSUPPORTED		= ${HEAD} \
			  ${SUPPORTED_DIGITAL} ${UNSUPPORTED_DIGITAL}\
			  ${SUPPORTED_XTERM} ${UNSUPPORTED_XTERM} \
			  ${UNSUPPORTED_OTHERS} \
			  ${TAIL}

DATAFILES		= termcap .mrg..termcap
CLEANFILES		= termcap.src termcap

ILIST			= ${DATAFILES}
IDIR			= /usr/share/lib/
IMODE			= 644
.mrg..termcap_IMODE	= 744

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

termcap: reorder termcap.src
	ex - termcap.src < reorder

termcap.src: ${SUPPORTED}
	cat ${SUPPORTED} > termcap.src

