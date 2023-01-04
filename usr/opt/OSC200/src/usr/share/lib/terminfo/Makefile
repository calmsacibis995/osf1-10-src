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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/08/11 22:49:04 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modifications History
# ---------------------
# 001	gws	91/05/28
#	the README file is no longer installed
#	xterm.ti added for Xterm emulators
#	dec.ti was split into Supported dec.ti, Unsupported dec_Unsupported.ti
#	  and xterm.ti
#	print.ti and virtual.ti are split into Supported and Unsupported files
#	The files are grouped into Suppported, Unsupported, and common
#	  boilerplate.  By default, only the Supported and common boilerplate
#	  files are installed.  A rule may be added in the future to install
#	  the Unsupported files.
#	added CLEANFILES
#

HEAD			= header
TAIL			= trailer

SUPPORTED_DIGITAL	= dec.ti
SUPPORTED_XTERM		= xterm.ti
SUPPORTED_OTHERS	= pmcons.ti print.ti special.ti \
			  virtual.ti pccons.ti

UNSUPPORTED_DIGITAL	= dec_Unsupported.ti
UNSUPPORTED_XTERM 	=
UNSUPPORTED_OTHERS	= adds.ti annarbor.ti \
			  atcons.ti beehive.ti \
			  cdc.ti concept.ti datamedia.ti \
			  diablo.ti falco.ti \
			  general.ti hardcopy.ti hazeltine.ti \
			  heath.ti homebrew.ti hp.ti ibm.ti lsi.ti \
			  microterm.ti misc.ti pc.ti perkinelm.ti \
			  print_Unsupported.ti \
			  sun.ti tektronix.ti \
			  teleray.ti teletype.ti televideo.ti ti.ti \
			  virtual_Unsupported.ti \
			  visual.ti wyse.ti

DIGITAL			= ${SUPPORTED_DIGITAL} ${UNSUPPORTED_DIGITAL}
XTERM			= ${SUPPORTED_XTERM} ${UNSUPPORTED_XTERM}

SUPPORTED		= ${HEAD} \
			  ${SUPPORTED_DIGITAL} ${SUPPORTED_XTERM} \
			  ${SUPPORTED_OTHERS} \
			  ${TAIL}

UNSUPPORTED		= ${HEAD} \
			  ${DIGITAL} ${XTERM} \
			  ${SUPPORTED_OTHERS} ${UNSUPPORTED_OTHERS} \
			  ${TAIL}

DATAFILES		= ${SUPPORTED}

ILIST			= ${DATAFILES}
IDIR			= /usr/share/lib/terminfo/
IMODE			= 644

CLEANFILES		= ${DATAFILES}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
