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
# @(#)$RCSfile: OSFX130.k,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/07/08 18:38:45 $
#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1992    *
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
# 		OSFX130.k
#
# global (product-wide) definitions
NAME='DEC OSF/1 V1.3 (Alpha APX) Worksystem Software ( Rev. 930623 )'
CODE=OSW
VERS=130
MI=OSFX130.mi
ROOT=0
RXMAKE=0
COMPRESS=1
#
#  The Flag expressions will force a mup subset to be mandatory when 
#  the parent subset lock file is present on the system.
#
FLGEXPDECW='`if [ -f ./usr/.smdb./OSFDECW120.lk ]; then echo 0 ; else echo 2 ; fi  `'
FLGEXPEMACS='`if [ -f ./usr/.smdb./OSFEMACS120.lk ]; then echo 0 ; else echo 2 ; fi  `'
FLGEXPEMACSRC='`if [ -f ./usr/.smdb./OSFEMACSRC120.lk ]; then echo 0 ; else echo 2 ; fi  `'
FLGEXPSER='`if [ \`sizer -wu\` -gt 0 -a -f ./usr/.smdb./OSFSER120.lk ]; then echo 0; else echo 2; fi`'
FLGEXPX11='`if [ \`sizer -wu\` -gt 0 -a -f ./usr/.smdb./OSFX11120.lk ]; then echo 0; else echo 2; fi`'
FLGEXPFONT15='`if [ \`sizer -wu\` -gt 0 -a -f ./usr/.smdb./OSFFONT15120.lk ]; then echo 0; else echo 2; fi`'
FLGEXPXCDA='`if [ -f ./usr/.smdb./OSFXCDA120.lk ]; then echo 0 ; else echo 2 ; fi  `'
FLGEXPXCDADEV='`if [ -f ./usr/.smdb./OSFXCDADEV120.lk ]; then echo 0 ; else echo 2 ; fi  `'
FLGEXPXDEV='`if [ -f ./usr/.smdb./OSFXDEV120.lk ]; then echo 0 ; else echo 2 ; fi  `'
FLGEXPXMAIL='`if [ -f ./usr/.smdb./OSFXMAIL120.lk ]; then echo 0 ; else echo 2 ; fi  `'
FLGEXPXMIT='`if [ -f ./usr/.smdb./OSFXMIT120.lk ]; then echo 0 ; else echo 2 ; fi  `'
%%
OSFSER130	OSFSER120	FLGEXPSER	'X Servers UPGRADE%Windowing Environment'
OSFX11130	OSFX11120	FLGEXPX11	'Basic X Environment UPGRADE%Windowing Environment'
OSFDECW130	OSFDECW120	FLGEXPDECW	'Additional DECwindows Appl UPGRADE%Windows Applications'
OSFXMIT130	OSFXMIT120	FLGEXPXMIT	'Additional X Applications UPGRADE%Windows Applications'
OSFXDEV130	OSFXDEV120	FLGEXPXDEV	'X Window Software Development UPGRADE%Software Development'
OSFEMACS130	OSFEMACS120	FLGEXPEMACS	'GNU Emacs UPGRADE%General Applications'
FSFEMACSSRC130	.	FLGEXPEMACSRC	'GNU Emacs Source%GNU Source'
OSFXDEBUG130	.	2	'X Debug library UPGRADE%X Debug'
