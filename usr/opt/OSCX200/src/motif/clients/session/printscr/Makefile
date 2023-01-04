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
# CDA prsc Makefile
#
# $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/Makefile,v 1.1 90/01/01 00:00:00 devrcs Exp $

# History
#

TOP=../../..
CDEBUGFLAGS = -O 

INCLUDES = -I. \
	-I/usr/lib/DXM/lib/Xt -I/usr/lib/DXM/lib -I$(ISLROOT)/usr/include/X11 \
	-I$(ISLROOT)/usr/include -I$(BASEDIR)/clients/lib/print \
	-I../session \
	-I$(DESTDIR)/usr/include 

DEFINES = -DLANGUAGE_C 
CFLAGS = $(CDEBUGFLAGS) $(INCLUDES) $(DEFINES)
IMGLIB = $(ISLROOT)/usr/lib/libimg.a
IDSLIB = $(ISLROOT)/usr/lib/libids.a 
CHFLIB = $(ISLROOT)/usr/lib/libchf.a
DDIFLIB = -lddif

XLIB= -lMrm -lXm -lXt \
      $(DESTDIR)/usr/lib/libdps.a \
      $(DESTDIR)/usr/lib/libXext.a $(DESTDIR)/usr/lib/libX11.a

PWLIB = $(BASEDIR)/clients/lib/print/libpw.a 
LIBS = $(IDSLIB) $(DWTLIB) $(IMGLIB) $(CHFLIB) $(XLIB) $(DDIFLIB)   
CC = cc   
 
OBJECTS = creatisl.o creatoutput.o dither.o error.o getcolor.o getcoor.o \
getfile.o getqueue.o parse.o prdw_c.o swap.o valopt.o 
#OBJECTS = creatisl.o creatoutput.o dither.o error.o getcolor.o getcoor.o \
#getfile.o getqueue.o parse.o prdw_c.o printscreen.o swap.o valopt.o 

HEADERS = iprdw.h prdw.h desc.h ps.h

#
# PRSC component
#

all: prsc ${HEADERS}

prsc: $(OBJECTS) libprsc.a                                         

#prsc: $(OBJECTS) $(LIBS) libprsc.a                                         
#	$(CC) $(OBJECTS) $(LIBS) -lm -o prsc                        

libprsc.a:	$(OBJECTS)
	rm -rf libprsc.a
	ar cr libprsc.a $(OBJECTS)
	ranlib libprsc.a

install:  all
#	no longer need to install since binary is not used
#	install -c -m 0755 prsc ${DESTROOT}/usr/lib

clean:
	rm -f *.o ,* *~ *.bak *.BAK *.a core

clobber: clean
	rm -f tags
	${SCCS} clean

sccsget:
	${SCCS} get ${DATE} SCCS

sccsinfo:
	@${SCCS} info

tags:
	ctags *.[ch]

lint:
	${LINT} ${LINTFLAGS} *.c



