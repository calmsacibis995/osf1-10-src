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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:35:59 $  
#
Options	= -DSYS5DIR -D_BSD -DISI -DMHRC -DWHATNOW -DMHE -DBSD42 -DBERK \
          -DBSD43 -DLOCKF -DNETWORK -DBIND -DRPATHS -DDPOP -DRPOP -DUK \
          -DFOLDPROT='"0700"' -DMSGPROT='"0600"' -DTYPESIG='void' \
          -DPOPSERVICE='"pop"' -DSPRINTF='int' -DSENDMTS -DSMTP -DPOP -DX400

CFLAGS = ${CFLAGS} ${Options}
LIBRARIES = libmh.a
OFILES =	add.o adios.o admonish.o addrsbr.o advertise.o advise.o \
		ambigsw.o atooi.o brkstring.o closefds.o concat.o copy.o \
		copyip.o cpydata.o cpydgst.o discard.o done.o fdcompare.o \
		gans.o getans.o getanswer.o getcpy.o help.o fmtcompile.o \
		formataddr.o formatsbr.o m_atoi.o m_backup.o m_convert.o \
		m_delete.o m_draft.o m_find.o m_fmsg.o m_foil.o m_getdefs.o \
		m_getfld.o m_getfolder.o m_gmprot.o m_gmsg.o m_maildir.o \
		m_name.o m_readefs.o m_remsg.o m_replace.o m_scratch.o \
		m_seq.o m_seqbits.o m_seqflag.o m_seqnew.o m_setcur.o \
		m_setseq.o m_setvis.o m_sync.o m_tmpfil.o m_update.o \
		m_whatnow.o makedir.o path.o peekc.o pidwait.o pidstatus.o \
		printsw.o push.o putenv.o pwd.o refile.o remdir.o r1bindex.o \
		showfile.o smatch.o sprintb.o ssequal.o strindex.o trimcpy.o \
		uleq.o uprf.o vfgets.o

version ::	$(OFILES)
		@/bin/sh version.sh 6.7

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
