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
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: %W% %E%";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * This module contains routines responsible for writing the .mm files
 * produced by WML. All files are written into the current directory.
 *
 * Input:
 *	The resolved objects
 *
 * Output:
 *	wml-uil.mm
#ifdef DEC_MOTIF_EXTENSION
 *	wml-uil.sdml
#endif
 *
 */


#include "wml.h"

#ifdef NULL
#undef NULL
#endif
#include <stdio.h>


/*
 * Routines used only in this module
 */
void wmlOutputWmlUilMm ();
void wmlOutputWmlUilMmClass ();
#ifdef DEC_MOTIF_EXTENSION
void wmlOutputWmlUilSdmlControls();
#endif

/*
 * globals
 */
int			tabno = 1;	/* table number */

/*
 * lists re-used repeatedly to order lists for printing
 */
DynamicHandleListDef	mm_arg;
DynamicHandleListDefPtr	mm_arg_ptr = &mm_arg;
DynamicHandleListDef	mm_rsn;
DynamicHandleListDefPtr	mm_rsn_ptr = &mm_rsn;
DynamicHandleListDef	mm_ctl;
DynamicHandleListDefPtr	mm_ctl_ptr = &mm_ctl;



/*
 * Output control routine, which simply outputs each .mm file in turn.
 */

void wmlOutputMmFiles ()

{

wmlOutputWmlUilMm ();

}



/*
 * Routine to write out wml-uil.mm
 *
 * This .mm file contains the tables which are to be included as an
 * appendix to the Uil manual. The tables give the arguments with their
 * default values, reasons, constraints, and controls for each class
 * in the class vectors.
 */

void wmlOutputWmlUilMm ()

{

FILE			*outfil;	/* output file */
#ifdef	    DEC_MOTIF_EXTENSION
FILE			*sdmlfil;	/* sdml output file */
#endif
int			ndx;		/* loop index */
WmlClassDefPtr		clsobj;		/* class object */



/*
 * Open the output file. Write the canned header stuff
 */
outfil = fopen ("wml-uil.mm", "w");
#ifdef	    DEC_MOTIF_EXTENSION
sdmlfil = fopen ("wml-uil.sdml", "w");
#endif

/*
 * Initialize order lists for the tables.
 */
wmlInitHList (mm_arg_ptr, 200, TRUE);
wmlInitHList (mm_rsn_ptr, 200, TRUE);
wmlInitHList (mm_ctl_ptr, 200, TRUE);

/*
 * Write out a table for each class, for both widget and gadget variants
 */
for ( ndx=0 ; ndx<wml_obj_class_ptr->cnt ; ndx++ )
    {
    clsobj = (WmlClassDefPtr) wml_obj_class_ptr->hvec[ndx].objptr;
#ifdef DEC_MOTIF_EXTENSION
    wmlOutputWmlUilMmClass (outfil, sdmlfil, clsobj, clsobj->syndef->name);
#else
    wmlOutputWmlUilMmClass (outfil, clsobj, clsobj->syndef->name);
#endif
    }

#ifdef DEC_MOTIF_EXTENSION
wmlOutputWmlUilSdmlControls (outfil, sdmlfil, "AllWidgetsAndGadgets");
wmlOutputWmlUilSdmlControls (outfil, sdmlfil, "AllWidgets");
#endif

/*
 * close the output file
 */
#ifdef DEC_MOTIF_EXTENSION
printf ("\nCreated wml-uil.mm and wml-uil.sdml");
fclose (sdmlfil);
#else
printf ("\nCreated wml-uil.mm");
#endif
fclose (outfil);

}



/*
 * Routine to write a table for a class entry
 */

#ifdef DEC_MOTIF_EXTENSION
void wmlOutputWmlUilMmClass (outfil, sdmlfil, clsobj, name)
    FILE			*outfil;
    FILE			*sdmlfil;
#else
void wmlOutputWmlUilMmClass (outfil, clsobj, name)
    FILE			*outfil;
#endif
    WmlClassDefPtr		clsobj;
    char			*name;

{

char		*canned1 =
".bp\n\
.ps 12\n";

char		*canned2 =
".ps 10\n\
.vs 12\n\
.LP\n\
.TS H\n\
tab(@);\n\
lB lB\n\
l l.\n\
_\n\
.sp 6p\n\
Controls@Reasons\n\
.sp 6p\n\
_\n\
.sp 6p\n\
.TH\n";

char		*canned3 =
".TE\n\
.TS H\n\
tab(@);\n\
lB lB lB\n\
l l l.\n\
_\n\
.sp 6p\n\
UIL Argument Name@Argument Type@Default Value\n\
.sp 6p\n\
_\n\
.sp 6p\n\
.TH\n";

char		*canned4 =
".TE\n";

#ifdef DEC_MOTIF_EXTENSION
char		*sdmlCanned1 =
"<PAGE>\n\
<HEAD1>(%s\\%s_sec)\n\
<X>(%s)\n";

char		*sdmlCanned2 =
"<ONLINE_POPUP>(TABLE)\n\
<TABLE>\n\
<TABLE_ATTRIBUTES>(\\wide\\multipage)\n\
<TABLE_SETUP>(2\\15)\n\
<TABLE_HEADS>(Controls\\Reasons)\n";

char		*sdmlCanned3 =
"<ENDTABLE>\n\
<ENDONLINE_POPUP>\n\
\n\
<ONLINE_POPUP>(TABLE)\n\
<TABLE>\n\
<TABLE_ATTRIBUTES>(\\wide\\multipage)\n\
<TABLE_SETUP>(3\\22\\15)\n\
<TABLE_HEADS>(UIL Argument Name\\Argument Type\\Default Value)\n";

char		*sdmlCanned4 =
"<ENDTABLE>\n\
<ENDONLINE_POPUP>\n\
\n";

int		skip1 = FALSE;
#endif

WmlClassResDefPtr	argref;		/* current argument reference */
WmlClassResDefPtr	rsnref;		/* current reason reference */
WmlClassCtrlDefPtr	ctlref;		/* current controls reference */
int			argndx;		/* to access ordered vector */
int			rsnndx;		/* to access ordered vector */
int			ctlndx;		/* to access ordered vector */


/*
 * Write out header information
 */
fprintf (outfil, canned1);
fprintf (outfil, "%s\n", name);
fprintf (outfil, canned2);

#ifdef DEC_MOTIF_EXTENSION
fprintf (sdmlfil, sdmlCanned1, name, name, name);
fprintf (sdmlfil, sdmlCanned2);
#endif

/*
 * Alphabetize the controls, reason, and argument lists
 */
wmlClearHList (mm_arg_ptr);
wmlClearHList (mm_rsn_ptr);
wmlClearHList (mm_ctl_ptr);

argref = clsobj->arguments;
while ( argref!= NULL )
    {
    while ( argref != NULL && argref->exclude == WmlAttributeTrue )
	argref = argref->next;
    if ( argref != NULL )
	{
	wmlInsertInHList (mm_arg_ptr,
			  argref->act_resource->syndef->name,
			  (ObjectPtr)argref);
	argref = argref->next;
	}
    }

rsnref = clsobj->reasons;
while ( rsnref!=NULL )
    {
    while ( rsnref != NULL && rsnref->exclude == WmlAttributeTrue )
	rsnref = rsnref->next;
    if ( rsnref != NULL )
	{
	wmlInsertInHList (mm_rsn_ptr,
			  rsnref->act_resource->syndef->name,
			  (ObjectPtr)rsnref);
	
	rsnref = rsnref->next;
	}
    }

#ifdef DEC_MOTIF_EXTENSION
ctlref = clsobj->controls;
if (ctlref != NULL)
    {
    if ((strcmp (clsobj->syndef->controls->name,"AllWidgetsAndGadgets") == 0) ||
	(strcmp (clsobj->syndef->controls->name,"AllWidgets") == 0))
	{
	wmlInsertInHList (mm_ctl_ptr,
			  clsobj->syndef->controls->name,
			  (ObjectPtr)clsobj);
	}
    else
	{
	while ( ctlref != NULL )
	    {
	    wmlInsertInHList (mm_ctl_ptr,
			      ctlref->ctrl->syndef->name,
			      (ObjectPtr)ctlref);
	    ctlref = ctlref->next;
	    }
	}
    }
#else
ctlref = clsobj->controls;
while ( ctlref != NULL )
    {
    wmlInsertInHList (mm_ctl_ptr,
		      ctlref->ctrl->syndef->name,
		      (ObjectPtr)ctlref);
    ctlref = ctlref->next;
    }
#endif

/*
 * Write out the controls and reason table.
 */
rsnndx = 0;
ctlndx = 0;
if ( mm_ctl_ptr->cnt == 0 )
#ifdef DEC_MOTIF_EXTENSION
    {
    fprintf (outfil, "No children are supported");
    fprintf (sdmlfil, "<TABLE_ROW>(No children are supported\\");
    skip1 = TRUE;
    }
#else
    fprintf (outfil, "No children are supported");
#endif
while ( rsnndx<mm_rsn_ptr->cnt || ctlndx<mm_ctl_ptr->cnt )
    {
    if ( ctlndx < mm_ctl_ptr->cnt )
	{
#ifdef	DEC_MOTIF_EXTENSION
	fprintf (outfil, "%s@",
		 mm_ctl_ptr->hvec[ctlndx].objname);
	fprintf (sdmlfil, "<TABLE_ROW>(%s\\",
		 mm_ctl_ptr->hvec[ctlndx].objname);
#else
	ctlref = (WmlClassCtrlDefPtr) mm_ctl_ptr->hvec[ctlndx].objptr;
	fprintf (outfil, "%s@",
		 ctlref->ctrl->syndef->name);
#endif
	ctlndx += 1;
	}
    else
#ifdef DEC_MOTIF_EXTENSION
	{
	fprintf (outfil, "@");
	if ( skip1 )
	    skip1 = FALSE;
	else
	    fprintf (sdmlfil, "<TABLE_ROW>(\\");
	}
#else
	fprintf (outfil, "@");
#endif
    
    if ( rsnndx < mm_rsn_ptr->cnt )
	{
	rsnref = (WmlClassResDefPtr) mm_rsn_ptr->hvec[rsnndx].objptr;
	fprintf (outfil, "%s\n",
		 rsnref->act_resource->syndef->name);
#ifdef DEC_MOTIF_EXTENSION
	fprintf (sdmlfil, "%s)\n",
		 rsnref->act_resource->syndef->name);
#endif
	rsnndx += 1;
	}
    else
#ifdef DEC_MOTIF_EXTENSION
	{
	fprintf (outfil, "\n");
	fprintf (sdmlfil, ")\n");
	}
#else
	fprintf (outfil, "\n");
#endif
    }
fprintf (outfil, canned3);
#ifdef DEC_MOTIF_EXTENSION
if (mm_arg_ptr->cnt)
    {
    fprintf (sdmlfil, sdmlCanned3);
    }
else
    {
    fprintf (sdmlfil, sdmlCanned4);
    }
#endif

/*
 * Write out the argument table
 */
argndx = 0;
while ( argndx < mm_arg_ptr->cnt )
    {
    argref = (WmlClassResDefPtr) mm_arg_ptr->hvec[argndx].objptr;
    fprintf (outfil, "%s@%s@",
	     argref->act_resource->syndef->name,
	     argref->act_resource->dtype_def->syndef->name);
#ifdef DEC_MOTIF_EXTENSION
    fprintf (sdmlfil, "<TABLE_ROW>(%s\\%s\\",
	     argref->act_resource->syndef->name,
	     argref->act_resource->dtype_def->syndef->name);
#endif
    if ( argref->dflt != NULL )
	{
	if ( strchr(argref->dflt," ") != 0 )
#ifdef DEC_MOTIF_EXTENSION
	    {
	    fprintf (outfil, "T{\n%s\nT}\n",
		     argref->dflt);
	    fprintf (sdmlfil, "%s)\n",
		     argref->dflt);
	    }
	else
	    {
	    fprintf (outfil, "%s\n",
		     argref->dflt);
	    fprintf (sdmlfil, "%s)\n",
		     argref->dflt);
	    }
#else
	    fprintf (outfil, "T{\n%s\nT}\n",
		     argref->dflt);
	else
	    fprintf (outfil, "%s\n",
		     argref->dflt);
#endif
	}
    else
	{
	if ( strchr(argref->act_resource->syndef->dflt,' ') != 0 )
#ifdef DEC_MOTIF_EXTENSION
	    {
	    fprintf (outfil, "T{\n%s\nT}\n",
		     argref->act_resource->syndef->dflt);
	    fprintf (sdmlfil, "%s)\n",
		     argref->act_resource->syndef->dflt);
	    }
	else
	    {
	    fprintf (outfil, "%s\n",
		     argref->act_resource->syndef->dflt);
	    fprintf (sdmlfil, "%s)\n",
		     argref->act_resource->syndef->dflt);
	    }
#else
	    fprintf (outfil, "T{\n%s\nT}\n",
		     argref->act_resource->syndef->dflt);
	else
	    fprintf (outfil, "%s\n",
		     argref->act_resource->syndef->dflt);
#endif
	}
    argndx += 1;
    }
fprintf (outfil, canned4);

#ifdef DEC_MOTIF_EXTENSION
if (mm_arg_ptr->cnt)
    {
    fprintf (sdmlfil, sdmlCanned4);
    }
#endif

}




#ifdef DEC_MOTIF_EXTENSION
/*
 * Routine to write a table for a All* cotrols list
 */

void wmlOutputWmlUilSdmlControls (outfil, sdmlfil, name)
    FILE			*outfil;
    FILE			*sdmlfil;
    char			*name;

{

char		*canned1 =
".bp\n\
.ps 12\n";

char		*canned2 =
".ps 10\n\
.vs 12\n\
.LP\n\
.TS H\n\
tab(@);\n\
lB lB\n\
l l.\n\
_\n\
.sp 6p\n\
.TH\n";

char		*canned3 =
".TE\n";

char		*sdmlCanned1 =
"<PAGE>\n\
<HEAD1>(%s\\%s_sec)\n\
<X>(%s)\n";

char		*sdmlCanned2 =
"<ONLINE_POPUP>(TABLE)\n\
<LIST>(UNNUMBERED)\n";

char		*sdmlCanned3 =
"<ENDLIST>\n\
<ENDONLINE_POPUP>\n\
\n";

WmlClassCtrlDefPtr	ctlref;		/* current controls reference */
WmlClassDefPtr		clsobj;		/* current class reference */
int			ndx;		/* loop index */
int			ctlndx;		/* to access ordered vector */

/*
 * Write out header information
 */
fprintf (outfil, canned1);
fprintf (outfil, "%s\n", name);
fprintf (outfil, canned2);
fprintf (sdmlfil, sdmlCanned1, name, name, name);
fprintf (sdmlfil, sdmlCanned2);

for ( ndx=0 ; ndx<wml_obj_class_ptr->cnt ; ndx++ )
    {
    clsobj = (WmlClassDefPtr) wml_obj_class_ptr->hvec[ndx].objptr;

/*
 * No control then loop
 */
    ctlref = clsobj->controls;
    if (ctlref == NULL)
	continue;	
/*
 * If this object doesn't match what we are looking for then loop
 */
    if (strcmp (clsobj->syndef->controls->name,name) != 0)
	continue;
/*
 * Alphabetize the controls lists
 */
    wmlClearHList (mm_ctl_ptr);
    while ( ctlref != NULL )
	{
	wmlInsertInHList (mm_ctl_ptr,
			  ctlref->ctrl->syndef->name,
			  (ObjectPtr)ctlref);
	ctlref = ctlref->next;
	}

/*
 * Write out the controls list.
 */
    ctlndx = 0;
    while ( ctlndx<mm_ctl_ptr->cnt )
	{
	fprintf (outfil, "%s\n",
		 mm_ctl_ptr->hvec[ctlndx].objname);
	fprintf (sdmlfil, "<LE>%s\n",
		 mm_ctl_ptr->hvec[ctlndx].objname);
	ctlndx += 1;
	}
    fprintf (outfil, canned3);
    fprintf (sdmlfil, sdmlCanned3);
/* end the for loop */
    break; 
    }

}
#endif


