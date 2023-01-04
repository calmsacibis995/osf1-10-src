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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2.4.7 $ (DEC) $Date: 1992/09/29 15:10:03 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PMAX_SUBDIRS		= mips cmplrs

PMAX_DATAFILES		= a.out.h alignof.h alloca.h aouthdr.h \
			  disassembler.h elf_abi.h elf_mips.h exc.h \
			  excepthdr.h excpt.h filehdr.h float.h fp_class.h \
			  getpath.h imghdr.h linenum.h \
			  mach_o_header_md.h mach_o_types.h \
			  nan.h nlist.h obj.h obj_ext.h obj_list.h obj_type.h \
			  opnames.h ranlib.h reloc.h rld_interface.h \
			  scncomment.h scnhdr.h \
			  setjmp.h sex.h stamp.h stdarg.h storclass.h sym.h \
			  symconst.h syms.h values.h varargs.h

#
#  These are the header files needed for the compiler bootstrap on ULTRIX.
#
CMPLRS_DATAFILES	= a.out.h alignof.h \
			  alloca.h aouthdr.h ar.h disassembler.h elf_abi.h \
			  elf_mips.h exc.h excepthdr.h excpt.h filehdr.h \
			  getpath.h ldfcn.h linenum.h nan.h nlist.h \
			  obj.h obj_ext.h obj_list.h obj_type.h opnames.h \
			  reloc.h rld_interface.h \
			  scncomment.h scnhdr.h sex.h stamp.h \
			  stdarg.h storclass.h sym.h symconst.h syms.h \
			  values.h varargs.h fp.h

PMAX_OTHERS		= frame.h regdef.h asm.h

OBJSRC= section.c type.c list.c file.c obj.c procedure.c symbol.c search.c \
        line.c obj_elf.c obj_nlist.c obj_rewrite.c  objfcn.c 

OBJEXTH=obj_ext.h
OBJEXTSED=objext.sed

$(OBJEXTH): $(OBJEXTSED) $(OBJSRC) ${ALWAYS}
	${RM} ${_RMFLAGS_} $(OBJEXTH)
	@echo "#ifndef __OBJ_EXT_H" >> $(OBJEXTH)
	@echo "#define __OBJ_EXT_H" >> $(OBJEXTH)
# needed for Dynamic fields of obj
	@echo "#include <elf_abi.h>" >> $(OBJEXTH)
	sed -f $(OBJEXTSED) $(OBJSRC) >> $(OBJEXTH)
	@echo "#endif /* __OBJ_EXT_H */" >> $(OBJEXTH)
	@echo "$(OBJEXTH) rebuilt" 
 
frame.h regdef.h asm.h: ${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s machine/$@ $@

cmplrs_hdrs: ${CMPLRS_DATAFILES}
