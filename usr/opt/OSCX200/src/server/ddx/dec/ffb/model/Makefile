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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:20:49 $
#
#
#
# HISTORY
#
  
CC       = cc

#for speed
CDEBUGFLAGS = -O 
CFLAGS   =  -I. -Ibeh  $(CDEBUGFLAGS)  

TABLES = tables.o

###
### smodel
###
### this builds a model for testing the bits we send
### to the software people.
###
smodel:  smodel.o  batch.o $(TABLES) rsmBusInterface.o beh.o BehRAM.o zbeh.o wrapper.o parts.o MemBeh.o
	rm -f smodel
	cc -o smodel smodel.o  batch.o $(TABLES) rsmBusInterface.o BehRAM.o  beh.o zbeh.o wrapper.o parts.o MemBeh.o

tables.o: tables.c io_struct.h
	$(CC) -c $(CFLAGS) -O0 tables.c

libSim.a: $(TABLES) rsmBusInterface.o BehRAM.o beh.o zbeh.o wrapper.o parts.o macroCmds.o parse.o MemBeh.o
	rm -f libSim.a
	touch -f libSim.a
# we don't really need the .a - not used by server ld
#	ar rc libSim.a $(TABLES) rsmBusInterface.o BehRAM.o beh.o zbeh.o wrapper.o parts.o macroCmds.o parse.o MemBeh.o
#	ranlib libSim.a
#	tar cf - libSim.a | compress >libSim.a.tar.Z

#
# Behavioral models
#
beh.o : beh.c dithermatrix.h types.h vars.h

MemBeh.o : MemBeh.c types.h vars.h
#
# Binaries
#
rsmBusInterface.o: BusInterface.c defs.h types.h vars.h vram.h io_struct.h
	$(CC) -DRSM -o rsmBusInterface.o -c $(CFLAGS) BusInterface.c

wrapper.o: wrapper.c

batch.o: batch.c defs.h types.h vars.h vram.h

macroCmds.o: macroCmds.c defs.h types.h vars.h vram.h

parse.o:  parse.c defs.h types.h vars.h
