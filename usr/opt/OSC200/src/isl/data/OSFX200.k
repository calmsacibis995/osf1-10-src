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
# @(#)$RCSfile: OSFX200.k,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/14 20:23:46 $
# 
# 		OSFX200.k
#
# global (product-wide) definitions
NAME='DEC OSF/1 X2.0-Prebl9 Alpha APX Worksystem Software ( Rev. 931214 )'
CODE=OSW
VERS=200
MI=OSFX200.mi
ROOT=0
RXMAKE=0
COMPRESS=1
FLGEXP1='`if [ $ADVFLAG ]; then echo 0; else echo 2; fi `'
FLGEXP2='`if [ "$WHOAMI" = root ]; then echo 0; else echo 2; fi`'
FLGEXP3='`if [ "$WHOAMI" = root ]; then echo 0; else echo 2; fi`'
FLGEXP4='`if [ "$WHOAMI" = root ] && sizer -wu | egrep -s 0; then echo 2; else echo 0; fi`'
FLGEXPSCP='`DEFPATH=$DEFPATH CMDSW=$CMDSW ACT=C $L_DIR/$L_SUB.scp GETFLAG`'
# subset definitions follow
%%
OSFSER200	OSFBASE200	FLGEXPSCP	'X Servers%Windowing Environment'
OSFX11200	OSFBASE200	FLGEXPSCP	'Basic X Environment%Windowing Environment'
OSFFONT15200	OSFBASE200	FLGEXPSCP	'DECwindows 100dpi Fonts%Windowing Environment'
OSFDPSFONT200	OSFBASE200	FLGEXPSCP	'Adobe Fonts%Windowing Environment'
OSFMITFONT200	OSFBASE200	FLGEXPSCP	'X Fonts%Windowing Environment'
OSFFONT200	OSFBASE200	FLGEXPSCP	'DECwindows 75dpi Fonts%Windowing Environment'
OSFDECW200	OSFBASE200	2	'Additional DECwindows Applications%Windows Applications'
OSFXDEV200	OSFBASE200	2	'X Window Software Development%Software Development'
OSFXMIT200	OSFBASE200	2	'Additional X Applications%Windows Applications'
OSFXMAIL200	OSFBASE200|OSFMH200	2	'DECwindows Mail Interface%Mail Applications'
OSFXCDADEV200	OSFBASE200|OSFXDEV200	2	'CDA(tm) for X/Motif Development%Software Development'
OSFEMACS200	OSFBASE200	2	'GNU Emacs%General Applications'
FSFEMACSSRC200	OSFBASE200	2	'GNU Emacs Source%GNU Source'
OSFXC2SEC200	OSFBASE200	2	'C2-Security GUI%System Administration'
OSFMOTDEM200	OSFBASE200	2	'Additional Motif Demo Sources%Software Development'
OSFXDEBUG200	.	2	'X Debug library%X Debug'
