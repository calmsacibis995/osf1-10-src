/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
***********************************************************
**                                                        *
**  Copyright (c) Digital Equipment Corporation, 1990  	  *
**  All Rights Reserved.  Unpublished rights reserved	  *
**  under the copyright laws of the United States.	  *
**                                                        *
**  The software contained on this media is proprietary	  *
**  to and embodies the confidential technology of 	  *
**  Digital Equipment Corporation.  Possession, use,	  *
**  duplication or dissemination of the software and	  *
**  media is authorized only pursuant to a valid written  *
**  license from Digital Equipment Corporation.	    	  *
**  							  *
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	  *
**  disclosure by the U.S. Government is subject to	  *
**  restrictions as set forth in Subparagraph (c)(1)(ii)  *
**  of DFARS 252.227-7013, or in FAR 52.227-19, as	  *
**  applicable.	    					  *
**  		                                          *
***********************************************************
**++
**  FACILITY:
**
**	< to be supplied >
**
**  ABSTRACT:
**
**	< to be supplied >
**
**  ENVIRONMENT:
**
**	< to be supplied >
**
**  MODIFICATION HISTORY:
**
**	RKR		05-Mar-1991
**	Remove the resources assocated with On Context, On Version and On Terms,
**	namely;
**	    DXmNoncontextLabel, DXmNoncontextLabelMnem, DXmNoncontextLabelMnemCS
**	    DXmNglossaryLabel,  DXmNglossaryLabelMnem,  DXmNglossaryLabelMnemCS
**	    DXmNaboutLabel,     DXmNaboutLabelMnem,     DXmNaboutLabelMnemCS
**
**	Will	   	04-Apr-1991
**      Add function prototypes.
**	RKR		21-Aug-1990
**	Add external definition of DXmCreateHelpDialog
**	RKR		19-Aug-1990
**	Define resource for dialog style
**	RKR		25-Jun-1990
**	Define resources for On_Context help.
**
**	RKR		15-Jun-1990
**	Define resources for mnemonics.
**
**	RKR 		17-May-1990
**	Remove definition of DXmNunmapCallback to avoid confusion.
**
**
**--
**/

#ifndef _DXmHelpBox_h
#define _DXmHelpBox_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:Xm.h>
#else
#include <Xm/Xm.h>
#endif

/* Class record constants */

externalref WidgetClass		dxmHelpWidgetClass ;


/* help widget library type */

#define DXmTextLibrary		1


/*
 * help_widget.c  (sub-class of common)
 */

#define DXmNcols		"DXmcols"
#define DXmCCols		"DXmCols"
#define DXmNrows		"DXmrows"
#define DXmCRows		"DXmRows"
#define DXmNdefaultPosition	"DXmdefaultPosition"
#define DXmCDefaultPosition	"DXmDefaultPosition"
#define DXmNlibraryType		"DXmlibraryType"
#define DXmCLibraryType 	"DXmLibraryType"
#define DXmNlibrarySpec		"DXmlibrarySpec"
#define DXmNapplicationName	"DXmapplicationName"
#define DXmNfirstTopic		"DXmfirstTopic"
#define DXmNoverviewTopic	"DXmoverviewTopic"
#define DXmNglossaryTopic	"DXmglossaryTopic"
#define DXmNviewLabel		"DXmviewLabel"
#define DXmNgotoLabel		"DXmgotoLabel"
#define DXmNgobackLabel		"DXmgobackLabel"
#define DXmNgooverLabel		"DXmgooverLabel"
#define DXmNvisitLabel		"DXmvisitLabel"
#define DXmNvisitglosLabel	"DXmvisitglosLabel"
#define DXmNfileLabel		"DXmfileLabel"
#define DXmNsaveasLabel		"DXmsaveasLabel"
#define DXmNexitLabel		"DXmexitLabel"
#define DXmNeditLabel		"DXmeditLabel"
#define DXmNcopyLabel		"DXmcopyLabel"			
#define DXmNselectallLabel	"DXmselectallLabel"
#define DXmNsearchLabel		"DXmsearchLabel"
#define DXmNhistoryLabel	"DXmhistoryLabel"
#define DXmNtitleLabel		"DXmtitleLabel"
#define DXmNkeywordLabel	"DXmkeywordLabel"
#define DXmNhelpLabel		"DXmhelpLabel"	

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNglossaryLabel	"DXmglossaryLabel"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNaboutLabel		"DXmaboutLabel"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNoncontextLabel	"DXmoncontextLabel"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#define DXmNaddtopicLabel	"DXmaddtopicLabel"
#define DXmNhistoryboxLabel	"DXmhistoryboxLabel"
#define DXmNtopictitlesLabel	"DXmtopictitlesLabel"
#define DXmNdismissLabel	"DXmdismissLabel"
#define DXmNsearchtitleboxLabel	"DXmsearchtitleboxLabel"
#define DXmNtitlesLabel		"DXmtitlesLabel"
#define DXmNsearchkeywordboxLabel   "DXmsearchkeywordboxLabel"
#define DXmNkeywordsLabel	"DXmkeywordsLabel"
#define DXmNsearchapplyLabel	 "DXmsearchapplyLabel" 
#define DXmNbadlibMessage	 "DXmbadlibMessage"
#define DXmNbadframeMessage	 "DXmbadframeMessage"
#define DXmNnulllibMessage	 "DXmnulllibMessage"
#define DXmNnotitleMessage	 "DXmnotitleMessage"
#define DXmNnokeywordMessage	 "DXmnokeywordMessage"
#define DXmNerroropenMessage	 "DXmerroropenMessage"
#define DXmNgototopicLabel	 "DXmgototopicLabel"
#define DXmNgobacktopicLabel	 "DXmgobacktopicLabel"
#define DXmNvisittopicLabel	 "DXmvisittopicLabel"
#define DXmNcloseLabel		 "DXmcloseLabel"
#define DXmNhelphelpLabel	 "DXmhelphelpLabel"
#define DXmNhelpontitleLabel	 "DXmhelpontitleLabel"
#define DXmNhelptitleLabel	 "DXmhelptitleLabel"
#define DXmNhelpAcknowledgeLabel "DXmhelpAcknowledgeLabel"
#define DXmNhelpOnHelpTitle	 "DXmhelpOnHelpTitle"
#define DXmNcacheHelpLibrary	 "DXmcacheHelpLibrary"
#define DXmCCacheHelpLibrary	 "DXmCacheHelpLibrary"

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNaboutLabelMnem	"DXmaboutLabelMnemonic"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNoncontextLabelMnem	"DXmoncontextLabelMnemonic"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#define DXmNcopyLabelMnem	"DXmcopyLabelMnemonic"
#define DXmNeditLabelMnem	"DXmeditLabelMnemonic"
#define DXmNexitLabelMnem	"DXmexitLabelMnemonic"
#define DXmNfileLabelMnem	"DXmfileLabelMnemonic"

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNglossaryLabelMnem	"DXmglossaryLabelMnemonic"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#define DXmNgooverLabelMnem	"DXmgooverLabelMnemonic"
#define DXmNhelpLabelMnem	"DXmhelpLabelMnemonic"
#define DXmNhistoryLabelMnem	"DXmhistoryLabelMnemonic"
#define DXmNkeywordLabelMnem	"DXmkeywordLabelMnemonic"
#define DXmNsaveasLabelMnem	"DXmsaveasLabelMnemonic"
#define DXmNsearchLabelMnem	"DXmsearchLabelMnemonic"
#define DXmNselectallLabelMnem	"DXmselectallLabelMnemonic"
#define DXmNtitleLabelMnem	"DXmtitleLabelMnemonic"
#define DXmNviewLabelMnem	"DXmviewLabelMnemonic"
#define DXmNvisitglosLabelMnem	"DXmvisitglosLabelMnemonic"
#define DXmNgototopicLabelMnem	"DXmgototopicLabelMnemonic"
#define DXmNgobackLabelMnem	"DXmgobackLabelMnemonic"
#define DXmNvisittopicLabelMnem	"DXmvisittopicLabelMnemonic"
#define DXmNhelphelpLabelMnem	"DXmhelphelplabelMnemonic"

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNaboutLabelMnemCS	"DXmaboutLabelMnemonicCS"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNoncontextLabelMnemCS "DXmoncontextLabelMnemonicCS"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#define DXmNcopyLabelMnemCS	"DXmcopyLabelMnemonicCS"
#define DXmNeditLabelMnemCS	"DXmeditLabelMnemonicCS"
#define DXmNexitLabelMnemCS	"DXmexitLabelMnemonicCS"
#define DXmNfileLabelMnemCS	"DXmfileLabelMnemonicCS"

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmNglossaryLabelMnemCS	"DXmglossaryLabelMnemonicCS"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#define DXmNgooverLabelMnemCS	"DXmgooverLabelMnemonicCS"
#define DXmNhelpLabelMnemCS	"DXmhelpLabelMnemonicCS"
#define DXmNhistoryLabelMnemCS	"DXmhistoryLabelMnemonicCS"
#define DXmNkeywordLabelMnemCS	"DXmkeywordLabelMnemonicCS"
#define DXmNsaveasLabelMnemCS	"DXmsaveasLabelMnemonicCS"
#define DXmNsearchLabelMnemCS	"DXmsearchLabelMnemonicCS"
#define DXmNselectallLabelMnemCS "DXmselectallLabelMnemonicCS"
#define DXmNtitleLabelMnemCS	"DXmtitleLabelMnemonicCS"
#define DXmNviewLabelMnemCS	"DXmviewLabelMnemonicCS"
#define DXmNvisitglosLabelMnemCS "DXmvisitglosLabelMnemonicCS"
#define DXmNgototopicLabelMnemCS "DXmgototopicLabelMnemonicCS"
#define DXmNgobackLabelMnemCS	"DXmgobackLabelMnemonicCS"
#define DXmNvisittopicLabelMnemCS "DXmvisittopicLabelMnemonicCS"
#define DXmNhelphelpLabelMnemCS	"DXmhelphelplabelMnemonicCS"

#define DXmNdialogStyle		"DXmNdialogStyle"

/*
 * help widget external routines  
 */

#ifdef _NO_PROTO
extern Widget DXmCreateHelp( );
extern Widget DXmCreateHelpDialog( );
#else
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
extern Widget DXmCreateHelp (Widget parent , char *name , ArgList arglist , int argcount );
extern Widget DXmCreateHelpDialog (Widget parent , char *name , ArgList arglist , int argcount );
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* _NO_PROTO */

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif
