# Makefile generated by imake - do not edit!
# $XConsortium: imake.c,v 1.65 91/07/25 17:50:17 rws Exp $

# -------------------------------------------------------------------------
# Makefile generated from "Imake.tmpl" and </tmp/IIf.aahgua>
# $XConsortium: Imake.tmpl,v 1.139 91/09/16 08:52:48 rws Exp $
#
# From the Alpha Imakefile
# Platform-specific parameters may be set in the appropriate <vendor>.cf
# configuration files.  Site-specific parameters should be set in the file
# site.def.  Full rebuilds are recommended if any parameters are changed.
#
# If your C preprocessor does not define any unique symbols, you will need
# to set BOOTSTRAPCFLAGS when rebuilding imake (usually when doing
# "make World" the first time).
#

# -------------------------------------------------------------------------
# site-specific configuration parameters that need to come before
# the platform-specific parameters - edit site.def to change

# site:  $XConsortium: site.def,v 1.2 91/07/30 20:26:44 rws Exp $

# -------------------------------------------------------------------------
# platform-specific configuration parameters - edit osf1.cf to change

# platform: osf1.cf

# Operating system:  1/1 2.0

     CROSSBASE = $(TOP)/../tools/alpha_osf1/cc

   	SHAREDSWITCH =

#  Using 1 shared library rules

# -------------------------------------------------------------------------
# site-specific configuration parameters that go after
# the platform-specific parameters - edit site.def to change

# site:  $XConsortium: site.def,v 1.2 91/07/30 20:26:44 rws Exp $

            SHELL = /bin/sh

              TOP = ../../../../.
      CURRENT_DIR = ./motif/clients/xif/sysadmin

               AR = ar cqls
            LOCAR = ar cqls
  BOOTSTRAPCFLAGS =
               CC = cc $(SHAREDSWITCH)
            LOCCC = COMP_HOST_ROOT=/ COMP_TARGET_ROOT=/ /usr/bin/cc -non_shared
        LOCSHARCC = COMP_HOST_ROOT=/ COMP_TARGET_ROOT=/ /usr/bin/cc
               AS = as

         COMPRESS = compress
              CPP = /lib/cpp $(STD_CPP_DEFINES)
    PREPROCESSCMD = cc $(SHAREDSWITCH) -E $(STD_CPP_DEFINES)
          INSTALL = ksh $(TOP)/util/scripts/install.sh
               LD = ld
             LINT = lint
      LINTLIBFLAG = -o
         LINTOPTS = -ax
               LN = ln -s
             MAKE = make
               MV = mv
               CP = cp

               RM = rm -f
            TROFF = psroff
         MSMACROS = -ms
              TBL = tbl
              EQN = eqn
     STD_INCLUDES = -I$(CROSSBASE)/usr/include
  LOCSTD_INCLUDES =
  STD_CPP_DEFINES = -DDEC -DR5_XLIB -DSMT
      STD_DEFINES = -DDEC -DR5_XLIB -DSMT
 EXTRA_LOAD_FLAGS =
  EXTRA_LIBRARIES =
 LOCEXTRA_LIBRARIES =
             TAGS = ctags

    SHAREDCODEDEF = -DSHAREDCODE
         SHLIBDEF =

    PROTO_DEFINES = -DFUNCPROTO=3

     INSTPGMFLAGS = -s

     INSTBINFLAGS = -m 0755
     INSTUIDFLAGS = -m 4755
     INSTLIBFLAGS = -m 0644
     INSTINCFLAGS = -m 0444
     INSTMANFLAGS = -m 0444
     INSTDATFLAGS = -m 0444
    INSTKMEMFLAGS = -m 4755

     TOP_INCLUDES = -I$(TOP) -I$(TOP)/X11

      CDEBUGFLAGS = -O2 -Olimit 2000
        CCOPTIONS = 	-I -L -L$(CROSSBASE)/usr/shlib -L$(CROSSBASE)/usr/ccs/lib
     LOCCCOPTIONS =

      ALLINCLUDES = $(INCLUDES) $(EXTRA_INCLUDES) $(TOP_INCLUDES) $(STD_INCLUDES)
   LOCALLINCLUDES = $(INCLUDES) $(EXTRA_INCLUDES) $(TOP_INCLUDES) $(LOCSTD_INCLUDES)
       ALLDEFINES = $(ALLINCLUDES) $(STD_DEFINES) $(EXTRA_DEFINES) $(PROTO_DEFINES) $(DEFINES)
    LOCALLDEFINES = $(LOCALLINCLUDES) $(STD_DEFINES) $(EXTRA_DEFINES) $(PROTO_DEFINES) $(DEFINES)
           CFLAGS = $(CDEBUGFLAGS) $(CCOPTIONS) $(ALLDEFINES)
        LOCCFLAGS = $(CDEBUGFLAGS) $(LOCCCOPTIONS) $(LOCALLDEFINES)
        LINTFLAGS = $(LINTOPTS) -DLINT $(ALLDEFINES)

           LDLIBS = $(SYS_LIBRARIES) $(EXTRA_LIBRARIES)

        LOCLDLIBS = $(SYS_LIBRARIES) $(LOCEXTRA_LIBRARIES)

        LDOPTIONS = $(CDEBUGFLAGS) $(CCOPTIONS) $(LOCAL_LDFLAGS)

     LOCLDOPTIONS = $(CDEBUGFLAGS) $(LOCCCOPTIONS) $(LOCAL_LDFLAGS)

   LDCOMBINEFLAGS = -X -r
      DEPENDFLAGS = -D__alpha  -D__osf__ -I -I$(CROSSBASE)/usr/include

        MACROFILE = osf1.cf
           RM_CMD = $(RM) *.CKP *.ln *.BAK *.bak *.o core errs ,* *~ *.a .emacs_* tags TAGS make.log MakeOut

    IMAKE_DEFINES =

         IRULESRC = $(CONFIGSRC)
        IMAKE_CMD = $(NEWTOP)$(IMAKE) -I$(NEWTOP)$(IRULESRC) $(IMAKE_DEFINES)

     ICONFIGFILES = $(IRULESRC)/Imake.tmpl $(IRULESRC)/Imake.rules \
			$(IRULESRC)/Project.tmpl $(IRULESRC)/site.def \
			$(IRULESRC)/$(MACROFILE) $(EXTRA_ICONFIGFILES)

# -------------------------------------------------------------------------
# X Window System Build Parameters
# $XConsortium: Project.tmpl,v 1.142 92/01/25 16:44:02 rws Exp $

# -------------------------------------------------------------------------
# X Window System make variables; this need to be coordinated with rules

          PATHSEP = /
        USRLIBDIR = /usr/lib
           BINDIR = /usr/bin/X11
          INCROOT = /usr/include
     BUILDINCROOT = $(TOP)
      BUILDINCDIR = $(BUILDINCROOT)/X11
      BUILDINCTOP = ..
           INCDIR = $(INCROOT)/X11
           ADMDIR = /usr/adm
           LIBDIR = $(USRLIBDIR)/X11
        CONFIGDIR = $(LIBDIR)/config
       LINTLIBDIR = $(USRLIBDIR)/lint

          FONTDIR = $(LIBDIR)/fonts
         XINITDIR = $(LIBDIR)/xinit
           XDMDIR = $(LIBDIR)/xdm
           TWMDIR = $(LIBDIR)/twm
          MANPATH = /usr/man
    MANSOURCEPATH = $(MANPATH)/man
        MANSUFFIX = n
     LIBMANSUFFIX = 3
           MANDIR = $(MANSOURCEPATH)$(MANSUFFIX)
        LIBMANDIR = $(MANSOURCEPATH)$(LIBMANSUFFIX)
           NLSDIR = $(LIBDIR)/nls
        PEXAPIDIR = $(LIBDIR)/PEX
      XAPPLOADDIR = $(LIBDIR)/app-defaults
       FONTCFLAGS = -t

     INSTAPPFLAGS = $(INSTDATFLAGS)

            IMAKE = $(IMAKESRC)/imake.local

           DEPEND = $(DEPENDSRC)/makedepend
              RGB = $(RGBSRC)/rgb

            FONTC = $(BDFTOPCFSRC)/bdftopcf

        MKFONTDIR = $(MKFONTDIRSRC)/mkfontdir
        MKDIRHIER = /bin/sh $(SCRIPTSRC)/mkdirhier.sh

        CONFIGSRC = $(TOP)/config
       DOCUTILSRC = $(TOP)/doc/util
        CLIENTSRC = $(TOP)/clients
          DEMOSRC = $(TOP)/demos
           LIBSRC = $(TOP)/lib
          FONTSRC = $(TOP)/fonts
       INCLUDESRC = $(TOP)/X11
        SERVERSRC = $(TOP)/server
          UTILSRC = $(TOP)/util
        SCRIPTSRC = $(UTILSRC)/scripts
       EXAMPLESRC = $(TOP)/examples
       CONTRIBSRC = $(TOP)/../contrib
           DOCSRC = $(TOP)/doc
           RGBSRC = $(TOP)/rgb
        DEPENDSRC = $(TOP)/util/makedepend_local
         IMAKESRC = $(CONFIGSRC)
         XAUTHSRC = $(LIBSRC)/Xau
          XLIBSRC = $(LIBSRC)/X
           XMUSRC = $(LIBSRC)/Xmu
       TOOLKITSRC = $(LIBSRC)/Xt
       AWIDGETSRC = $(LIBSRC)/Xaw
       OLDXLIBSRC = $(LIBSRC)/oldX
      XDMCPLIBSRC = $(LIBSRC)/Xdmcp

      BDFTOPCFSRC = $(FONTSRC)/clients/bdftopcf

     MKFONTDIRSRC = $(FONTSRC)/clients/mkfontdir
         FSLIBSRC = $(FONTSRC)/lib/fs
    FONTSERVERSRC = $(FONTSRC)/server
     EXTENSIONSRC = $(TOP)/extensions
         XILIBSRC = $(EXTENSIONSRC)/lib/xinput
      XTESTLIBSRC = $(EXTENSIONSRC)/lib/xtest
      XTRAPLIBSRC = $(EXTENSIONSRC)/lib/xtrap
      PHIGSLIBSRC = $(EXTENSIONSRC)/lib/PEX

      USRSHLIBDIR = /usr/shlib

       SHLIBFLAGS = -check_registry $(CROSSBASE)/usr/shlib/so_locations 	-L$(CROSSBASE)/usr/shlib -L$(CROSSBASE)/usr/ccs/lib

  DEPEXTENSIONLIB = $(EXTENSIONSRC)/lib/libXext.so
     EXTENSIONLIB = $(DEPEXTENSIONLIB)

          DEPXLIB = $(DEPEXTENSIONLIB) $(XLIBSRC)/libX11.so
             XLIB = $(DEPXLIB)

        DEPXMULIB = $(XMUSRC)/libXmu.so
           XMULIB = $(DEPXMULIB)

       DEPOLDXLIB = $(OLDXLIBSRC)/liboldX.so
          OLDXLIB = $(DEPOLDXLIB)

      DEPXTOOLLIB = $(TOOLKITSRC)/libXt.so
         XTOOLLIB = $(DEPXTOOLLIB)

        DEPXAWLIB = $(AWIDGETSRC)/libXaw.so
           XAWLIB = $(DEPXAWLIB)

        DEPXILIB = $(XILIBSRC)/libXi.a
           XILIB = $(DEPXILIB)

          DNETLIB = -ldnet_stub

        SOXLIBREV = SharedXlibRev
          SOXTREV = SharedXtRev
         SOXAWREV = SharedXawRev
        SOOLDXREV = SharedOldXRev
         SOXMUREV = SharedXmuRev
        SOXEXTREV = SharedXextRev
      SOXINPUTREV = SharedXinputRev
       SOXTESTREV = SharedXTestRev
       SOXTRAPREV = SharedXTrapRev

      DEPXAUTHLIB = $(XAUTHSRC)/libXau.a
         XAUTHLIB = 			 $(DEPXAUTHLIB)
      DEPXDMCPLIB = $(XDMCPLIBSRC)/libXdmcp.a
         XDMCPLIB = 			 $(DEPXDMCPLIB)

     DEPXTESTLIB = $(XTESTLIBSRC)/libXtst.a
        XTESTLIB = 			 $(DEPXTESTLIB)

     DEPXTRAPLIB = $(XTRAPLIBSRC)/libXTrap.a
        XTRAPLIB = 			 $(DEPXTRAPLIB)

        DEPPHIGSLIB = $(PHIGSLIBSRC)/libphigs.a
           PHIGSLIB = 			 $(DEPPHIGSLIB)

       DEPXBSDLIB = $(LIBSRC)/Berk/libXbsd.a
          XBSDLIB = 			 $(DEPXBSDLIB)

 LINTEXTENSIONLIB = $(EXTENSIONSRC)/lib/llib-lXext.ln
         LINTXLIB = $(XLIBSRC)/llib-lX11.ln
          LINTXMU = $(XMUSRC)/llib-lXmu.ln
        LINTXTOOL = $(TOOLKITSRC)/llib-lXt.ln
          LINTXAW = $(AWIDGETSRC)/llib-lXaw.ln
           LINTXI = $(XILIBSRC)/llib-lXi.ln
        LINTPHIGS = $(PHIGSLIBSRC)/llib-lphigs.ln

       DEPXBSDLIB = $(LIBSRC)/Berk/libXbsd.a
          XBSDLIB = 			 $(DEPXBSDLIB)

          DEPLIBS = $(DEPXAWLIB) $(DEPXMULIB) $(DEPXTOOLLIB) $(DEPXLIB)

         DEPLIBS1 = $(DEPLIBS)
         DEPLIBS2 = $(DEPLIBS)
         DEPLIBS3 = $(DEPLIBS)

       UILFLAGS = -I
        MLIBSRC = $(TOP)/motif/lib
          XMSRC = $(MLIBSRC)/Xm
         LINTXM = $(XMSRC)/llib-lXm.ln
      USRINCDIR = $(INCDIR)
        TESTSRC = $(TOP)/tests
    MTOOLKITSRC = $(LIBSRC)/Xt
     MWIDGETSRC = $(MLIBSRC)/Xm
   MRESOURCESRC = $(MLIBSRC)/Mrm
     MCLIENTSRC = $(TOP)/motif/clients
         UILSRC = $(MCLIENTSRC)/uil
    MINCLUDESRC = $(INCLUDESRC)

            UIL = $(TOP)/motif/clients/uil/uil.static

     INSTALLLIB = $(DESTDIR)/usr/lib
   INSTALLSHLIB = $(DESTDIR)/usr/shlib

        DEPXMLIB = $(XMSRC)/libXm.so
           XMLIB = $(XMSRC)/libXm.so

DEPMRESOURCELIB = $(MRESOURCESRC)/libMrm.so
   MRESOURCELIB = $(MRESOURCESRC)/libMrm.so

    UILINCLUDES = -I$(BUILDINCDIR)/uil

         UILLIB = $(UILSRC)/libUil.a
LINTXMWIDGETLIB = $(MWIDGETSRC)/llib-l/Xm.ln
     LINTXMTOOL = $(MTOOLKITSRC)/llib-lXt.ln
     LINTUILLIB = $(UILSRC)/llib-lUil.ln

# -------------------------------------------------------------------------
# DEC-Specific System Build Parameters

          HELPDIR = $(LIBDIR)/help
           UIDDIR = $(LIBDIR)/uid
           CDADIR = $(USRLIBDIR)/cda

           DXMSRC = $(MLIBSRC)/DXm
           DPSSRC = $(EXTENSIONSRC)/DPS/lib/dps
         DPSTKSRC = $(EXTENSIONSRC)/DPS/lib/dpstk
         PSRESSRC = $(EXTENSIONSRC)/DPS/lib/psres
           XIESRC = $(EXTENSIONSRC)/lib/xie
           IDSSRC = $(MLIBSRC)/ids
           BKRSRC = $(MLIBSRC)/bkr
        DWI18NSRC = $(MLIBSRC)/dwi18n
           LWKSRC = $(MLIBSRC)/lwk
           DVRSRC = $(MLIBSRC)/dvr

       DEPDXMLIB = $(DXMSRC)/libDXm.so
          DXMLIB = $(DXMSRC)/libDXm.so

       MDXMLIB = $(DXMLIB)

        DEPDPSLIB = $(DPSSRC)/libdps.so
           DPSLIB = $(DEPDPSLIB)

      DEPDPSTKLIB = $(DPSTKSRC)/libdpstk.so
         DPSTKLIB = $(DEPDPSTKLIB)

        DEPPSRESLIB = $(PSRESSRC)/libpsres.so
           PSRESLIB = $(DEPPSRESLIB)

        DEPXIELIB = $(XIESRC)/libXie.a
           XIELIB = 			 $(DEPXIELIB)

        DEPIDSLIB = $(IDSSRC)/libids.so
           IDSLIB = $(DEPIDSLIB)

        DEPBKRLIB = $(BKRSRC)/libbkr.so
           BKRLIB = $(DEPBKRLIB)

        DEPDWI18NLIB = $(DWI18NSRC)/libdwi18n.a
           DWI18NLIB = 			 $(DEPDWI18NLIB)

        DEPLWKLIB = $(LWKSRC)/liblwkdxm.so
           LWKLIB = $(DEPLWKLIB)

        DEPDVRLIB = $(DVRSRC)/libdvr.so
           DVRLIB = $(DEPDVRLIB)

        DEPIMGLIB =
        DEPIPSLIB =
        DEPCHFLIB =
        DEPCDALIB =
        DEPDVSLIB =

    DEPIDS_NOXLIB =

           IMGLIB = -limg
           IPSLIB = -lips
           CHFLIB = -lchf
           CDALIB = -lcda
       IDS_NOXLIB = -lids_nox
           DVSLIB = -ldvs

     DEBUGUSRLIBDIR=/usr/debugshlib

# -------------------------------------------------------------------------
# Imake rules for building libraries, programs, scripts, and data files
# rules:  $XConsortium: Imake.rules,v 1.123 91/09/16 20:12:16 rws Exp $

# -------------------------------------------------------------------------
# DEC-Specific System Build Rules
# $Id: Dec.rules,v 1.1.2.8 92/12/08 19:29:22 Leela_Obilichetti Exp $

# -------------------------------------------------------------------------
# start of Imakefile

#
# Imakefile for secclients/xif/sysadmin
#

 	ROLESDIR = $(TOP)/motif/clients/xif
    XSYSADMINDIR = $(ROLESDIR)/sysadmin
 	XISSODIR = $(ROLESDIR)/isso
 	AUDITDIR = $(XISSODIR)/audit
     ACCOUNTSDIR = $(XISSODIR)/accounts
       DEVICEDIR = $(XISSODIR)/devices
       COMMONDIR = $(ROLESDIR)/common
 	TOOLSDIR = $(ROLESDIR)/tools
 	    LIBS = -laccounts -laudit -lcommon

 	 MSGTOOL = $(TOOLSDIR)/msgprep

 	SYSLIBS = $(DXMLIB) $(BKRLIB) $(MRESOURCELIB) $(XMLIB) $(XTOOLLIB) $(XLIB) -lsecurity -laud

    STD_DEFINES = ${SEC_DEFS} -DOSF

    CDEBUGFLAGS =
       INCLUDES = -I. -I$(COMMONDIR)
    UILINCLUDES = -I. -I$(COMMONDIR) -I$(ACCOUNTSDIR)

 	 LDLIBS = -L$(COMMONDIR) -L$(ACCOUNTSDIR) -L$(AUDITDIR)

 	   ALL = XSysAdmin XSysAdmin.uid
 	  SRCS = XSysAdmin.c adminLocalInfo.c CreateAcc.c CreateGrp.c \
                 RetireAcc.c
 	  OBJS = XSysAdmin.o adminLocalInfo.o CreateAcc.o CreateGrp.o \
                 RetireAcc.o

     EXEC_DEST = /usr/tcb/bin

 	 UILS1 = XSysAdmin.uil XSysAdmin.src.uil \
		 $(COMMONDIR)/literals_main.uil \
		 $(COMMONDIR)/literals_error.uil \
		 $(COMMONDIR)/errordb.uil \
		 $(COMMONDIR)/main_procedures.uil \
		 $(COMMONDIR)/SelList.uil \
		 $(COMMONDIR)/TextList.uil \
		 $(COMMONDIR)/hyperhelp.uil \
		 MainMenu.uil \
		 $(ACCOUNTSDIR)/AccValues.uil \
		 $(ACCOUNTSDIR)/AccAudit.uil \
		 $(ACCOUNTSDIR)/AccCommon.uil \
		 $(ACCOUNTSDIR)/Accounts.uil \
		 $(ACCOUNTSDIR)/AccTmplts.uil \
		 $(ACCOUNTSDIR)/AccMsgs.uil \
		 CreateAcc.uil \
		 RetireAcc.uil \
		 CreateGrp.uil

# uil doesn't have any control flow.
# as we need to eliminate the help stuff for Osf1 systems,
# use standard ifdef's and run through preprocessor
XSysAdmin.uil: $(UILS1)
	$(RM) XSysAdmin.uil

	$(CPP) -P XSysAdmin.src.uil >XSysAdmin.uil

all:: $(ALL)

XSysAdmin: $(OBJS)
	$(RM) $@
	$(CC) -o $@ $(OBJS) $(LDOPTIONS) $(LIBS)  $(LDLIBS) $(SYSLIBS) $(EXTRA_LOAD_FLAGS)

clean::
	$(RM) XSysAdmin

install::
	@case '${MFLAGS}' in *[i]*) set +e;; esac;
	@for i in $(LIBDIR)/uid; do if [ -d $(DESTDIR)$$i ]; then \
		set +x; else (set -x; $(MKDIRHIER) $(DESTDIR)$$i); fi \
	done

all:: XSysAdmin.uid

XSysAdmin.uid::  $(UILS1)
	$(RM) $@
	$(UIL) -o $@ XSysAdmin.uil $(UILFLAGS) $(UILINCLUDES)

clean::
	$(RM) XSysAdmin.uid

install:: XSysAdmin.uid
	$(INSTALL) -c $(INSTDATFLAGS) XSysAdmin.uid $(DESTDIR)$(LIBDIR)/uid

depend:: $(DEPEND)

$(DEPEND):
	@echo "checking $@ over in $(DEPENDSRC) first..."; \
	cd $(DEPENDSRC); $(MAKE); \
	echo "okay, continuing in $(CURRENT_DIR)"

depend::
	$(DEPEND) $(DEPENDFLAGS) -s "# DO NOT DELETE" -- $(ALLDEFINES) -- $(SRCS)

install:: XSysAdmin
	@if [ -d $(DESTDIR)$(EXEC_DEST) ]; then set +x; \
		else (set -x; $(MKDIRHIER) $(DESTDIR)$(EXEC_DEST)); fi
	$(INSTALL) -c $(INSTPGMFLAGS)  XSysAdmin $(DESTDIR)$(EXEC_DEST)

install:: XSysAdmin.res
	@if [ -d $(DESTDIR)$(XAPPLOADDIR) ]; then set +x; \
		else (set -x; $(MKDIRHIER) $(DESTDIR)$(XAPPLOADDIR)); fi
	$(INSTALL) -c $(INSTDATFLAGS) XSysAdmin.res $(DESTDIR)$(XAPPLOADDIR)/XSysAdmin

# -------------------------------------------------------------------------
# common rules for all Makefiles - do not edit

emptyrule::

clean::
	$(RM_CMD) "#"*

Makefile:: $(IMAKE)

$(IMAKE):
	@(cd $(IMAKESRC); if [ -f Makefile ]; then \
	echo "checking $@ in $(IMAKESRC) first..."; $(MAKE) all; else \
	echo "bootstrapping $@ from Makefile.ini in $(IMAKESRC) first..."; \
	$(MAKE) -f Makefile.ini BOOTSTRAPCFLAGS="$(BOOTSTRAPCFLAGS)"; fi; \
	echo "okay, continuing in $(CURRENT_DIR)")

Makefile::
	-@if [ -f Makefile ]; then set -x; \
		$(RM) Makefile.bak; $(MV) Makefile Makefile.bak; \
	else exit 0; fi
	$(IMAKE_CMD) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)

tags::
	$(TAGS) -w *.[ch]
	$(TAGS) -xw *.[ch] > TAGS

# -------------------------------------------------------------------------
# empty rules for directories that do not have SUBDIRS - do not edit

install::
	@echo "install in $(CURRENT_DIR) done"

install.man::
	@echo "install.man in $(CURRENT_DIR) done"

Makefiles::

includes::

# -------------------------------------------------------------------------
# dependencies generated by makedepend

# DO NOT DELETE

XSysAdmin.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stdio.h
XSysAdmin.o: ../../../.././../tools/alpha_osf1/cc/usr/include/standards.h
XSysAdmin.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/secdefines.h
XSysAdmin.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/security.h
XSysAdmin.o: ../../../.././X11/Xm/Text.h ../../../.././X11/Xm/Xm.h
XSysAdmin.o: ../../../.././X11/Intrinsic.h ../../../.././X11/Xlib.h
XSysAdmin.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/types.h
XSysAdmin.o: ../../../.././X11/X.h ../../../.././X11/Xfuncproto.h
XSysAdmin.o: ../../../.././X11/Xosdefs.h
XSysAdmin.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stddef.h
XSysAdmin.o: ../../../.././X11/Xutil.h ../../../.././X11/Xresource.h
XSysAdmin.o: ../../../.././../tools/alpha_osf1/cc/usr/include/string.h
XSysAdmin.o: ../../../.././X11/Core.h ../../../.././X11/Composite.h
XSysAdmin.o: ../../../.././X11/Constraint.h ../../../.././X11/Object.h
XSysAdmin.o: ../../../.././X11/RectObj.h ../../../.././X11/Shell.h
XSysAdmin.o: ../../../.././X11/Xatom.h ../../../.././X11/Xm/XmStrDefs.h
XSysAdmin.o: ../../../.././X11/StringDefs.h ../../../.././X11/Xm/VirtKeys.h
XSysAdmin.o: ../../../.././X11/Xm/VendorS.h ../../../.././X11/Mrm/MrmAppl.h
XSysAdmin.o: ../../../.././X11/Xm/XmP.h ../../../.././X11/IntrinsicP.h
XSysAdmin.o: ../../../.././X11/CoreP.h ../../../.././X11/CompositeP.h
XSysAdmin.o: ../../../.././X11/ConstrainP.h ../../../.././X11/ObjectP.h
XSysAdmin.o: ../../../.././X11/RectObjP.h ../../../.././X11/Xm/BaseClassP.h
XSysAdmin.o: ../../../.././X11/Mrm/MrmPublic.h
XSysAdmin.o: ../../../.././X11/Mrm/MrmDecls.h ../../../.././X11/Xm/Xm.h
XSysAdmin.o: ../../../.././X11/Xm/Display.h ../../../.././X11/Xm/DragC.h
XSysAdmin.o: ../../../.././X11/Xm/Display.h ../../../.././X11/Xm/DropSMgr.h
XSysAdmin.o: ../../../.././motif/clients/xif/common/Vuit.h
XSysAdmin.o: ../../../.././motif/clients/xif/common/Utilities.h
XSysAdmin.o: ../../../.././motif/clients/xif/common/XMain.h
XSysAdmin.o: ../../../.././motif/clients/xif/common/Resources.h
adminLocalInfo.o: ../../../.././../tools/alpha_osf1/cc/usr/include/time.h
adminLocalInfo.o: ../../../.././../tools/alpha_osf1/cc/usr/include/standards.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stdio.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/standards.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/pwd.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/types.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/grp.h
CreateAcc.o: ../../../.././X11/Xm/TextF.h ../../../.././X11/Xm/Xm.h
CreateAcc.o: ../../../.././X11/Intrinsic.h ../../../.././X11/Xlib.h
CreateAcc.o: ../../../.././X11/X.h ../../../.././X11/Xfuncproto.h
CreateAcc.o: ../../../.././X11/Xosdefs.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stddef.h
CreateAcc.o: ../../../.././X11/Xutil.h ../../../.././X11/Xresource.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/string.h
CreateAcc.o: ../../../.././X11/Core.h ../../../.././X11/Composite.h
CreateAcc.o: ../../../.././X11/Constraint.h ../../../.././X11/Object.h
CreateAcc.o: ../../../.././X11/RectObj.h ../../../.././X11/Shell.h
CreateAcc.o: ../../../.././X11/Xatom.h ../../../.././X11/Xm/XmStrDefs.h
CreateAcc.o: ../../../.././X11/StringDefs.h ../../../.././X11/Xm/VirtKeys.h
CreateAcc.o: ../../../.././X11/Xm/VendorS.h ../../../.././X11/Xm/PushB.h
CreateAcc.o: ../../../.././X11/Mrm/MrmAppl.h ../../../.././X11/Xm/XmP.h
CreateAcc.o: ../../../.././X11/IntrinsicP.h ../../../.././X11/CoreP.h
CreateAcc.o: ../../../.././X11/CompositeP.h ../../../.././X11/ConstrainP.h
CreateAcc.o: ../../../.././X11/ObjectP.h ../../../.././X11/RectObjP.h
CreateAcc.o: ../../../.././X11/Xm/BaseClassP.h
CreateAcc.o: ../../../.././X11/Mrm/MrmPublic.h
CreateAcc.o: ../../../.././X11/Mrm/MrmDecls.h ../isso/XIsso.h
CreateAcc.o: ../../../.././motif/clients/xif/common/Vuit.h
CreateAcc.o: ../../../.././motif/clients/xif/common/XMain.h
CreateAcc.o: ../../../.././motif/clients/xif/common/XAccounts.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/secdefines.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/security.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/audit.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/time.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/limits.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/signal.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/time.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/param.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/machparam.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/endian.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/machlimits.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/syscall.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/systm.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/habitat.h
CreateAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/prot.h
CreateAcc.o: ../../../.././motif/clients/xif/common/Utilities.h
CreateAcc.o: ../../../.././motif/clients/xif/common/Resources.h
CreateAcc.o: ../isso/accounts/AccountSelection.h ../isso/accounts/AccCommon.h
CreateAcc.o: ../../../.././motif/clients/xif/common/Messages.h
CreateAcc.o: ../../../.././motif/clients/xif/common/hyperhelp.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stdio.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/standards.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/pwd.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/types.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/grp.h
CreateGrp.o: ../../../.././X11/Xm/TextF.h ../../../.././X11/Xm/Xm.h
CreateGrp.o: ../../../.././X11/Intrinsic.h ../../../.././X11/Xlib.h
CreateGrp.o: ../../../.././X11/X.h ../../../.././X11/Xfuncproto.h
CreateGrp.o: ../../../.././X11/Xosdefs.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stddef.h
CreateGrp.o: ../../../.././X11/Xutil.h ../../../.././X11/Xresource.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/string.h
CreateGrp.o: ../../../.././X11/Core.h ../../../.././X11/Composite.h
CreateGrp.o: ../../../.././X11/Constraint.h ../../../.././X11/Object.h
CreateGrp.o: ../../../.././X11/RectObj.h ../../../.././X11/Shell.h
CreateGrp.o: ../../../.././X11/Xatom.h ../../../.././X11/Xm/XmStrDefs.h
CreateGrp.o: ../../../.././X11/StringDefs.h ../../../.././X11/Xm/VirtKeys.h
CreateGrp.o: ../../../.././X11/Xm/VendorS.h ../../../.././X11/Xm/PushB.h
CreateGrp.o: ../../../.././X11/Mrm/MrmAppl.h ../../../.././X11/Xm/XmP.h
CreateGrp.o: ../../../.././X11/IntrinsicP.h ../../../.././X11/CoreP.h
CreateGrp.o: ../../../.././X11/CompositeP.h ../../../.././X11/ConstrainP.h
CreateGrp.o: ../../../.././X11/ObjectP.h ../../../.././X11/RectObjP.h
CreateGrp.o: ../../../.././X11/Xm/BaseClassP.h
CreateGrp.o: ../../../.././X11/Mrm/MrmPublic.h
CreateGrp.o: ../../../.././X11/Mrm/MrmDecls.h ../isso/XIsso.h
CreateGrp.o: ../../../.././motif/clients/xif/common/Vuit.h
CreateGrp.o: ../../../.././motif/clients/xif/common/XMain.h
CreateGrp.o: ../../../.././motif/clients/xif/common/XAccounts.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/secdefines.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/security.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/audit.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/time.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/limits.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/signal.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/time.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/param.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/machparam.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/endian.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/machlimits.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/syscall.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/systm.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/habitat.h
CreateGrp.o: ../../../.././../tools/alpha_osf1/cc/usr/include/prot.h
CreateGrp.o: ../../../.././motif/clients/xif/common/Utilities.h
CreateGrp.o: ../../../.././motif/clients/xif/common/Resources.h
CreateGrp.o: ../isso/accounts/AccountSelection.h ../isso/accounts/AccCommon.h
CreateGrp.o: ../../../.././motif/clients/xif/common/Messages.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stdio.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/standards.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/pwd.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/types.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/grp.h
RetireAcc.o: ../../../.././X11/Xm/TextF.h ../../../.././X11/Xm/Xm.h
RetireAcc.o: ../../../.././X11/Intrinsic.h ../../../.././X11/Xlib.h
RetireAcc.o: ../../../.././X11/X.h ../../../.././X11/Xfuncproto.h
RetireAcc.o: ../../../.././X11/Xosdefs.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/stddef.h
RetireAcc.o: ../../../.././X11/Xutil.h ../../../.././X11/Xresource.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/string.h
RetireAcc.o: ../../../.././X11/Core.h ../../../.././X11/Composite.h
RetireAcc.o: ../../../.././X11/Constraint.h ../../../.././X11/Object.h
RetireAcc.o: ../../../.././X11/RectObj.h ../../../.././X11/Shell.h
RetireAcc.o: ../../../.././X11/Xatom.h ../../../.././X11/Xm/XmStrDefs.h
RetireAcc.o: ../../../.././X11/StringDefs.h ../../../.././X11/Xm/VirtKeys.h
RetireAcc.o: ../../../.././X11/Xm/VendorS.h ../../../.././X11/Xm/PushB.h
RetireAcc.o: ../../../.././X11/Mrm/MrmAppl.h ../../../.././X11/Xm/XmP.h
RetireAcc.o: ../../../.././X11/IntrinsicP.h ../../../.././X11/CoreP.h
RetireAcc.o: ../../../.././X11/CompositeP.h ../../../.././X11/ConstrainP.h
RetireAcc.o: ../../../.././X11/ObjectP.h ../../../.././X11/RectObjP.h
RetireAcc.o: ../../../.././X11/Xm/BaseClassP.h
RetireAcc.o: ../../../.././X11/Mrm/MrmPublic.h
RetireAcc.o: ../../../.././X11/Mrm/MrmDecls.h ../isso/XIsso.h
RetireAcc.o: ../../../.././motif/clients/xif/common/Vuit.h
RetireAcc.o: ../../../.././motif/clients/xif/common/XMain.h
RetireAcc.o: ../../../.././motif/clients/xif/common/XAccounts.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/secdefines.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/security.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/audit.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/time.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/limits.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/signal.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/time.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/param.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/machparam.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/endian.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/machine/machlimits.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/syscall.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/systm.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/sys/habitat.h
RetireAcc.o: ../../../.././../tools/alpha_osf1/cc/usr/include/prot.h
RetireAcc.o: ../../../.././motif/clients/xif/common/Utilities.h
RetireAcc.o: ../../../.././motif/clients/xif/common/Resources.h
RetireAcc.o: ../isso/accounts/AccountSelection.h ../isso/accounts/AccCommon.h
RetireAcc.o: ../../../.././motif/clients/xif/common/Messages.h
