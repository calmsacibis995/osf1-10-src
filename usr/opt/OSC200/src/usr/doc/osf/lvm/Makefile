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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:04:22 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

SOELIM	= soelim
# double-quotes below are important!
SOFLAGS	= -p .:/project/osc/build/latest/src/usr/doc/osf/lvm

# MUST get eroff versions below.....
TBL	= /usr/local/bin/tbl
PIC	= /usr/local/bin/pic
PICFLAGS = -D
TROFF	= /usr/local/bin/troff
TFLAGS	= -Tps -meroff
EPS	= eps

LEM2PIC	= /usr/local/pub/lem2pic

DOC	= lvm
TFILE	= $(DOC).t
TFILES	= overview.t \
		terminology.t \
		limits.t \
		physvol.t \
		vgda.t \
		prog_interface.t \
		operations.t \
		implementation.t \
		mirror_recovery.t \
		phys_driver_intfc.t \
		fontmacros.m \
		picmacros.m \
		copyright.m
NSOFILE	= $(DOC).nso
PICFILE	= $(DOC).npic
TBLFILE	= $(DOC).ntbl
PSFILE	= $(DOC).ps
OUTFILE	= $(DOC).out

PICFILES = LV_entry.pic PP_map_entry.pic PV_header.pic datastruct.pic \
	VGDA.pic VG_header.pic VG_trailer.pic ioflow.pic exemodel.pic \
	lvm_layering.pic physvol.pic VGSA.pic MWC.pic LVM_rec.pic \
	BB_entry.pic

OTHERS	= $(DOC)
GARBAGE = $(NSOFILE) $(TBLFILE) $(PICFILES)

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk

NROFF	= /usr/local/bin/nroff
NFLAGS	=

$(DOC): $(PSFILE) $(OUTFILE)

$(PSFILE) $(OUTFILE): $(PICFILES) $(TFILES)

.PRECIOUS: $(TFILE)

#
# foo.t [soelim]=> foo.nso [pic]=> foo.npic [tbl]=> foo.ntbl
#				[troff]=> foo.eps [eps]=> foo.ps
# foo.lem [lem2pic]=> foo.pic
#
.SUFFIXES: .ps .eps .out .ntbl .npic .nso .t .pic .lem

# single-function transforms
.lem.pic:
	@$(RM) -f $@
	$(LEM2PIC) $<

.t.nso:
	@$(RM) -f $*..
	$(SOELIM) $(SOFLAGS) $< > $*..
	$(MV) -f $*.. $@

.nso.npic:
	@$(RM) -f $@
	$(PIC) $(PICFLAGS) $< > $*..
	$(MV) -f $*.. $@

.npic.ntbl:
	@$(RM) -f $*..
	$(TBL) $< > $*..
	$(MV) -f $*.. $@

.ntbl.eps:
	@$(RM) -f $*..
	$(TROFF) $(TFLAGS) $< > $*..
	$(MV) -f $*.. $@

.eps.ps:
	@$(RM) -f $*..
	$(EPS) $< > $*..
	$(MV) -f $*.. $@

# 2-function transforms
.t.npic:
	@$(RM) -f $*..
	$(SOELIM) $(SOFLAGS) $< | $(PIC) $(PICFLAGS) > $*..
	$(MV) -f $*.. $@

.nso.ntbl:
	@$(RM) -f $*..
	$(PIC) $(PICFLAGS) $< | $(TBL) > $*..
	$(MV) -f $*.. $@

.npic.eps:
	@$(RM) -f $*..
	$(TBL) $< | $(TROFF) $(TFLAGS) - > $*..
	$(MV) -f $*.. $@

.ntbl.ps:
	@$(RM) -f $*..
	$(TROFF) $(TFLAGS) $< | $(EPS) > $*..
	$(MV) -f $*.. $@

# 3-function transforms
.t.ntbl:
	@$(RM) -f $*..
	$(SOELIM) $(SOFLAGS) $< | $(PIC) $(PICFLAGS) | $(TBL) > $*..
	$(MV) -f $*.. $@

.nso.eps:
	@$(RM) -f $*..
	$(PIC) $(PICFLAGS) $< | $(TBL) | $(TROFF) $(TFLAGS) - > $*..
	$(MV) -f $*.. $@

.npic.ps:
	@$(RM) -f $*..
	$(TBL) $< | $(TROFF) $(TFLAGS) - | $(EPS) > $*..
	$(MV) -f $*.. $@

# 4-function transforms
.t.eps:
	@$(RM) -f $*..
	$(SOELIM) $(SOFLAGS) $< | $(PIC) $(PICFLAGS) | $(TBL) | $(TROFF) $(TFLAGS) - > $*..
	$(MV) -f $*.. $@

.nso.ps:
	@$(RM) -f $*..
	$(PIC) $(PICFLAGS) $< | $(TBL) | $(TROFF) $(TFLAGS) - | $(EPS) > $*..
	$(MV) -f $*.. $@

# 5-function transform
.t.ps:
	@$(RM) -f $*..
	$(SOELIM) $(SOFLAGS) $< | $(PIC) $(PICFLAGS) | $(TBL) | $(TROFF) $(TFLAGS) - | $(EPS) > $*..
	$(MV) -f $*.. $@

.t.out:
	@$(RM) -f $*..
	$(SOELIM) $(SOFLAGS) $< | $(PIC) | $(TBL) | $(NROFF) $(NFLAGS) - | col - > $*..
	$(MV) -f $*.. $@
