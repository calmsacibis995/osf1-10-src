# global (product-wide) definitions
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
# @(#)$RCSfile: OSFX140.k,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/05/24 17:09:33 $
#
NAME='DEC OSF/1 X1.4-3 Alpha APX Worksystem Software ( Rev. 930520 )'
CODE=OSW
VERS=140
MI=OSFX140.mi
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
OSFSER140	OSFBASE140	FLGEXPSCP	'X Servers%Windowing Environment'
OSFX11140	OSFBASE140	FLGEXPSCP	'Basic X Environment%Windowing Environment'
OSFFONT15140	OSFBASE140	FLGEXPSCP	'DECwindows 100dpi Fonts%Windowing Environment'
OSFDPSFONT140	OSFBASE140	FLGEXPSCP	'Adobe Fonts%Windowing Environment'
OSFMITFONT140	OSFBASE140	FLGEXPSCP	'X Fonts%Windowing Environment'
OSFFONT140	OSFBASE140	FLGEXPSCP	'DECwindows 75dpi Fonts%Windowing Environment'
OSFDECW140	OSFBASE140	2	'Additional DECwindows Applications%Windows Applications'
OSFXDEV140	OSFBASE140	2	'X Window Software Development%Software Development'
OSFXMIT140	OSFBASE140	2	'Additional X Applications%Windows Applications'
OSFXMAIL140	OSFBASE140|OSFMH140	2	'DECwindows Mail Interface%Mail Applications'
OSFXCDADEV140	OSFBASE140|OSFXDEV140	2	'CDA(tm) for X/Motif Development%Software Development'
OSFEMACS140	OSFBASE140	2	'GNU Emacs%General Applications'
FSFEMACSSRC140	OSFBASE140	2	'GNU Emacs Source%GNU Source'
OSFXC2SEC140	OSFBASE140	2	'X C2 Security%System Administration'
OSFXDEBUG140	.	2	'X Debug library%X Debug'
