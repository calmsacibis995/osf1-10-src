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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:36:31 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# ---------------------------------
# 28-May-1991   GWS
#	all the *.uu files were decoded and most placed under SCCS control,
#	  so rules to handle .uu files are deleted.  See the README file in
#	  this directory for list of files NOT under SCCS control.
#	vt100[.uu] replaced by 2 files:
#		vt100	- for VT100 series and VT200 series terminals running
#			  in 80-column mode
#		vt100-w	- for VT100 series and VT200 series terminals running
#			  in 132-column mode
#
#	vt300 is added for VT300 series and VT400 series terminals.  It handles
#	both 80-column and 132-column modes.
#
#	The files are divided up into Supported and Unsupported groups.  The
#	Supported files are DATAFILES, since they are no longer uuencoded.
#	By default, only the Supported files are built and installed.  An
#	"Unsupported" build rule may be added at a future date.
#
#	OTHERS is deleted since all files are now uudecoded.
#	others.mk is deleted too.
#
#	DATAFILES is added, pointing only to Supported files.
#	datafiles.mk is also added. 
#	
#	ILIST is changed to reference only the DATAFILES files.

SUPPORTED		= vt100 vt100-w vt300
UNSUPPORTED		= 3101 aa aed512 beehive diablo dtc382 ibm3101 \
			  std stdcrt tandem653 teleray wyse-adds \
			  xerox1720 xerox1730-lm xerox1730 zenith29

DATAFILES		= ${SUPPORTED}

CLEANFILES		= ${DATAFILES}

ILIST			= ${DATAFILES}
IDIR			= /usr/share/lib/tabset/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
