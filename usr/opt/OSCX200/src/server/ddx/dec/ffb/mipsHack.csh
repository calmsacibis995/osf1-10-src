#!/bin/csh
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
# @(#)$RCSfile: mipsHack.csh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:20:00 $
# 
#!/bin/csh -f
set acc_copy=`grep ZZZ_copy Makefile | cut -c10-256`
set acc=`grep YYY_multicompile Makefile | cut -c20-256`
set acc_lnk=`grep WWW_links Makefile | cut -c12-256`
sed -e "s/_copyZ/$acc_copy/" -e "s/_multicompileY/$acc/" -e "s/_linkW/$acc_lnk/" Makefile > tmp
mv -f tmp Makefile

echo "Makefile variables "
echo "		MULTICOMPILE_SRCS_EXPANDED and "
echo "		COPY_SRCS_EXPANDED"
echo "		LINKED_SRCS"
echo " have been expanded"
