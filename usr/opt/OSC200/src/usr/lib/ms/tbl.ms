...\"
...\" *****************************************************************
...\" *                                                               *
...\" *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
...\" *                                                               *
...\" *   All Rights Reserved.  Unpublished rights  reserved  under   *
...\" *   the copyright laws of the United States.                    *
...\" *                                                               *
...\" *   The software contained on this media  is  proprietary  to   *
...\" *   and  embodies  the  confidential  technology  of  Digital   *
...\" *   Equipment Corporation.  Possession, use,  duplication  or   *
...\" *   dissemination of the software and media is authorized only  *
...\" *   pursuant to a valid written license from Digital Equipment  *
...\" *   Corporation.                                                *
...\" *                                                               *
...\" *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
...\" *   by the U.S. Government is subject to restrictions  as  set  *
...\" *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
...\" *   or  in  FAR 52.227-19, as applicable.                       *
...\" *                                                               *
...\" *****************************************************************
...\"
...\"
...\" HISTORY
...\"
.\"
.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\"
.\"
.\" OSF/1 Release 1.0
.	\" @(#)tbl.ms	1.2 4/22/88
.	\" TS - table start
.de TS
.br
.if !\\n(1T .RT
.ul 0
.ti \\n(.iu
.if t .sp .5
.if n .sp
.if \\$1H .TQ
.nr IX 1
..
.de TQ
.di TT
.nr IT 1
..
.	\" TH - end table header (begun with .TS H)
.de TH
.if \\n(.d>.5v .nr T. 0
.if \\n(.d>.5v .T# 0
.di
.nr TQ \\n(.i
.nr HT 1
.in 0
.mk #a
.mk #b
.mk #c
.mk #d
.mk #e
.mk #f
.TT
.in \\n(TQu
.mk #T
..
.	\" TE - table end
.de TE
.nr IX 0
.if \\n(IT>0 .if \\n(HT=0 \{\
.	di
.	nr UR \\n(.u
.	nf
.	TT
.	if \\n(UR>0 .fi
.\}
.nr IT 0
.nr HT 0
.sp \\n(DDu
.rm a+ b+ c+ d+ e+ f+ g+ h+ i+ j+ k+ l+ m+ n+
.rr a- b- c- d- e- f- g- h- i- j- k- l- m-
.rr a| b| c| d| e| f| g| h| i| j| k| l| m|
.rr 32 33 34 35 36 37 38 40 79 80 81 82
..
.	\" B1 - begin boxed text
.de B1
.br
.di BB
.nr BC 0
.if \\$1C .nr BC 1
.nr BE 1
..
.	\" B2 - end boxed text
.de B2
.br
.nr BI 1n
.if \\n(.$>0 .nr BI \\$1n
.di
.nr BE 0
.nr BW \\n(dl
.nr BH \\n(dn
.ne \\n(BHu+\\n(.Vu
.nr BQ \\n(.u
.nf
.if \\n(BC>0 .in +(\\n(.lu-\\n(BWu)/2u
.in +\\n(BIu
.BB
.in -\\n(BIu
.nr BW +2*\\n(BI
.sp -1
\l'\\n(BWu\(ul'\L'-\\n(BHu'\l'|0\(ul'\h'|0'\L'\\n(BHu'
.if \\n(BC>0 .in -(\\n(.lu-\\n(BWu)/2u
.if \\n(BQ .fi
.br
..
