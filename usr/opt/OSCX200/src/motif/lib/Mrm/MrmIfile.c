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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: MrmIfile.c,v $ $Revision: 1.1.6.4 $ $Date: 1993/10/18 16:09:06 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */



/*
 *++
 *  FACILITY:
 *
 *      UIL Resource Manager (URM)
 *
 *  ABSTRACT:
 *
 *	This module contains the low-level file utilities
 *
 *--
 */


/*
 *
 *  INCLUDE FILES
 *
 */


#include <Mrm/MrmAppl.h>
#include <Mrm/Mrm.h>
#include <Mrm/IDB.h>

#ifdef VMS
#include <rmsdef.h>	/* RMS status definitions			*/
#include <nam.h>	/* NAM block definitions			*/
#include <fab.h>	/* FAB block definitions			*/
#include <rab.h>	/* RAB block definitions			*/
#endif

#ifndef VMS
#ifndef VAXC
#include <stdio.h>		/* Standard IO definitions		*/
#include <errno.h>
#include <fcntl.h>
#ifdef WIN32
#include <X11/xlib_nt.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#endif /* WIN32 */
#else
#define unlink remove		/* VAXC has no unlink			*/
#include <unixio.h>		/*     Ultrix like IO definitions	*/
#include <stdio.h>		/* Standard IO definitions		*/
#include <file.h>		/* FILE specific definitions		*/
#include <types.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#endif
#endif

#ifndef X_NOT_STDC_ENV
#if defined(VMS) || defined (__VMS)
#include <unixlib.h>
#else
#ifndef WIN32
#include <unistd.h>
#endif
#endif
#endif

#ifdef VMS
#ifdef CRMPSC
#include <ctype.h>
#endif
#endif

/*
 *
 *  DEFINE and MACRO DEFINITIONS
 *
 */

#ifndef VMS
#define	PMODE	0644	/* protection mode: RW for owner, all others R	*/
#define FAILURE	-1	/* creat/stat returns this			*/
#endif

/*
 *
 *  EXTERNAL VARIABLE DECLARATIONS
 *
 */


/*
 *
 *  GLOBAL VARIABLE DECLARATIONS
 *
 */


/*
 *
 *  OWN VARIABLE DECLARATIONS
 *
 */

Cardinal Idb__FU_OpenFile 

#ifndef _NO_PROTO
	(char *name,
	MrmCode access,
	MrmOsOpenParamPtr os_ext,
	IDBLowLevelFilePtr *file_id,
	char *returned_fname)
#else
	(name, access, os_ext, file_id, returned_fname)
    char		*name ;
    MrmCode		access ;
    MrmOsOpenParamPtr	os_ext ;
    IDBLowLevelFilePtr	*file_id ;
    char		*returned_fname ;
#endif

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This routine will take the file name specified and
 *	open or create it depending on the access parameter.
 *	An attempt is made to save any existing file of the
 *	same name.
 *
 *
 *  FORMAL PARAMETERS:
 *
 * 	name		the system-dependent file spec of the IDB file
 *			to be opened.
 *	accss		access type desired, read or write access.
 *	os_ext		an operating specific structure to take advantage
 *			of file system features (if any).
 *	file_id		IDB file id used in all calls to low level routines.
 *	returned_fname	The resultant file name.
 *
 *  IMPLICIT INPUTS:
 *
 *      NONE
 *
 *  IMPLICIT OUTPUTS:
 *
 *      NONE
 *
 *  FUNCTION VALUE:
 *
 *	Returns an integer:
 *
 *	MrmSUCCESS	- When access is read and open works
 *	MrmCREATE_NEW	- When access is write and open works
 *	MrmNOT_FOUND	- When access is read and the file isn't present
 *	MrmFAILURE	- When the open fails for any other reason
 *
 *  SIDE EFFECTS:
 *
 *      Opens or creates the named file and assigns a channel to it.
 *
 *--
 */


#ifdef VMS
{
/*
 * External functions
 */
int	sys$create();			/* RMS - create a file		*/
int	sys$open();			/* RMS - open a file		*/
int	sys$connect();			/* RMS - assign record stream	*/

/*
 * Local variables
 */
int	status;				/* ret status for sys services	*/
char	*fname_string;			/* resultant file name string	*/
struct	FAB *fab;			/* RMS File Acces Block		*/
struct	NAM *nam;			/* RMS NAMe block		*/
struct	RAB *rab;			/* RMS Record Acess Block	*/

#ifdef CRMPSC
#include <psldef.h>
#include <secdef.h>
#include <descrip.h>

AddressRange inadr;
AddressRange mapadr;
int 	flags;
int 	prot = 0xEEEE;		/* No write, delete, execute access */
struct dsc$descriptor_s mapfile; 
char    section_name[NAM$C_MAXRSS+1];
int     uidname_length;
int	i;

if (access == URMReadAccess){
/*
 *  First try $MGBLSC to see if the
 *  file has been mapped as a global section.  If not, use $CRMPSC
 *  to create a local section.
 */
inadr.range_start = inadr.range_end = 1;
mapadr.range_start = mapadr.range_end = 0;

#ifdef MGBLSC
/*
 *  Don't do this until we figure out if I18N is going to work.
 *  The problem is that once the .UID file is mapped at 
 *  DECwindows startup, that .UID file will be used regardless
 *  of the user's language setting.
 */
flags = SEC$M_EXPREG | SEC$M_SYSGBL;   
/*
 *  Append _UID to the uid file name to generate the logical name to
 *  use in the lookup.
 *  NOTE:  the uid name passed in can consist of any file
 *         specification.  This code will only successfully map
 *         a global section if only the filename and optionally
 *         DECW$SYSTEM_DEFAULTS: and/or .UID are entered in the
 *         name parameter.
 */
uidname_length = strlen(name);
for (i = 0; i < uidname_length; i++)
    section_name[i] = _toupper(name[i]);
if (uidname_length > 21 && 
    strncmp(&section_name[0], "DECW$SYSTEM_DEFAULTS:", 21) == 0)
    {
        for (i = 0; i < uidname_length; i++)
	    section_name[i] = section_name[i+21];
        uidname_length -= 21;
    }
if (uidname_length > 4 && 
    strncmp(&section_name[uidname_length-4], ".UID", 4) == 0)
    section_name[uidname_length-4] = '_';
else
{
    strncpy(&section_name[uidname_length], "_UID", 4);
    uidname_length += 4;
}
mapfile.dsc$w_length  = uidname_length;
mapfile.dsc$b_dtype   = DSC$K_DTYPE_T;     /* 8 bit chars */
mapfile.dsc$b_class   = DSC$K_CLASS_S;     /* fixed length */
mapfile.dsc$a_pointer = section_name;
status = sys$mgblsc(&inadr, &mapadr, PSL$C_USER, flags, &mapfile, 0, 0);

if ((status & 1) != 1)
    /*
     *  If we can't get it global, try local (later...)
     */
    flags = SEC$M_EXPREG;
else
{
/*
 * Save the resultant file name so the user can examine it
 */
    strncpy (returned_fname, section_name, uidname_length);
    returned_fname[uidname_length] = 0;
    *file_id = XtMalloc (sizeof (IDBLowLevelFile));
    if (*file_id==0)
        return MrmFAILURE;

    (*file_id)->rab = 0;
    (*file_id)->channel = -1;
    (*file_id)->mapadr = mapadr;
    return MrmSUCCESS;
}
#else
flags = SEC$M_EXPREG;
#endif /* MGBLSC */
}
#endif /* CRMPSC */

/*
 * Allocate the RMS blocks we need
 */
fab = XtMalloc (FAB$C_BLN);
if (fab == 0)
    return MrmFAILURE;

nam = XtMalloc (NAM$C_BLN);
if (nam == 0)
    {
    XtFree (fab);
    return MrmFAILURE;
    }


#ifdef CRMPSC
rab = 0;
if (access != URMReadAccess){
#endif /* CRMPSC */
    rab = XtMalloc (RAB$C_BLN);
    if (rab == 0)
        {
        XtFree (fab);
        XtFree (nam);
        return MrmFAILURE;
        }
#ifdef CRMPSC
}
#endif /* CRMPSC */

fname_string = XtMalloc (NAM$C_MAXRSS+1);
if (fname_string == 0)
    {
    XtFree (fab);
    XtFree (nam);
    XtFree (rab);
    return MrmFAILURE;
    }

/*
 * Initialize the FAB and NAM blocks
 */
*fab = cc$rms_fab;
*nam = cc$rms_nam;
#ifdef CRMPSC
if (access != URMReadAccess)
#endif /* CRMPSC */
    *rab = cc$rms_rab;

/*
 * Enter our values in to the FAB, RAB, and NAM blocks
 */
fab->fab$l_nam = nam;		/* tell the fab where the nam is	*/
nam->nam$l_esa = fname_string;	/* space for the expanded file name	*/
nam->nam$l_rsa = fname_string;	/* space for the resultant file name	*/
nam->nam$b_rss = NAM$C_MAXRSS;
nam->nam$b_ess = NAM$C_MAXRSS;

#ifdef CRMPSC
if (access == URMReadAccess){
    fab->fab$l_fop = FAB$M_UFO;
    fab->fab$b_shr = FAB$M_UPI;
    fab->fab$w_mrs = 512;
    fab->fab$b_rtv = -1; 
}
else {
#endif /* CRMPSC */
    fab->fab$l_alq = IDBRecordSize/128;	/* Pre-allocate some blocks	*/
    fab->fab$w_mrs = IDBRecordSize;	/* Default record size 		*/
    fab->fab$w_deq = IDBRecordSize/256;	/* Default extend amount	*/
    fab->fab$w_gbc = 0;			/* no global buff (use our own)	*/
#ifdef CRMPSC
}
#endif /* CRMPSC */

fab->fab$l_fna = name;			/* the file name		*/
fab->fab$b_fns = strlen (name);
fab->fab$b_org = FAB$C_SEQ;		/* sequential organization	*/
fab->fab$b_rat = FAB$M_CR;		/* Carriage control		*/ 
fab->fab$b_rfm = FAB$C_FIX;		/* ? record format		*/
if (os_ext != 0)
    if (os_ext->version != MrmOsOpenParamVersion)
	return MrmFAILURE;
    else {
	fab->fab$l_dna = os_ext->default_fname;		/* defaults	*/
	fab->fab$b_dns = strlen (os_ext->default_fname);
	nam->nam$l_rlf = os_ext->nam_flg.related_nam;	/* related NAM	*/
	}

#ifdef CRMPSC
if (access != URMReadAccess){
#endif /* CRMPSC */
    rab->rab$l_fab = fab;		/* tell the rab where the fab is	*/
    rab->rab$l_rop = RAB$M_BIO | 	/* Block IO only and			*/
        		 RAB$M_NLK;	/* no record locking			*/
    rab->rab$b_mbf = 1;			/* One buffer (we'll buffer the rest)	*/
    rab->rab$w_rsz = IDBRecordSize;	/* record size				*/
#ifdef CRMPSC
}
#endif /* CRMPSC */

if (access == URMWriteAccess)
    {
/*
 * this file is to be opened for write access
 */
    fab->fab$b_fac = FAB$M_BIO |FAB$M_PUT | /* BLOCK IO, Read & Write	*/
		     FAB$M_GET;
    fab->fab$b_shr = FAB$M_UPI |	/* user provided interlock	*/
		     FAB$M_SHRGET |	/* we need shrget		*/
		     FAB$M_SHRPUT;	/* we need shrput too		*/
    fab->fab$l_fop = FAB$M_OFP;		/* Output file parse		*/
    status = sys$create (fab);			/* Create the file	*/
    if ((status & 1) != 1)
	{
	strncpy (returned_fname, fname_string, nam->nam$b_esl);
	returned_fname[nam->nam$b_esl] = 0;
	XtFree (fab);
	XtFree (nam);
	XtFree (rab);
	XtFree (fname_string);
	return MrmFAILURE;
	}
    }
    else if (access == URMReadAccess)
	{
/*
 * Else this file is to opened for read access
 */
#ifndef CRMPSC
	fab->fab$b_fac = FAB$M_BIO |	/* BLOCK IO & Read		*/
		     FAB$M_GET;
	fab->fab$b_shr = FAB$M_UPI |	/* user provided interlock	*/
		     FAB$M_SHRGET;	/* we need shrget too		*/
#endif /* CRMPSC */
	status = sys$open (fab);		/* Open the file	*/
	if ((status & 1) != 1)
	    {
	    strncpy (returned_fname, fname_string, nam->nam$b_esl);
	    returned_fname[nam->nam$b_esl] = 0;
	    XtFree (fab);
	    XtFree (nam);
	    XtFree (fname_string);
	    if (status == RMS$_FNF)
		return MrmNOT_FOUND;
	    else
		return MrmFAILURE;
	    }
	}
	else return MrmFAILURE;	/* invalid access type		*/

#ifdef CRMPSC
if (access == URMReadAccess) {
    /*
     *  Use $CRMPSC to create a local section.
     */
    status = sys$crmpsc(&inadr, &mapadr, PSL$C_USER, flags,
		    0, 0, 0, fab->fab$l_stv, 0, 0, prot, 64);

    if ((status & 1) != 1) {
        /*
         *  Close the file and don't use it.
         *  (since the file was opened UFO, we merely
         *   DEASSIGN the channel)
         */
        sys$dassgn( fab->fab$l_stv );
        strncpy (returned_fname, fname_string, nam->nam$b_esl);
        returned_fname[nam->nam$b_esl] = 0;
	XtFree (fab);
	XtFree (nam);
	XtFree (fname_string);
        return MrmFAILURE;
        }
    }
else {
#endif  /* CRMPSC */
/*
 * Attach the record stream to the file
 */
    status = sys$connect (rab);
if ((status & 1) != 1)
    {
    strncpy (returned_fname, fname_string, nam->nam$b_esl);
    returned_fname[nam->nam$b_esl] = 0;
    XtFree (fab);
    XtFree (nam);
    XtFree (rab);
    XtFree (fname_string);
    return MrmFAILURE;
    }
#ifdef CRMPSC
}
#endif /* CRMPSC */

/*
 * Save the resultant file name so the user can examine it
 */
strncpy (returned_fname, fname_string, nam->nam$b_rsl);
returned_fname[nam->nam$b_rsl] = 0;

/*
 * Return a pointer to the rab (which points to the fab which points to the nam);
 */

*file_id = XtMalloc (sizeof (IDBLowLevelFile));
if (*file_id==0)
    {
    XtFree (fab);
    XtFree (nam);
    XtFree (rab);
    XtFree (fname_string);
    return MrmFAILURE;
    }

(*file_id)->rab = rab;
#ifdef CRMPSC
if (access == URMReadAccess){
    (*file_id)->channel = fab->fab$l_stv;
    (*file_id)->mapadr = mapadr;
    XtFree (nam->nam$l_esa);
    XtFree (nam);
    XtFree (fab);
}
#endif /* CRMPSC */

if (access == URMWriteAccess)
    return MrmCREATE_NEW;
else
    return MrmSUCCESS;

}				/* end of routine Idb__FU_OpenFile	*/
#endif


#ifndef VMS
{
/*
 * External routines
 */

/*
 * Local variables
 */
int		file_desc;		/* 'unix' file descriptor		*/
int		length;			/* the length of the above string	*/
IDBLowLevelFile *a_file;		/* pointer to the file_id		*/

/* Fill in the result name with the name specified so far		    */
length = strlen (name);
strcpy (returned_fname, name);
returned_fname[length] = 0;

/* Check if this file is to be opened for read or write access		    */
if (access == URMWriteAccess)
    {
#ifndef WIN32
    file_desc = open (name, O_RDWR, PMODE);
#else
    file_desc = open (name, _O_RDWR | _O_BINARY, PMODE);
#endif
    if (file_desc != FAILURE)		/* there exists a file by this
					   name already			*/
      {
	if (os_ext == 0)
	  return MrmFAILURE;		/* we need to know if we can
					   clobber the existing file	*/
	else if (!os_ext->nam_flg.clobber_flg)
	  return MrmEXISTS;		/* no clobber. return Exists	*/
	else if (os_ext->version != MrmOsOpenParamVersion)
	  return MrmFAILURE;
	close (file_desc);		/* we care not what close returns*/
      }

    file_desc = creat (name,PMODE);
    if (file_desc == FAILURE)		/* verify that worked		*/
	return MrmFAILURE;

    close (file_desc);		/* we care not what close returns	*/
#ifndef WIN32
    file_desc = open (name, O_RDWR, PMODE);
#else
    file_desc = open (name, _O_RDWR | _O_BINARY, PMODE);
#endif

    if (file_desc == FAILURE)		/* verify that worked		*/
	return MrmFAILURE;
    }


/* Else this file is to opened for read access				    */
else if (access == URMReadAccess)
    {
#ifndef WIN32
    file_desc = open (name, O_RDONLY, PMODE);
#else
    file_desc = open (name, _O_RDONLY | _O_BINARY, PMODE);
#endif

    /* verify that worked						    */
    if (file_desc == FAILURE)
	{
	if ( errno == EACCES )
	    return MrmFAILURE;
	else
	    return MrmNOT_FOUND;
	}
    }

/* Not URMReadAccess or URMWriteAccess, so return invalid access type	    */
else
    return MrmFAILURE;


/*
 * now all we have to do is set up the IDBFile and return the
 * proper success code.
 */

*file_id = (IDBLowLevelFilePtr)
    XtMalloc(sizeof (IDBLowLevelFile));
if (*file_id==0)
    return MrmFAILURE;

a_file = (IDBLowLevelFile *) *file_id;

a_file->name = XtMalloc (length+1);
if (a_file->name==0)
    {
    XtFree ((char*)*file_id);
    return (MrmFAILURE);
    }

a_file->file_desc = file_desc;
strcpy (a_file->name, name);
a_file->name[length] = 0;

if (access == URMWriteAccess)
    return (MrmCREATE_NEW);
else
    return (MrmSUCCESS);

}
#endif



Cardinal Idb__FU_CloseFile 
#ifndef _NO_PROTO
    (IDBLowLevelFile	*file_id ,
     int		delete)
#else

(file_id, delete)
    IDBLowLevelFile	*file_id ;
    int			delete ;
#endif
/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This routine will close the file and free any allocated storage
 *
 *
 *  FORMAL PARAMETERS:
 *
 *	file_id		IDB file id
 *	delete		delete the file if == true
 *
 *  IMPLICIT INPUTS:
 *
 *      the file name and channel from the IDBFile record
 *
 *  IMPLICIT OUTPUTS:
 *
 *      NONE
 *
 *  FUNCTION VALUE:
 *
 *	MrmSUCCESS	- When the file is closed [and deleted] successfully
 *	MrmFAILURE	- When the close fails
 *
 *  SIDE EFFECTS:
 *
 *      Closes the file, deassigns the channel and possible deletes the file.
 *
 *--
 */

#ifdef VMS
{
/*
 * External functions
 */
int	sys$close();			/* RMS close a file		*/
int	sys$erase();			/* RMS delete a file		*/

/*
 * Local variable
 */
int	status;				/* ret status for sys services	*/
struct	NAM	*nam;			/* NAM Block pointer		*/
struct	FAB	*fab;			/* FAB Block pointer		*/
struct	RAB	*rab;			/* RAB Block pointer		*/
char	aname [NAM$C_MAXRSS];		/* Resultant string buffer	*/


rab = (struct RAB *) file_id->rab;
#ifdef CRMPSC
if (rab == NULL)
    if (file_id->channel == -1) {}
    else
    {
	AddressRange mapadr;
	sys$deltva( &file_id->mapadr, &mapadr, PSL$C_USER );
        sys$dassgn( file_id->channel );
    }
else {
#endif /* CRMPSC */
    fab = rab->rab$l_fab;
    nam = fab->fab$l_nam;

    status = sys$close (fab);
    if ((status & 1) != 1) 
        return MrmFAILURE;

    if (delete) {

        fab->fab$l_fna = nam->nam$l_rsa;	/* use the name we opened for	*/
        fab->fab$b_fns = nam->nam$b_rsl;	/* the erase.			*/

        nam->nam$l_rsa = aname;
        nam->nam$b_rss = NAM$C_MAXRSS;

        status = sys$erase (fab);

        if ((status & 1) != 1) 
            return MrmFAILURE;
        }

    XtFree (nam->nam$l_esa);
    XtFree (nam);
    XtFree (rab);
    XtFree (fab);
#ifdef CRMPSC
}
#endif /* CRMPSC */

XtFree (file_id);
return MrmSUCCESS;

}				/* end of routine Idb__FU_CloseFile	*/
#endif



#ifndef VMS
{
/*
 * Local variables
 */
int	status;				/* ret status for sys services	*/


status = close (file_id->file_desc);
if (status != 0)
    return MrmFAILURE;

if (delete) {
    status = unlink (file_id->name);
    }

XtFree (file_id->name);
XtFree ((char*)file_id);
return MrmSUCCESS;

}				/* end of routine Idb__FU_CloseFile	*/
#endif


Cardinal Idb__FU_GetBlock 

#ifndef _NO_PROTO
    (IDBLowLevelFile	*file_id,
    IDBRecordNumber	block_num,
    char		*buffer)
#else
	(file_id, block_num, buffer)
    IDBLowLevelFile	*file_id ;
    IDBRecordNumber	block_num ;
    char		*buffer ;
#endif

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This function reads in the desired record into the given
 *	buffer.
 *
 *  FORMAL PARAMETERS:
 *
 *	file_id		the IDB file identifier
 *	block_num	the record number to retrieve
 *	buffer		pointer to the buffer to fill in
 *
 *  IMPLICIT INPUTS:
 *
 *      NONE
 *
 *  IMPLICIT OUTPUTS:
 *
 *      NONE
 *
 *  FUNCTION VALUE:
 *
 *	MrmSUCCESS	operation succeeded
 *	MrmNOT_FOUND	entry not found
 *	MrmFAILURE	operation failed, no further reason
 *
 *  SIDE EFFECTS:
 *
 *      The buffer is filled in.  Should the $READ fail the buffer's
 *	content is not predictable.
 *
 *--
 */

#ifdef VMS
{
/*
 * Local variables
 */
struct RAB	*rab;			/* record access block		*/
int		status;			/* ret status for sys services	*/


if (block_num < IDBHeaderRecordNumber)
    return MrmFAILURE;

rab = (struct RAB *) file_id->rab;

rab->rab$l_bkt =
    (block_num-1)*IDBRecordSize/512+1;	/* record to read		*/
rab->rab$l_ubf = buffer;		/* place data read here		*/
rab->rab$w_usz = IDBRecordSize;		/* buffer size/transfer amount	*/

status = sys$read (rab);

if ((status & 1) != 1)
    if (status == RMS$_EOF)
	return MrmNOT_FOUND;
    else
	return MrmFAILURE;
else
    return MrmSUCCESS;
}				/* end of routine Idb__FU_GetBlock	*/
#endif



#ifndef VMS
{
/*
 * Local variables
 */
int	number_read;		/* the number of bytes actually read	*/
int	fdesc ;			/* file descriptor from lowlevel desc */


fdesc = file_id->file_desc ;
lseek (fdesc, (block_num-1)*IDBRecordSize, 0);
number_read = read (file_id->file_desc, buffer, IDBRecordSize);

if (number_read != IDBRecordSize) 
    return MrmFAILURE;
else
    return MrmSUCCESS;
}				/* end of routine Idb__FU_GetBlock	*/
#endif


Cardinal Idb__FU_PutBlock 

#ifndef _NO_PROTO
    (IDBLowLevelFile	*file_id,
    IDBRecordNumber	block_num,
    char		*buffer)
#else
        (file_id, block_num, buffer)
    IDBLowLevelFile	*file_id ;
    IDBRecordNumber	block_num ;
    char		*buffer ;
#endif

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This function writes the data in the givin buffer into
 *	the desired record in the file.
 *
 *  FORMAL PARAMETERS:
 *
 *	file_id		the IDB file identifier
 *	block_num	the record number to write
 *	buffer		pointer to the buffer to read from
 *
 *  IMPLICIT INPUTS:
 *
 *      NONE
 *
 *  IMPLICIT OUTPUTS:
 *
 *      NONE
 *
 *  FUNCTION VALUE:
 *
 *	Returns an integer by value:
 *
 *	MrmSUCCESS	operation succeeded
 *	MrmFAILURE	operation failed, no further reason
 *
 *  SIDE EFFECTS:
 *
 *	the file is modified.
 *
 *--
 */

#ifdef VMS
{
/*
 * Local variables
 */
int	status;				/* ret status for sys services	*/
struct	RAB *rab;			/* RMS record access block	*/


if (block_num < IDBHeaderRecordNumber)
    return MrmFAILURE;

rab = (struct RAB *) file_id->rab;

rab->rab$l_bkt =
    (block_num-1)*IDBRecordSize/512+1;	/* record to write		*/
rab->rab$l_rbf = buffer;		/* get data to write from here	*/
rab->rab$w_rsz = IDBRecordSize;		/* buffer size/transfer amount	*/

status = sys$write (rab);

if ((status & 1) != 1)
    return MrmFAILURE;
else
    return MrmSUCCESS;
}				/* end of routine Idb__FU_PutBlock	*/
#endif



#ifndef VMS
{
/*
 * Local variables
 */
int	number_written;		/* the # of bytes acctually written	*/
int	fdesc ;			/* file descriptor from lowlevel desc */


fdesc = file_id->file_desc ;
lseek (fdesc, (block_num-1)*IDBRecordSize, 0);
number_written = write (file_id->file_desc, buffer, IDBRecordSize);

if (number_written != IDBRecordSize)
    return MrmFAILURE;
else
    return MrmSUCCESS;
}				/* end of routine Idb__FU_PutBlock	*/
#endif


