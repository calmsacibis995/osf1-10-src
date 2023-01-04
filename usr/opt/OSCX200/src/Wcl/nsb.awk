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
#!/bin/awk
# SCCS_data: @(#) nsb.awk 1.4 92/03/17 16:01:59
# 
# Eat lines which look like cpp file syncronization lines (# number "file")
# and collpse multiple blank lines so only one blank line is printed.
#
# Intended to be used as a postprocessing filter following cpp which leaves
# blank lines whenever it sees a cpp directive or a C comment.
#
BEGIN				{ prevBlankLine = 0 }

$1 == "#" && $2 ~ /^[0-9]+$/ 	{ prevBlankLine++ ; next }

NF == 0				{ if (!prevBlankLine)
				  { prevBlankLine++ ; print }
				  next
				}

				{ prevBlankLine = 0 ; print }
