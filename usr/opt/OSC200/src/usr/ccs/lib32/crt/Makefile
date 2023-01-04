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
#/*	@(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/04/15 16:28:20 $
# --------------------------------------------------- 
# | Copyright (c) 1986 MIPS Computer Systems, Inc.  | 
# | All Rights Reserved.                            | 
# --------------------------------------------------- 


ILIST	= crt0.o 
IDIR	= /usr/opt/alt/usr/lib/
IMODE	= 444

EXPLIB_TARGETS		= export_crt0.o
EXPDIR			=  /usr/opt/alt/usr/lib/
NOSTRIP =

SRCDIR	= ${SOURCEBASE}/usr/ccs/lib32/crt
CRT0_S  = crt0.s
OBJECTS = crt0.o mcrt0.o vcrt0.o vmcrt0.o pcrt0.o pmcrt0.o

ASFLAGS = ${_ASFLAGS_} -D_32BIT

LDFLAGS	= 
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/compiler.mk

all: $(OBJECTS)

crt0.o:	${CRT0_S}
	$(_CC_) -c ${_GENINC_} $(ASFLAGS) -DCRT0 ${CRT0_S} -o $@
	$(_LD_) $(LDFLAGS) -r crt0.o
	mv a.out $@

mcrt0.o: ${CRT0_S}
	$(_CC_) -c ${_GENINC_} $(ASFLAGS) -DMCRT0 ${CRT0_S} -o $@
	$(_LD_) $(LDFLAGS) -r $@
	mv a.out $@

vcrt0.o: ${CRT0_S}
	$(_CC_) ${_GENINC_} $(ASFLAGS) -c -DCRT0 -DSYSTEM_FIVE  ${CRT0_S} -o $@
	$(_LD_) $(LDFLAGS) -r $@
	mv a.out $@

vmcrt0.o: ${CRT0_S}
	$(_CC_) ${_GENINC_} $(ASFLAGS) -c -DMCRT0 -DSYSTEM_FIVE  ${CRT0_S} -o $@
	$(_LD_) $(LDFLAGS) -r $@
	mv a.out $@

pcrt0.o: ${CRT0_S}
	$(_CC_) ${_GENINC_} $(ASFLAGS) -c -DCRT0 -DPOSIX ${CRT0_S} -o $@
	$(_LD_) $(LDFLAGS) -r $@
	mv a.out $@

pmcrt0.o: ${CRT0_S}
	$(_CC_) ${_GENINC_} $(ASFLAGS) -c -DMCRT0 -DPOSIX ${CRT0_S} -o $@
	$(_LD_) $(LDFLAGS) -r $@
	mv a.out $@

copy:
	set -x;for i in ${OBJECTS}; do \
		$(LIBINSTALL) $${i} $(BASEDIR)/$${i}$(REL); done
	set -x;cd $(BASEDIR); \
	    for i in ${OBJECTS}; do \
		$(RM) -f $$i; $(LINK) $${i}$(REL) $$i; done

copybin:
	cd $(BASEDIR); \
	for i in ${OBJECTS}; do \
		$(RM) -f $${i}; $(LINK) $${i}$(REL) $$i; done

clobber:
	$(RM) $(OBJECTS)

check:
