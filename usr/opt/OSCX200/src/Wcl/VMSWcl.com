$ ! VMSWCL.COM
$ ! SCCS_data: @(#) VMSWcl.com 1.2 92/03/18 10:38:08
$ !
$ !     ----------  THIS WILL NOT WORK AS PROVIDED! ----------
$ !
$ ! This is a crude "makefile" for Wcl on VMS (5.4 plus DECWINDOWS MOTIF 1.0)
$ ! You may have to make substantial changes to use this with different
$ ! DECWINDOWS and Motif "developer's kits"
$ !
$ ! You must specify the value of the WCL_ERRORDB flag passed to
$ ! Wc/WcCreate.c which under UNIX is usually $(LIBDIR)/WclErrorDB
$ !
$ ! You must specify the value of the XAPPDIR flag passed to
$ ! Wc/WcLoadRes.c which under UNIX is usually $(XAPPLOADDIR)
$ !
$ ! You may or may not be able to use the CCCMD for Xmp/Table.c.
$ ! I think it should work to define the cpp flags -DUSE_XtResizeWidget
$ ! and -DUSE_XtMoveWidget, but this means the resultant Table will
$ ! certainly crash if any gadget children are somehow provided.  Table
$ ! never intends to support Gadgets, and R4 and later intrinsics will
$ ! never allow gadget children of Tables.  However, R3 intrinsics,
$ ! like those provided with early versions of Motif, do allow gadgets
$ ! as children of XmpTables (it is a bug in those releases of Xt).
$ ! If it does not work, then comment out that line so the Table widget
$ ! is not built into the Xmp library.
$ !
$ ! You must figure out how to pre-process the application defaults
$ ! files provided with Mri.  These require cpp and awk on UNIX systems.
$ ! Look at Mri/MakeByHand, WcMakeC.tmpl, WcClient.tmpl, and AppDef.rules.
$ !
$ ! You also have to munge resource files, as their XtInitalize
$ ! uppercases application name (we want Mri, they change it to MRI)
$ ! and of course, you must add the .dat extension to all resource files.
$ !
$ ! 10/15/91 Martin Brunecky
$ ! 03/01/91 David Smyth (but not tested!!)
$ !
$ Set NoOn
$ olddef = f$environment("DEFAULT")
$ !
$ ! Find where we are (assuming this file is located in the Wcl top directory)
$ this = f$environment("PROCEDURE") - "][" - "]["
$ root = f$element(0,"]",this)
$ !
$ if P1 .eqs. "TEST" then goto TEST
$ !
$ ! Define logicals necessary to allow syntax like #include <X11/Atom.h>
$ ! For "Motif developers Kit" you may have to change those ....
$ define X11 decw$include:
$ define Xt  decw$include:
$ define Xm  decw$include:
$ define Wc  'root'.Wc]
$ !
$ define VAXC$INCLUDE [],X11,Xt,Xm,Wc,decw$include,sys$library
$ define    C$INCLUDE [],X11,Xt,Xm,Wc,decw$include,sys$library
$ !
$ ! Compile everything in Wc subdirectory
$ SET DEF 'root'.Wc]
$ if f$search("WC.OLB") .eqs. "" then $ LIBRARY/CREATE WC.OLB
$ CCCMD = "CC/DEFINE=VAX"
$!CCCMD = "CC/DEFINE=(XtNameToWidgetBarfsOnGadgets)
$ !
$!CCCMD XT4GETRESL.C   
$!CCCMD XTMACROS.C
$!CCCMD XTNAME.C
$ CCCMD MAPAG.C
$ CCCMD WCACTCB.C      
$ CCCMD WCCONVERT.C      
$ CCCMD WCCREATE.C       
$ CCCMD WCINVOKE.C
$ CCCMD WCLOADRES.C
$ CCCMD WCNAME.C         
$ CCCMD WCREG.C          
$ CCCMD WCREGXT.C
$ CCCMD WCSETVALUE.C
$ CCCMD WCTEMPLATE.C
$ CCCMD WCWARN.C
$ PURGE *.OBJ        
$ LIBR/REPLACE WC.OLB *.OBJ
$ !
$ ! Compile everything in Xmp subdirectory
$ SET DEF 'root'.Xmp]
$ if f$search("XMP.OLB") .eqs. "" then $ LIBRARY/CREATE XMP.OLB
$ CCCMD = "CC/DEFINE=(VAX,USE_XtMoveWidget,USE_XtResizeWidget)"
$ ! 
$ CCCMD XMP.C
$ CCCMD XMPREGALL.C
$ CCCMD TABLE.C
$ CCCMD TABLELOC.C
$ CCCMD TABLEVEC.C
$ PURGE *.OBJ
$ LIBR/REPLACE XMP.OLB *.OBJ
$ !
$ ! Compile and build Mri
$ !
$ ! You also need to pre-process the application resource files
$ ! like the makefiles and Imakefiles do, to tailor them to Motif 1.0
$ ! or Motif 1.1 and to provide installation locations within them.
$ ! I do not know how to do that on a VAX - des
$ !
$ SET DEF 'root'.Mri]
$ CCCMD = "CC/DEFINE=VAX"
$ CCCMD MRI.C
$ LINK/EXE=MRI MRI.OBJ,[-.WC]WC/LIB,[-.XMP]XMP/LIB,[-.WC]WC/LIB,sys$input/OPT
sys$library:DECW$XMLIBSHR/share
sys$library:DECW$XTSHR/share
sys$library:DECW$XLIBSHR/share
sys$library:VAXCRTL/share
$ SET DEF 'olddef'
$ !
$ ! Execute MRI demos/tests
$TEST:
$ MRI == "$''root'.MRI]MRI.EXE"
$ DEFINE DECW$USER_DEFAULTS 'root'.MRI]
$ MRI HELLO
$ MRI GOODBYE
$ MRI WCALL
$ MRI MENUBAR
$ MRI OPTMENU
$ MRI DIALOGS
$ MRI TRAVERSAL
$ MRI LISTRC
$ MRI LISTTABLE
$ MRI FORM
$ MRI PERIODIC
$ MRI PERTEM
$ MRI TEMPLATE
$ MRI POPUP
$ MRI FSB
$ MRI MODAL
$ MRI TABLEDIALOG
$ MRI APPSHELLS
$ EXIT 
