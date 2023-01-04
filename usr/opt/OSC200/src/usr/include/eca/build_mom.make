#
# mom make file
#
MOM = [[mom_target]]
PROGRAMS	= ${MOM}
ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

INCFLAGS	= -I. -I${MAKETOP}../../export/${target_machine}/usr/include/eca
CFLAGS		= -DSNMP_OID -DSNMP
LIBS            = -lmoss -lmoi -lpthreads -lc_r -lc  -lbsd -lpthreads -lmld -lmach

MO_GENERIC_SRC = \
	access.c \
	create.c \
	delete.c \
	directive.c \
	get.c \
	get_instance.c \
	getnext.c \
	init.c \
	set.c \
	trap.c \
	utility_routines.c

MO_GENERIC_OBJ = $(MO_GENERIC_SRC:.c=.o)

/*-insert-code-list_of_mom_class_files-*/

MO_SRC = \
/*-insert-code-mom_class_src_list-*/
	$(MO_GENERIC_SRC)

MO_OBJ = \
/*-insert-code-mom_class_obj_list-*/
	$(MO_GENERIC_OBJ)

#
# Build targets
#

OFILES = ${MO_OBJ}

include ${MAKEFILEPATH}/rules.mk
