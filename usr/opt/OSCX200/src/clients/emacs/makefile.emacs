# HISTORY
# $Log: makefile.emacs,v $
# Revision 1.1.2.1  1992/11/12  13:29:37  devrcs
# Fix revision
#
# Revision 1.1  90/01/01  00:00:00  devrcs
# Initial load into Alpha pool
# 
# Revision 1.1.2.2  92/02/11  12:42:26  Scott_Sewall
# 	Copied from Makefile. Converted to ODE-II environment. Doesn't use ODE make.
# 	[92/02/11  12:21:57  Scott_Sewall]
# 
# $EndLog$
#
# BuildSystemHeader added automatically
# $Header: /usr/sde/osf1/rcs/x11/src/clients/emacs/makefile.emacs,v 1.1.2.1 1992/11/12 13:29:37 devrcs Exp $
# make all	to compile and build Emacs
# make install	to install it
# make install.sysv  to install on system V.
# make install.xenix  to install on Xenix
# make tags	to update tags tables
#
# make distclean	to delete everything that wasn't in the distribution
#	This is a very dangerous thing to do!
# make clean
#       This is a little less dangerous.

SHELL = /bin/sh

# Where to install things
# Note that on system V you must change MANDIR to /use/local/man/man1.
LIBDIR= $(DESTDIR)/usr/lib/emacs
BINDIR= $(DESTDIR)/usr/bin
MANDIR= $(DESTDIR)/usr/man/man1
DOCLIBDIR= $(DESTDIR)/usr/share/doclib
DOCDIR= ${DOCLIBDIR}/fsf

# Flags passed down to subdirectory makefiles.
MFLAGS=
MAKE=make

# Subdirectories to make recursively.  `lisp' is not included
# because the compiled lisp files are part of the distribution
# and you cannot remake them without installing Emacs first.
SUBDIR= etc src

# Subdirectories to install
COPYDIR= etc info lisp src oldXMenu shortnames

# Subdirectories to clean
CLEANDIR= ${COPYDIR} lisp/term

# etc executable clean up
EXECUTABLES=etags ctags emacsclient
#test-distrib etags ctags wakeup make-docfile \
   digest-doc sorted-doc movemail cvtmail fakemail yow env \
   server emacsclient
 
all:	src/paths.h ${SUBDIR}

src/paths.h: makefile.emacs src/paths.h-dist
	/bin/sed 's;/usr/lbin/emacs;${LIBDIR};g' < src/paths.h-dist > src/paths.h

src:	etc

.RECURSIVE: ${SUBDIR}

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS} all

install: all mkdir lockdir
	rm -rf ${LIBDIR}
	-mkdir ${LIBDIR}
	-mkdir ${LIBDIR}/etc
	-mkdir ${LIBDIR}/lisp
	-mkdir ${LIBDIR}/src
	-mkdir ${LIBDIR}/oldXMenu
	-mkdir ${LIBDIR}/shortnames
	-mkdir ${DOCLIBDIR}
	-mkdir ${DOCDIR}
	-if [ `/bin/pwd` != `(cd ${LIBDIR}; /bin/pwd)` ] ; then \
		tar cf - ${COPYDIR} | (cd ${LIBDIR}; umask 0; tar xBf - ) ;\
		for i in ${CLEANDIR}; do \
			(rm -rf ${LIBDIR}/$$i/RCS; \
			 rm -f ${LIBDIR}/$$i/\#*; \
			 rm -f ${LIBDIR}/$$i/*~; \
			 rm -f ${LIBDIR}/$$i/*.o); \
		done \
	else true; \
	fi
#	-rm -f ${LIBDIR}/etc/${EXECUTABLES}
#	-mv ${LIBDIR}/src/kit_ymakefile ${LIBDIR}/src/ymakefile
#	-mv ${LIBDIR}/src/kit_Makefile ${LIBDIR}/src/Makefile
#	-rm -f ${LIBDIR}/src/xmakefile ${LIBDIR}/src/kit_*
	-rm -f ${LIBDIR}/oldXMenu/libXMenu11.a
	install -c -s etc/emacsclient ${BINDIR}/emacsclient
	install -c -s etc/etags ${BINDIR}/emacsetags
	install -c -s etc/ctags ${BINDIR}/emacsctags
#	install -c -s -m 1755 src/xemacs ${BINDIR}/xemacs
#	install -c -s -m 1755 src/bemacs ${LIBDIR}/etc/bemacs
	install -c -m 1755 src/temacs ${LIBDIR}/etc/temacs
	install -c -m 1755 etc/DOC ${LIBDIR}/etc/DOC
	-install -c -m 444 etc/emacs.1 ${MANDIR}/emacs.1
	-install -c -m 444 doc/elisp.ps ${DOCDIR}/elisp.ps
	-install -c -m 444 doc/emacs.ps ${DOCDIR}/emacs.ps
#	-rm -f ${BINDIR}/emacs
#	mv ${BINDIR}/xemacs ${BINDIR}/emacs

install.sysv: all mkdir lockdir
	-if [ `/bin/pwd` != `(cd ${LIBDIR}; /bin/pwd)` ] ; then \
		find ${COPYDIR} -print | cpio -pdum ${LIBDIR} ;\
		for i in ${CLEANDIR}; do \
			(rm -rf ${LIBDIR}/$$i/RCS; \
			 rm -f ${LIBDIR}/$$i/\#*; \
			 rm -f ${LIBDIR}/$$i/*~); \
		done \
	else true; \
	fi
	-cpset etc/emacsclient ${BINDIR}/emacsclient 755 bin bin
	-cpset etc/etags ${BINDIR}/etags 755 bin bin
	-cpset etc/ctags ${BINDIR}/ctags 755 bin bin
	-cpset etc/emacs.1 ${MANDIR}/emacs.1 444 bin bin
	-/bin/rm -f ${BINDIR}/emacs
	-cpset src/xemacs ${BINDIR}/emacs 1755 bin bin
  
install.xenix: all mkdir lockdir
	if [ `pwd` != `(cd ${LIBDIR}; pwd)` ] ; then \
		tar cf - ${COPYDIR} | (cd ${LIBDIR}; umask 0; tar xpf - ) ;\
		for i in ${CLEANDIR}; do \
			(rm -rf ${LIBDIR}/$$i/RCS; \
			 rm -f ${LIBDIR}/$$i/\#*; \
			 rm -f ${LIBDIR}/$$i/*~); \
		done \
	else true; \
	fi
	cp etc/etags etc/ctags etc/emacsclient ${BINDIR}
	chmod 755 ${BINDIR}/etags ${BINDIR}/ctags ${BINDIR}/emacsclient
	cp etc/emacs.1 ${MANDIR}/emacs.1
	chmod 444 ${MANDIR}/emacs.1
	-mv -f ${BINDIR}/emacs ${BINDIR}/emacs.old
	cp src/xemacs ${BINDIR}/emacs
	chmod 1755 ${BINDIR}/emacs
	-rm -f ${BINDIR}/emacs.old

mkdir: FRC
	-mkdir ${LIBDIR}
	-chmod 777 ${LIBDIR}

distclean:
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} distclean); done

clean:
	cd src; ${MAKE} clean
	if [ `pwd` != `(cd ${LIBDIR}; pwd)` ] ; then \
		cd etc; ${MAKE} clean; \
	else true; \
	fi

lockdir:
	-mkdir ${LIBDIR}/lock
	-chmod 777 ${LIBDIR}/lock

FRC:

tags:	etc
	cd src; ../etc/etags *.[ch] ../lisp/*.el ../lisp/term/*.el
