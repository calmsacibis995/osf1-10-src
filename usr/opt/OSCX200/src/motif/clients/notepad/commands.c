/*  module:  commands.c "V1-002"
 *
 *  Copyright (c) Digital Equipment Corporation, 1990
 *  All Rights Reserved.  Unpublished rights reserved
 *  under the copyright laws of the United States.
 *  
 *  The software contained on this media is proprietary
 *  to and embodies the confidential technology of 
 *  Digital Equipment Corporation.  Possession, use,
 *  duplication or dissemination of the software and
 *  media is authorized only pursuant to a valid written
 *  license from Digital Equipment Corporation.
 *
 *  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 *  disclosure by the U.S. Government is subject to
 *  restrictions as set forth in Subparagraph (c)(1)(ii)
 *  of DFARS 252.227-7013, or in FAR 52.227-19, as
 *  applicable.
 *
 *
 * FACILITY:
 *	Notepad
 *
 * ABSTRACT:
 *      This module contains the action routines that support the user
 * 	interface.  
 *
 * NOTES:
 *	Summary of module.
 *
 * REVISION HISTORY:
 *      1987-Oct-23  J. M. Gringorten    V1-001  JMG0001  Initial version.
 *
 * [cjh]  07-Jul-93
 *        - Deleted old revision history.
 *        - Inserted "CopyFile()" (effect: backup copy of file-being-edited,
 *          IS now a COPY -- independant and not-linked -- thus, when
 *          "editInPlace", an X-resource, is turned on, hard-linked
 *          file(s)-being-edited remain properly hard-linked).
 *    ref:  cld UVO02102 (already closed, but stating this debug to be done
 *          later.  Also, internal ootb-notes #69,74,75.)
 *
 * [cjh]  09-Jul-93
 *        - In DoSave(): Get mode of file-being-edited via stat(), and set
 *          the temp-file's mode to this just before the temp-file BECOMES
 *          the file-being-edited.
 *    ref:  Related to last fix.  Also, internal ootb-notes #62.
 *
 */

/* [bl] 25-Oct-93 (Refer Notes ootb_bugs 128)
 *	  - DoCopy(): XmClipboardStartCopy() is placed inside a while loop to
 *	    make sure that the process iterates till XmClipStartCopy 
 *	    returns ClipboardSuccess.
 *
 *   	    To test if selection has been made, XmTextGetSelection() is used
 *	    instead of the low level function 
 *	    "((*Psource->GetSelection)(Psource, &left, &right))
 *
 *	  - copyToClipboard(): XmClipboardCopy() and XmClipboardEndCopy are 
 *	    each placed inside a while loop to make sure that each return 
 *	    ClipboardSuccess when they come out of the loop.
 *
 *         - The second argument to XmClipboardCopy() is changed from 
 *	    XtWindow(textwindow) to XtWindow(toplevel) keeping with the
 *	    rule that the window passed to each of the motif clipboard
 *	    functions must be the same. 
 *
 *   	  - DoPaste(): The lower level function call ,
 *	     "tstatus = (*Psource->Replace)
 *                         (textwindow, NULL, &pos, &pos, &block, True);"  
 *          is replaced with the motif convenience function
 *	    "XmTextInsert(textwindow, pos)"
 *         
 *        -All occurences of 'CurrentTime' s are replaced with the last event time.
 */
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: [commands.c,v 1.4 91/08/17 05:58:12 rmurphy Exp ]$";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

/* Include files. */
#include "notepad.h"
#include <Xm/CutPaste.h>
#include <Xm/Text.h>

/*	Module specific declarations	*/



DoUndo ()
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    /* NEED: if(undo_counter == 0) {clearModify( <filename> );}  */
    if(PSUndo(Psource) == FALSE)
        Feep();
}                                /* end routine DoUndo */

DoRedo ()
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    if(PSRedo(Psource) == FALSE)
        Feep();
}

setLoadedFile (name)
    char *name;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    char wmName[1024];

    if(loadedfile) XtFree(loadedfile);
    loadedfile = XtMalloc(strlen(name)+1);
    strcpy(loadedfile, name);
    clearModified(name);
}

setSavedFile (name)
    char *name;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    if(savedfile) XtFree(savedfile);
    savedfile = XtMalloc(strlen(name)+1);
    strcpy(savedfile, name);
}

char *makeTempName (name)
    char *name;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    extern char  *mktemp();
    char         *tempname = XtMalloc(1024);

    sprintf(tempname, "%s.XXXXXX", name);
    return(mktemp(tempname));
}

char *makeBackupName (name)
    char *name;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    char *backupName = XtMalloc(1024);

    sprintf(backupName, "%s%s%s",backupNamePrefix, name, backupNameSuffix);
    return (backupName);
}

int CopyFile (theOrig, theCopy)
    char *theOrig;
    char *theCopy;
/*
 * Function:
 * 	Copies a file (theOrig) to a backup file (theCopy)
 *	
 * Inputs:
 * 	theOrig - ptr to name of from-file
 * 	theCopy - ptr to name of to-file 
 *	
 * Outputs:
 * 	ERR_NOERR   - copy operation succeeded
 * 	ERR_NOORIG  - theOrig doesn't exist
 * 	ERR_NOCOPY  - theOrig exists, but copy failed
 *	
 * Notes:
 * 	There is no UNIX system call to provide a copy file function so
 * 	is implemented as reads of theOrig and writes to theCopy. 
 * 	Note that the system(cp) call is NOT used since it does
 * 	not return the status of the cp command. 
 */
{
    register FILE  *src_stream, *dst_stream;
    size_t          buf_size    = BUFSIZ;
    int             err_status  = ERR_NOERR; /* Unless set otherwise. */
    void           *buffer;
    size_t          bytes_read;

    /*
     * Open source and destination files
     */
    src_stream = fopen(theOrig, "rb");    /* open source file for read on */
    if (!src_stream)                      /* if open failed */
    {   err_status = ERR_NOORIG;          /* set error code */
        goto exit;                        /* skip to end */
    }

    dst_stream = fopen(theCopy, "wb");    /* open dest for write */
    if (!dst_stream)                      /* if open failed */
    {   err_status = ERR_NOCOPY;          /* set error code */
        goto cleanup_src;                 /* skip to end */
    }

    /*
     * Allocate buffer for the copy.  If allocation fails, close and
     * delete target file, then skip to end to complete cleanup
     */
    buffer = (void *)XtMalloc(buf_size);  /* allocate buffer */
    if (!buffer)                          /* if allocation failed */
    {
        fclose(dst_stream);
        unlink(theCopy);
        goto cleanup_src;                 /* skip to end */
    }

    /*
     * Do the copy. (The last read is done when the number of elements
     * (BytesRead) read by fread is smaller than the buffer size.)
     */
    do
    {
        bytes_read = fread(buffer,1,buf_size,src_stream);
        if ferror(src_stream)
        {   err_status = ERR_NOCOPY;      /* set error code */
            clearerr(src_stream);         /* clear ferror */
            goto cleanup_all;             /* skip to end */
        }
        if (!bytes_read)
        {  if feof(src_stream) break;     /* if eof input it's not an error */
           err_status = ERR_NOCOPY;       /* set error code */
           goto cleanup_all;              /* skip to end */
        }
        if (fwrite(buffer,1,bytes_read, dst_stream) != bytes_read)
        {   err_status = ERR_NOCOPY;      /* set error code */
            goto cleanup_all;             /* skip to end */
        }
    }
    while (bytes_read == buf_size);

    /*
     * Close destination files
     */
    if (fclose(dst_stream))               /* returns 0 if successful */
        err_status = ERR_NOCOPY;          /* can't close dst file */

    /*
     * Cleanups and exit
     */
    cleanup_all:
        XtFree((void *)buffer);
    cleanup_src:
        fclose(src_stream);
    exit:
        return(err_status);
}

/*
error()
{
        extern int errno, sys_nerr;
        extern char *sys_errlist[];
 
        if (errno > 0 && errno < sys_nerr)
                tvcprintf(stderr, "(%s)\n", sys_errlist[errno]);
 
}
*/

#ifndef R_OK
#define R_OK    4/* test for read permission */
#define W_OK    2/* test for write permission */
#define X_OK    1/* test for execute (search) permission */
#define F_OK    0/* test for presence of file */
#endif

DoSave (filename)
    char *filename;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    char            *parseFilename();
    extern void      modifiedHandler();
    char            *backupFilename, *tempName;
    XmTextPosition   pos, end;
    XmTextBlock      text;
    XmTextBlockRec   t;
    FILE            *outStream;
    int              outfid;
    char             strippedName[256];
    char             tempDir[256];
    Arg              a[4];
    int              n = 0;

    text = &t;
    if(modified && !strlen(filename)){
	if(loadedfile == NULL || !strlen(loadedfile)){
	    showSimpleDialog(NULL, &saveDialog, 0);
	    return;	
    	} else {
	    filename = loadedfile;
	}
    } else {
	/*caution box describing that you'll write over an existing file
	  by that same name
	*/
    }
#ifdef VMS
    /*
     * strip off the VMS version number for the write file operation
     */
    if(strlen(filename))
    {
        char *pName = parseFilename(filename,tempDir);
	if(!pName)
	{
	    ShowMessage(&Stuff._badFilename, "badFilename");
	    return;
        }
        sprintf(strippedName,"%s%s", tempDir, pName );
	filename = strippedName;
    }
#endif


/* If there are no new edits to change, display a message to the
 * user and return
 */
    if((!modified && savedfile && !strcmp(savedfile, filename)) ||
       (!modified && !strlen(filename)))
    {
        if(exitWithSave)
	{
            closeInputFile();
	    exit(NormalStatus);
        }
 	/* message box explaing that there's nothing to save */
	ShowMessage(&Stuff._nothingToSave, "nothingToSave");
	return;
    } 
/* 
 * If file is not writeable, return
 */
    if(access(filename, F_OK) == 0){
       if(access(filename, W_OK) < 0) {
	    ShowMessage(&Stuff._noWriteMessage, "noWriteMessage");
	return;
        }
    }
    backupFilename = tempName = NULL; 
#ifndef VMS

    if(access(filename, F_OK) == 0)
    {
        /* Get current file permissions for file-being-edited. We cached
	 * these permission when the file was opened but the users
	 * could have "chmod" just before saving -- outside of notepad.
 	 * These current permissions will be applied to the final, saved file
         */
        struct stat  fileStats;
        if( stat(filename, &fileStats) == 0)  /* success returns 0  */
        {
            /* Update permissions in notepad's global structure. */
            FileMode = fileStats.st_mode;
        }
        else
        {
	    fprintf(stderr,"Using old permissions: Cannot stat: %s", filename);
        }
        if((!backedup) && (strlen(loadedfile)) && 
			(!strcmp(filename, loadedfile)))
        {
            backupFilename = makeBackupName(filename);
            unlink(backupFilename);
            if(CopyFile(filename, backupFilename))
            {
		ShowMessage(&Stuff._noBackupMessage, "noBackupMessage");
	        return;
	    }
	    PseudoDiskSourceDestroy(rsource); 
	    rsource = PseudoDiskSourceCreate(backupFilename);
	    PSsetROsource(Psource, rsource); 
            backedup = 1;
        }
/* If editInPlace is set, write right on top of the original file */
        if(editInPlace)
        {
	    if(!(outStream = fopen(filename, "w")))
            {
	        fprintf(stderr,"File is not writable: %s", filename);
	        return;
	    }
        }
        else
        {
	    /* Save to a temp file and then rename it to the original
             * NOTE: When the temp file is created below, creat() does
	     * not use the permissions specified in FileMode. Instead
	     * it applies the user's umask to FileMode. 
             * Thus, chmod(), which does no masking, will be
             * called below just prior to saving in order to restore
	     * the original file's permissions.
             */
	    tempName = makeTempName(filename);
	    if((outfid = creat(tempName, FileMode)) < 0)
            {
	        ShowMessage(&Stuff._noTempMessage, "noTempMessage");
	        return;
            }
	    outStream = fdopen(outfid, "w");
        } 
    }
    else
    {
        if(!(outStream = fopen(filename, "w")))
        {
	    ShowMessage(&Stuff._noWriteMessage, "noWriteMessage");
            return;
        }
    }
#endif
#ifdef VMS
    if(!(outStream = fopen(filename, "w"))){
	    ShowMessage(&Stuff._noWriteMessage, "noWriteMessage");
         return;
    }
#endif  
    /*
     *  WRITE ALL THE BITS OUT TO THE OUTPUT STREAM
     */
    end = (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);
    for(pos = 0; pos < end; ){
	pos = (*Psource->ReadSource)(Psource, pos, end, text);
	if(text->length == 0)
	    break;
	if(fwrite(text->ptr, 1, text->length, outStream) < text->length){
	    ShowMessage(&Stuff._writeErrorMessage, "writeErrorMessage");
	    break;
	}
    }
    fclose(outStream);
#ifndef VMS
    if(!editInPlace && tempName && strlen(tempName))
    {
        /* Update temp-file's permissions using chmod(). The mode change
         * is exact -- NO MASKING as with creat() above -- thus,
         * permissions of the file-being-edited are effectively preserved. 
         */
	if( chmod(tempName, FileMode) != 0)
        {
	    fprintf(stderr,"File permissions may change for: %s", tempName);
	    Feep();
        }
        /* Rename temp-file to file-being-edited.  Contents & gid
         * change, but permissions(mode) are preserved.
         */
	if(rename(tempName, filename) < 0)
        {
	    fprintf(stderr,"Error writing file. Edits will be left in: %s",
		tempName);
	    Feep();
	}
    }
    if(!enableBackups && backupFilename)
	if(unlink(backupFilename) < 0)
	    fprintf(stderr,"Error deleting backupfile: %s", backupFilename);
#endif
    setSavedFile(filename);
    /*
     * message to confirm success in saving
     */
    if(strlen(loadedfile))
	clearModified(loadedfile);
    else
	clearModified(filename);
#ifndef VMS
    if(backupFilename)
	XtFree(backupFilename);
    if(tempName)
	XtFree(tempName);
#endif
    PSbreakInput(Psource);
    PSsetApplicationCallback(Psource, modifiedHandler);
    closeInputFile();
    if(exitWithSave)
    {
	exit(NormalStatus);
    }
    else
    {
	XmTextPosition locs[64], places[64];
	int i;
	View *vp;
	for(vp = Stuff.viewHead, i=0; vp; vp = vp->flink, i++)
	{
	    _XmTextDisableRedisplay(vp->widget, TRUE);
	    places[i] = XmTextGetTopCharacter(vp->widget);
	    locs[i] = XmTextGetInsertionPosition(vp->widget);  
	}
        DoLoad(filename);
	for(vp = Stuff.viewHead, i=0; vp; vp = vp->flink, i++)
	{
	    XmTextSetTopCharacter(vp->widget, places[i]);
	    XmTextSetInsertionPosition(vp->widget, locs[i]);
	    XmTextSetCursorPosition(vp->widget, locs[i]);
    	    _XmTextEnableRedisplay(vp->widget);
        }
    }
}

DoLoad (filename)
    char *filename;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    int          n;
    Arg          a[6];
    struct stat  stats;

    if(!filename)
	filename = "";
    if(lock)
	closeInputFile();
    if(strlen(filename))
    {
	if(access(filename, R_OK) == 0)
	{
	    stat(filename, &stats);
	    FileMode = stats.st_mode;
            if(!read_only && access(filename, W_OK) != 0)
	    {
		read_only = TRUE;	
		ShowMessage(&Stuff._readOnlyMessage, "readOnlyMessage");
	    } 

	    if(!read_only)
	    {
		lock = lockFile(filename);
		if(!lock)
		{
		    ShowMessage(&Stuff._reopenMessage, "reopenMessage");
		    read_only = TRUE;
		}
            } 
        }
	else
	{
	    /* file not readable.  does it exist? */
	    if(access(filename,F_OK) == 0)
	    {
 	        /*  the file isn't accessable */
   	        Feep();
		ShowMessage(&Stuff._noAccessMessage, "noAccessMessage");
 	        return;
	    }
	}
    }
    else
    {
	ShowMessage(&Stuff._noFilenameMessage, "noFilenameMessage");
	return;
    }
    setSources(filename);
    backedup = FALSE;
    setLoadedFile(filename);
    read_only = FALSE;
}

/* (braindamaged) clipboard support */





static copyToClipboard (sel, count)
    char  *sel;
    int    count;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    int result;

   /* loop till XmClipboardCopy returns ClipboardSuccess */
    while((result = XmClipboardCopy(CurDpy, XtWindow(toplevel),    
	 clipItemID, "STRING", sel, count, NULL, &clipDataID)) != ClipboardSuccess)
      ;

   /* loop till XmClipboardEndCopy returns ClipboardSuccess */
    while((result = XmClipboardEndCopy(CurDpy, XtWindow (textwindow),
		 clipItemID)) != ClipboardSuccess)
      ;
}

void DoCopy (w, client_data, cb)
    Widget w;                   	/* Widget id */
    XtPointer client_data;		/* Our callback data */
    XmPushButtonCallbackStruct *cb;	/* Widget's data */
/*
 * Function: Callback routine to Copy selected text to the clipboard
 *
 * Inputs:
 * 	w - widget id
 *
 * Outputs:
 *
 * Notes:
 */
{
    char *sel;
    int result;
    XmTextPosition left, right;
    XButtonEvent *be = (XButtonEvent *)cb->event;


/* See if text is selected */

 if((sel = XmTextGetSelection(textwindow)) != NULL)   			
{

/* Start the copy. Sets up storage and data strcutures to receive data */
/*Loop until XmClipboardStartCopy returns ClipboardSuccess */
    while((result = XmClipboardStartCopy
            (
                CurDpy, 			/* Display */
		XtWindow(toplevel),		/* Window id */
			/* Label identifying clipboard data */
                XmStringLtoRCreate ("notepad" , "ISO8859-1"), 
                be->time, 			/* timestamp */
		w,              /* widget id needed to pass data by name */
		NULL,		/* Callback when passing data by name */ 
		&clipItemID     /* Number for this data item. Assigned by Xt */
            )
        ) != ClipboardSuccess)

   ;
/* If StartCopy was successful, get the selection  that has
 * been converted to the type FMT8BIT
 */



        XtGetSelectionValue
	(
            toplevel,			  /* widget id of requesting widget */ 
	    XA_PRIMARY, 	  	/* primary (vs. secondary) selection */
	    FMT8BIT,   			/* type to convert data to */
            callClosureWithSelection,  	/* callback after getting selection */
	    copyToClipboard,	  	/* argument for callback proc */
            be->time  		/* Timestamp of event triggering selection */
        ); 
    }
    else
    {
        Feep();
        return;
    }
}


void DoCut (w, client_data, cb)
    Widget w;                   	/* Widget id */
    XtPointer client_data;		/* Our callback data */
    XmPushButtonCallbackStruct *cb;	/* Widget's data */
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    XmTextBlockRec  block;
    XmTextPosition  left, right;
    XmTextStatus    status;

    DoCopy(w, client_data, cb);
    block.length = 0;
    block.ptr = NULL;
    block.format = FMT8BIT;

    if ((*Psource->GetSelection)(Psource, &left, &right))
    {
	status = (*Psource->Replace)
	    (textwindow, NULL, &left, &right, &block, True);
        if (status != EditDone) Feep();
    }
    else
    {
	Feep();
    }
}

void DoPaste (w)
    Widget w;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
    int             len, outlen, private_id, pos;
    char           *buf = NULL;
    XmTextBlockRec  block;
    XmTextStatus    tstatus;

    XmClipboardInquireLength
      (CurDpy, XtWindow(textwindow), "STRING", &len);
    buf = XtMalloc(len);
    if (XmClipboardRetrieve(CurDpy, XtWindow(textwindow),"STRING",
		buf, len,&outlen, &private_id) == ClipboardSuccess)
    {
        pos = XmTextGetInsertionPosition (textwindow);
	block.length = outlen;
	block.ptr = buf;
	block.format = FMT8BIT;

        buf[len] = '\0';
        XmTextInsert(textwindow, pos, buf); 
    }
    else
    {
	Feep();
    }
    XtFree(buf);
}

void DoInclude (filename)
    char *filename;
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
{
#define chunk 2048
    XmTextBlockRec  text;
    XmTextPosition  pos;
    int             amount;
    FILE           *file;
    char           *buf;
    XmTextStatus    status;

    file = fopen(filename, "r");
    if(!file)
    {
	ShowMessage(&Stuff._fileNotFoundMessage, "fileNotFoundMessage");
	return;
    }
    pos = XmTextGetInsertionPosition (textwindow);
    text.format = FMT8BIT;
    buf = XtMalloc(chunk);
    text.ptr = buf;
    while(( amount = fread(buf, 1, chunk, file)) > 0)
    {
	text.length = amount;
	status = (*Psource->Replace)
	    (textwindow, NULL, &pos, &pos, &text, True);
	if(status != EditDone)
	{
	    Feep();
	    break;
	}
	pos += amount; 
    }
    fclose(file);
    XtFree(buf);
}
