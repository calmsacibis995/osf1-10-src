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
#	@(#)$RCSfile: datafiles.mk,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/16 17:59:02 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
#  Definitions for .INOBJECTDIR rule
#
_INOBJECTDIR_OPTIONS_=${INOBJECTDIR_OPTIONS?${INOBJECTDIR_OPTIONS}:-e ''}
_INOBJECTDIR_=${INOBJECTDIR?${INOBJECTDIR}:${_DATAFILES_}}

#
#  Default update rule
#
.INOBJECTDIR: ${_INOBJECTDIR_}
	${SED} ${${@}_SED_OPTIONS?${${@}_SED_OPTIONS}:${SED_OPTIONS}} ${_INOBJECTDIR_OPTIONS_} <$@ >./$@.X
	${MV} -f ./$@.X ./$@

