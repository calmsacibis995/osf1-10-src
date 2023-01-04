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
#ifndef lint
static char rcs_id[] = "@(#)$RCSfile: msg.c,v $ $Revision: 1.1.6.11 $ (DEC) $Date: 1994/01/11 22:06:23 $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */


/* msgs.c -- handle operations on messages. */

#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "decxmail.h"
#include "actionprocs.h"
#include "msg.h"
#include "toc.h"
#include <X11/Xatom.h>
#include "tocintrnl.h"
#include "capsar.h"
#include <Xm/TextP.h>
#include <Xm/MessageB.h>
/* #include <DXm/DXmSvn.h> */
#include "EDiskSrc.h"
#include <syslog.h>
/* #include "glob_def.h" */

void MsgSetChanged();
static Boolean confirmDisplay();
static char *splitMessage();
void MsgDestroyPS();
static void MsgComposeInCustomEditor();
/* extern void ChildDone(); */
int catch_sigchld();
extern CountLines();
static void SendbackEditedMessage();

typedef struct  _custom_edit_ {
  unsigned int  edit_pid;
  char FileName[50];
  Widget compose_widget;
  Widget widget_edit;
  Scrn scrn_edit;
} Custom_Edit;


int DDIFdecode(MM *m, FILE *out, int decodeflag);


#ifdef IGNOREDDIF
capsar_quick(fid)
FILE *fid;
{
    return TEXT;
}

#define capsar_parse_file(a, b)		NULL
#define capsar_get_header_list(a)	NULL
#define capsar_is_body_type(a, b)	FALSE
#define capsar_get_body_type(a)		BODY_TYPE_DEF
#define capsar_extract(a, b)
#define capsar_Destroy(a)
#define capsar_unparse_file(a, b)
#define capsar_create(a, b)		NULL
#endif IGNOREDDIF


/* Return the user-viewable name of the given message. */

char *MsgGetName(msg)
Msg msg;
{
    static char result[1028];
    (void) sprintf(result, "%s %d", msg->toc->foldername, msg->msgid);
    return result;
}


/*
 * Get the mm record for this message.  Load it if necessary.  Returns NULL
 * if can't load.
 */

static MMPtr GetMMPtr(msg)
Msg msg;
{
    int fid;
    if (msg->mm == NULL) {
	fid = myopen(MsgFileName(msg), O_RDONLY, 0666);
	if (fid < 0) {
	    DEBUG(("Can't open file %s in GetMMPtr!\n", MsgFileName(msg)));
	    return NULL;
	}
	msg->mm = capsar_parse_file(fid, NOTSMTP);
	(void) myclose(fid);
    }
    return msg->mm;
}



/*
 * Get the string containing the message header from the given MMPtr.  Returns
 * the data in a static variable.
 */

static char *GetHeaderFromMMPtr(mm)
MMPtr mm;
{
    static char *buf = NULL;
    static int maxbuf = 0;
    register int length, l;
    char **ptr;
    length = 0;
    if (mm != NULL) {
	ptr = capsar_get_header_list(mm);
	while (ptr && *ptr) {
	    l = strlen(*ptr);
	    if (length + l >= maxbuf) {
		maxbuf = length + l + 1;
		buf = XtRealloc(buf, (unsigned) maxbuf);
	    }
	    bcopy(*ptr, buf + length, l);
	    length += l;
	    ptr++;
	}
    }
    if (length == 0) return "";
    buf[length] = 0;
    return buf;
}

static char	extfile[80];
struct opt {
	char	*tagline;
	int	cnt;
} procopt;

static struct opt procopts[] = {
	"cat.compress.uuencode",1,
	"ctod.compress.uuencode",1,
	"cat.uuencode",0,
	"ctod.uuencode",0,
	NULL,NULL
};

extract_capsar(m,out)
MM	*m;
FILE	*out;
{
/*------*\
  Locals
\*------*/

	char		*concat();	/* concatenate strings */

	if(strcmp(m->body_type,DOTSTAG)==0 || strcmp(m->body_type,DDIFTAG)==0
	   || strcmp(m->body_type, DTIFTAG)==0 ){
		if(DDIFdecode(m,out,1)== OK)return(OK);
		else return(NOTOK);
	}
	if(DDIFdecode(m,out,0)==OK)return(OK);
	else return(NOTOK);
}

DDIFdecode(m,out,decodeflag)
MM		*m;
FILE		*out;
int		decodeflag;
{
    char	*command;
    char	*concat();
    char	buf[2*BUFSIZ];
    char	file[128];
    FILE	*pipeptr,*fileptr;
    struct  opt *tp;
    char	*tcp;
    char	*match();
    char	stdbuf[BUFSIZ];
    register int	n,nwritten,offset;
    char	*uudecode;
    char        tmp_name[80];

    if (m->dataptr != NULL) {
	strcpy(extfile,"/tmp/ddif_extXXXXXX");

/* *\   Problem with Insight on "mktemp" ... no other reason.
 * *    (void) unlink(mktemp(extfile));
\* */
        strcpy(tmp_name,extfile);
        mktemp(&tmp_name[0]);
        strcpy(extfile,tmp_name);
	(void) unlink(extfile);

	if ((fileptr = fopen(extfile,"w")) == NULL) {
	    fprintf(stderr, " cannot open %s for writing \n",extfile);
	    return(NOTOK);
	}
	(void) chmod(extfile,0660);
	if (fwrite(m->dataptr,1,m->size,fileptr) == NULL) {
	    fprintf(stderr, " write error on file %s \n",extfile);
	    /* fprintf is used because libcapsar in now i18nized and
               the syntax of error_extract() has changed */ 
	    return(NOTOK);
	}
	(void) fclose(fileptr);
	(void) strcpy(file,extfile);
    } else
	if (m->swapfile !=NULL)
	    (void) strcpy(file,m->swapfile);
	else return(NOTOK);

    if(decodeflag) {
        uudecode = UUDECODE_CMD;
	if (access(UUDECODE_CMD,F_OK) == NOTOK) {
	    if (access(UUDECODE_ALT_CMD, F_OK) == NOTOK) {
		fprintf (stderr, " cannot access cmd : %s \n",UUDECODE_CMD);
		return(NOTOK);
	    } else {
	        uudecode = UUDECODE_ALT_CMD;
	    }
	}

	for(tp= procopts;tcp=tp->tagline;tp++){
	    if (match(strlen(tp->tagline),tp->tagline,strlen(m->start),
			m->start) )
		break;
	}
	if (tcp == NULL) return(NOTOK);	

	if (tp->cnt == NULL) {
	    command = concat(uudecode,"  ",file,NULLCP);
	} else {
	    if (access(UNCOMPRESS_CMD,F_OK) == NOTOK){
		fprintf(stderr, " cannot access cmd : %s \n",
		    UNCOMPRESS_CMD);
		return(NOTOK);
	    }
	    command = concat(uudecode,"  ",file," | ",UNCOMPRESS_CMD,NULLCP);
	}
	
	if ((pipeptr=popen(command,"r"))== NULL) {
	    fprintf(stderr, "cannot execute command \n %s \n",command);
	    return(NOTOK);
	}


	while ((n=read(fileno(pipeptr),stdbuf,sizeof(stdbuf))) > 0) {
	    offset=0;
	    do {
		nwritten = write(fileno(out),&stdbuf[offset],n);
		if (nwritten <=0) {
		    fprintf(stderr, "extract : write error ");
		    return(NOTOK);
		}
		offset += nwritten;
	    } while ((n -= nwritten) > 0);
	}
	if (n < 0) {
	    DEBUG(( "extract read error\n"));
	    (void) fclose(fileptr);		
	    (void) free(command);
	    return(NOTOK);
	}
	
	(void) fclose(fileptr);		
	(void) free(command);
    } else {
	if ((fileptr=fopen(file,"r"))== NULL) {
	    (void) fprintf (stderr," cannot open file %s for reading \n",file);
	    return(NOTOK);
	}
	while (fgets(buf,sizeof buf,fileptr) != NULL)
	    fputs(buf,out);

	(void) fclose(fileptr);		
    }
	
    if (access(file,F_OK) ==0) unlink(file);
    return(OK);
}

/*
 * Extract the ddif message from the given MM ptr into a temporaray file.
 * Returns the name of the file as a newly malloced string; free the name when
 * done...
 */

static char *GetDDIFFileFromMMPtr(mm)
MMPtr mm;
{
    char *filename;
    FILE *fid;
    MMPtr m;
    filename = MallocACopy(MakeNewTempFileName());
    fid = FOpenAndCheck(filename, "w");
    for (m = mm ; m ; m = m->mm_next) {
	if (strcmp(m->body_type, "DOTS") == 0 ||
	    strcmp(m->body_type, "DDIF") == 0 ||
	    strcmp(m->body_type, "DTIF") == 0 ||
	    strcmp(m->body_type, "dots") == 0 ||
	    strcmp(m->body_type, "dtif") == 0 ||
	    strcmp(m->body_type, "ddif") == 0)
	    extract_capsar(m, fid);
    }
    (void) myfclose(fid);
    return filename;
}

static char *GetPSFileFromMMPtr(mm)
MMPtr mm;
{
    MMPtr m;
    for (m = mm; m ; m = m->mm_next) {
	if (strcmp(m->body_type, "ps") == 0) {
	    return m->name;
	}
    }
    return NULL;
}


/* 
 * Set the message areas in the given screen to be displaying nothing.
 */

static void ClearSource(scrn)
Scrn scrn;
{

    if (scrn) {
	/* * XtEDiskSetSource(scrn->viewwidget, NullSource, 0, 0); 
         * */
        XmTextSetString( scrn->viewwidget, "" );
	if (scrn->ddifheaders) {
	    /* * XtEDiskSetSource(scrn->ddifheaders, NullSource, 0, 0);
             * */
            XmTextSetString( scrn->ddifheaders, "" );
	    DDIFShowFile(scrn, "/dev/null");
	}
    }
}


static void RaiseWidget(widget)
Widget widget;
{
    XtWidgetGeometry request, reply;
    request.request_mode = CWStackMode;
    request.stack_mode = Above;
    request.sibling = None;
    (void) XtMakeGeometryRequest(widget, &request, &reply);
}

/* A major msg change has occured; redisplay it.  (This also should
work even if we now have a new source to display stuff from.)  This
routine arranges to hide boring headers, and also will set the text
insertion point to the proper place if this is a composition and we're
viewing it for the first time. */

  extern  int  DXM_in_DDIFerrorHandler;

void RedisplayMsg(scrn)
Scrn scrn;
{
    Msg msg;
    XmTextPosition startPos, lastPos, nextPos, eightPos;
    register int length;
    char str[513], *filename;
    XmTextBlockRec block;
    XmTextWidget sourcewidget;
    XWindowAttributes   winfo;
	int aa;

    lastPos=0;
    DEBUG(("@ RedisplayMsg(scrn)\n"));
    BeginLongOperation();
    if (scrn && scrn->viewwidget) {
	msg = scrn->msg;
	if (!msg)
	    ClearSource(scrn);
	else {
	    startPos = 0;
	    if (defHideBoringHeaders && msg->toc != DraftsFolder) {
		lastPos = (*msg->source->Scan)(msg->source, 0, XmSELECT_ALL,
					       XmsdRight, 1, FALSE);
		DEBUG(("  LastPos of msg %d bytes\n",lastPos));
		DEBUG(("  Scanning message for header lines:\n"));
 		while (startPos < lastPos) {
		    nextPos = startPos;
		    length = 0;

/* Read eight bytes from the message for testing against the 'interesting'
 * set of message headers...
 */
		    eightPos = (*msg->source->Scan)(msg->source, startPos,
						    XmSELECT_POSITION,XmsdRight,
						    8, TRUE);
		    while (nextPos < eightPos) {
			nextPos =(*msg->source->ReadSource)(msg->source,nextPos,
							      eightPos, &block);
			(void) strncpy(str + length, block.ptr, block.length);
 			length += block.length;
		    }

/* If we are at an 'interesting' header line, then break out of this scan. */
		    if (length == 8) {
			if (strncmp(str, "From:", 5) == 0 ||
			    strncmp(str, "To:", 3) == 0 ||
			    strncmp(str, "Date:", 5) == 0 ||
			    strncmp(str, "Subject:", 8) == 0) break;
			str[8] = '\0';
			DEBUG(("  Ignoring line starting '%s'...\n",str));
		    }

/* Scan to the start of the next message line. */
		    startPos = (*msg->source->Scan)(msg->source,startPos,
							XmSELECT_LINE,XmsdRight,
							1,TRUE);
 		}
		DEBUG(("  COMPLETED.\n"));
		if (startPos >= lastPos)
		    startPos = 0;
	    }

            /* Cleanup any open Dvr files. */

            if (scrn->ddifbody && XtIsManaged(scrn->ddifbody) &&
               !DXM_in_DDIFerrorHandler ) {
                DvrCloseFile( scrn->ddifbody );
            }


	    if (msg->msgtype == MTtext) {
			sourcewidget = (XmTextWidget) scrn->viewwidget;
		if (scrn->ddifpane && XtIsManaged(scrn->ddifpane))
		    XtUnmanageChild(scrn->ddifpane);
	    } else {
		if (msg->msgtype == MTddif) {
		    if (scrn->ddifpane == NULL)
			MakeDDIFWidgets(scrn);
		    XtManageChild(scrn->ddifpane);
		    sourcewidget = (XmTextWidget) scrn->ddifheaders;
		    if (GetMMPtr(msg)) {
			filename = GetDDIFFileFromMMPtr(GetMMPtr(msg));
			DDIFShowFile(scrn, filename);
			XtFree(filename);
		    }
		    else

			DDIFShowFile(scrn, "/dev/null");
		} 
		else 		
		   if (msg->msgtype == MTdtif) {
		    if (scrn->ddifpane == NULL)
			MakeDDIFWidgets(scrn);
		    XtManageChild(scrn->ddifpane);
		    sourcewidget = (XmTextWidget) scrn->ddifheaders;
		    if (GetMMPtr(msg)) {
			filename = GetDDIFFileFromMMPtr(GetMMPtr(msg));
			DDIFShowFile(scrn, filename);
			XtFree(filename);
		    }
		    else

			DDIFShowFile(scrn, "/dev/null");
		  } 
		   else 
		     {
		     if (msg->msgtype == MTps) {
DEBUG(("scrn-> ddifpane = %lx scrn->ddifbody = %lx\n",scrn->ddifpane,scrn->ddifbody));
			if (scrn->ddifpane == NULL)
			     MakeDDIFWidgets(scrn);
DEBUG(("After MakeDDIFWIdgets: scrn-> ddifpane = %lx scrn->ddifbody = %lx\n",scrn->ddifpane,scrn->ddifbody));
			XtManageChild(scrn->ddifpane);
			XtManageChild(scrn->viewwidget);
			sourcewidget = (XmTextWidget) scrn->ddifheaders;
			if (GetMMPtr(msg)) {
			    filename = GetPSFileFromMMPtr(GetMMPtr(msg));
			    PSShowFile(scrn, filename);
			} else {
			    DDIFShowFile(scrn, "/dev/null");
			}
		      }
		   }
	      }
	    _XmTextDisableRedisplay(sourcewidget, FALSE);
	    XtEDiskSetSource(sourcewidget,msg->source,startPos,
				(XmTextPosition)0);
	    XmTextSetTopCharacter(sourcewidget, startPos);
	    _XmTextEnableRedisplay(sourcewidget);
	    if (msg->startPos > 0) {
		XmTextSetInsertionPosition(sourcewidget, lastPos);
		XmTextSetInsertionPosition(sourcewidget, msg->startPos);
		msg->startPos = 0; /* Start in magic place only once. */
	    }
	}
	if (msg && msg->msgtype == MTddif) {
	    if (!scrn->ddifontop) {
		RaiseWidget(scrn->ddifpane);
		scrn->ddifontop = TRUE;
	    }
	}
	else {
	    if (scrn->ddifontop) {
		RaiseWidget(scrn->viewwidget);
		scrn->ddifontop = FALSE;
	    }
        }

        XGetWindowAttributes( XtDisplay(scrn->viewwidget),
                              XtWindow(XtParent(scrn->main)), &winfo );
        if( winfo.map_state == IsViewable ) {

            XRaiseWindow( XtDisplay(scrn->viewwidget),
                          XtWindow(XtParent(scrn->main)) );
            XSetInputFocus( XtDisplay(scrn->viewwidget),
                            XtWindow(XtParent(scrn->main)),
                            RevertToParent, CurrentTime );
        }

	if (lastPos == 0)
		lastPos = (*msg->source->Scan)(msg->source, 0, XmSELECT_ALL,
					       XmsdRight, 1, FALSE);

sourcewidget->text.total_lines = CountLines(sourcewidget->text.source,1,lastPos);
DEBUG(("WIDGET = %lx total_lines = %d\n",sourcewidget,sourcewidget->text.total_lines));
    }
    EndLongOperation();
}

void RedisplayPSFile(scrn, msg)
Scrn scrn;
Msg msg;
{
    char *filename;
    FILE *fid;
    MMPtr m;
    Boolean found;

    if (msg->msgtype != MTps) return;

    if (msg->mm) {
        MsgDestroyPS(msg);
    }
    fid = myfopen(MsgFileName(msg), "r");
    if (fid == NULL) return;

    msg->mm = XtNew(MM);
    msg->mm->name = XtNewString(MakeNewTempFileName());
    msg->mm->start = msg->mm->stop = NULL;
    msg->mm->body_type = XtNewString("ps");
    msg->mm->separator = msg->mm->swapfile = NULL;
    msg->mm->mm_next = NULL;
    msg->mm->dataptr = splitMessage(fid, msg->mm->name);
    msg->msgtype = MTps;
    myfclose(fid);

    if (GetMMPtr(msg)) {
	filename = GetPSFileFromMMPtr(GetMMPtr(msg));
	PSShowFile(scrn, filename);
    }
}


static char tempDraftFile[1028] = "";

/* Temporarily move the draftfile somewhere else, so we can exec an mh
   command that affects it. */

static void TempMoveDraft()
{
    if (FileExists(draftFile)) {
	(void) strcpy(tempDraftFile, MakeNewTempFileNameInSameDir(draftFile));
	RenameAndCheck(draftFile, tempDraftFile);
    }
}



/* Restore the draftfile from its temporary hiding place. */

static void RestoreDraft()
{
    if (*tempDraftFile) {
	RenameAndCheck(tempDraftFile, draftFile);
	*tempDraftFile = 0;
    }
}



/* Public routines */


/* Given a message, return the corresponding filename. */

char *MsgFileName(msg)
Msg msg;
{
    static char result[500];
    if (msg == NULL) return NULL;
    if (msg->toc == NULL) return NULL;
    (void) sprintf(result, "%s/%d", msg->toc->path, msg->msgid);
    return result;
}


static Boolean WorkRescan(data)
Opaque data;
{
    TRACE(("@WorkRescan\n"));
    TocForceRescan((Toc) data);
    return TRUE;
}

/*
 * Get the type of message.  If ddif, load it into capsar.
 */

MsgType MsgGetMsgType(msg)
Msg msg;
{
#ifndef DDIFVIEWER
    msg->msgtype = MTtext;
    return MTtext;
#else
    static char *splitMessage();
    Boolean checkForPS();
    FILE *fid;
    MMPtr m;
    Boolean found;
    char msg_type_str[10];

    if (msg->msgtype == MTunknown) {
	fid = myfopen(MsgFileName(msg), "r");
	if (fid == NULL && msg->toc) {
	    DEBUG(("No file found in MsgGetMsgType; setting up to rescan.\n"));
	    XtAddWorkProc(WorkRescan, (Opaque) msg->toc);
	    return MTunknown;
	}
	switch (capsar_quick(fid)) {
	  case NOTOK:
	    DEBUG(("capsar_quick returned NOTOK; treating as text.\n"));
	    /* fall through */
	  case TEXT:
	    if (checkForPS(fid) &&
		confirmDisplay("This is a PostScript(R) message - display it?")) {
		msg->msgtype = MTps;
		DEBUG(("Capsar_quick() returned PS.\n"));
		msg->mm = XtNew(MM);
		msg->mm->name = XtNewString(MakeNewTempFileName());
		msg->mm->start = msg->mm->stop = NULL;
		msg->mm->body_type = XtNewString("ps");
		msg->mm->separator = msg->mm->swapfile = NULL;
		msg->mm->mm_next = NULL;
		msg->mm->dataptr = splitMessage(fid, msg->mm->name);
	    } else {
		msg->msgtype = MTtext;
		DEBUG(("Capsar_quick() returned TEXT.\n"));
	    }
	    break;
	  case CDA:
	    DEBUG(("Capsar_quick() returned CDA.\n"));
	    found = FALSE;
	    if (confirmDisplay("This is a CDA image - display it?")) {
	      if (msg->mm == NULL)
		msg->mm = capsar_parse_file(fileno(fid), NOTSMTP);
	      for (m = msg->mm ; m && !found ; m = m->mm_next) {
		DEBUG(("BodyType: '%s'\n",capsar_get_body_type(m)));
		strcpy(msg_type_str, capsar_get_body_type(m));
		if (strcmp(msg_type_str, BODY_TYPE_DEF) != 0)
		  found = TRUE;
	      }
	    }
	    if (found) {
	      if (strncmp(msg_type_str, DTIFTAG, strlen(DTIFTAG)) == 0)
		msg->msgtype = MTdtif;
	      else
		msg->msgtype = MTddif;
	    } else {
		DEBUG(("Whoops! Not really ddif!!!\n"));
		msg->msgtype = MTtext; /* Not really ddif after all! */
		capsar_Destroy(msg->mm);
		msg->mm = NULL;
	    }
	    break;
	  default:
	    Punt("Bad value returned from capsar_quick!");
	}
	myfclose(fid);
    }
    return msg->msgtype;
#endif DDIFVIEWER
}




/*
 * Create the source for this message, if there isn't already one.  
 */

void MsgCreateSource(msg, editable,scrn)
Msg msg;
Boolean editable;		/* Whether the user should be able to */
				/* edit the string. */
Scrn scrn;
{
    MMPtr mm;
    XmTextWidget tw;

    DEBUG(("@ MsgCreateSource(msg, editable)\n"));
    if (msg->source) return;
    if ( MsgGetMsgType(msg) != MTtext )
        if ( scrn->ddifpane == NULL )
            MakeDDIFWidgets(scrn);
    switch (MsgGetMsgType(msg)) {
      case MTtext:
        tw = (XmTextWidget) scrn->viewwidget;
	msg->source = XtCreateEDiskSource(MsgFileName(msg),editable,scrn,tw);
	break;
      case MTddif:
      case MTdtif:
        tw = (XmTextWidget) scrn->ddifheaders;
	msg->source =
	    XtCreateEDiskSourceFromString(GetHeaderFromMMPtr(GetMMPtr(msg)),
					  editable,scrn,tw);
	break;
      case MTps:
        tw = (XmTextWidget) scrn->ddifheaders;
	mm = GetMMPtr(msg);
	msg->source =
	    XtCreateEDiskSourceFromString(mm->dataptr,editable,scrn,tw);
	break;
      case MTunknown:
	msg->source = NULL;
	break;
    }
}


/*
 * Destroy the source for this message, if any.
 */

void MsgDestroySource(msg)
Msg msg;
{
    if (msg->source == NULL) return;
/*
    XtDestroyEDiskSource(msg->source);
*/
    msg->source = NULL;
    if (msg->mm) {
	capsar_Destroy(msg->mm);
	msg->mm = NULL;
    }
    msg->msgtype = MTunknown;
}


/*
 * Destroy the postscript file info for this
 * message
 */

void MsgDestroyPS(msg)
Msg msg;
{
    if (msg->mm == NULL) return;
    XtFree(msg->mm->name);
    XtFree(msg->mm->body_type);
    XtFree(msg->mm->dataptr);
    XtFree((char *)msg->mm);
    msg->mm = NULL;
    msg->msgtype = MTunknown;
}

/* Save any changes to a message.  Also calls the toc routine to update the
   scanline for this msg. */

void MsgSaveChanges2(msg,vw)
Msg msg;
XmTextWidget vw;
{
    register int i, length, fid;
    char *ptr;
    char *tmp;
    FILE *filptr;

    if (msg && msg->source) {
	if (msg->msgtype == MTtext) {
	    XtEDiskSaveFile(msg->source, vw);
	} else if (msg->msgtype == MTddif || msg->msgtype == MTdtif ) {
	    fid = OpenAndCheck(MsgFileName(msg), O_WRONLY | O_TRUNC | O_CREAT,
			       0666);
	    XtEDiskMarkSaved(msg->source);
            ptr = XmTextGetString(vw);
	    length = strlen(ptr);
	    WriteAndCheck(fid, ptr, length);
	    WriteAndCheck(fid, "\n", 1);
	    XtFree(ptr);
	    if (GetMMPtr(msg)) {
                capsar_unparse_file(GetMMPtr(msg), fid);
                /* *\ Allow "send" message more than once.
                 * *  capsar_Destroy(GetMMPtr(msg));
                 * *  msg->mm = NULL;
                \* */
	    }
	    (void) myclose(fid);
	} else if (msg->msgtype == MTps) {
	    tmp = MakeNewTempFileName();
	    fid = OpenAndCheck(tmp, O_WRONLY | O_TRUNC | O_CREAT,
			       0666);
	    XtEDiskMarkSaved(msg->source);
            ptr = XmTextGetString(vw);
	    length = strlen(ptr);
	    WriteAndCheck(fid, ptr, length);
	    WriteAndCheck(fid, "\n", 1);
	    XtFree(ptr);
	    (void) myclose(fid);
	    filptr = myfopen(MsgFileName(msg), "r");
	    XtFree(splitMessage(filptr, tmp));
	    (void) myfclose(filptr);
	    CopyFileAndCheck(tmp, MsgFileName(msg));
	    unlink(tmp);
	}
	TocSetCacheValid(msg->toc);
	TocMsgChanged(msg->toc, msg);
	for (i=0 ; i<msg->num_scrns ; i++)
	    EnableProperButtons(msg->scrn[i]);
    }
}


/*
 * Show the given message in the given scrn.  If a message is changed, and we
 * are removing it from any scrn, then ask for confirmation first.  If the
 * scrn was showing a temporary msg that is not being shown in any other scrn,
 * it is deleted.  If scrn is NULL, then remove the message from every scrn
 * that's showing it.
 */

static int SetScrn(msg, scrn, force)
Msg msg;
Scrn scrn;
Boolean force;	/* If TRUE, don't ask for confirm; just do it */
{
    char str[513];
    register int i, num_scrns;
    Toc origtoc;
    Boolean found;
    XmTextWidget sourcewidget;
    XWindowAttributes   winfo;

    DEBUG(("@ SetScrn(msg, scrn, force)\n"));
    if (scrn == NULL ) {
	if (msg == NULL || msg->num_scrns == 0) return 0;
	if (!force && MsgChanged(msg)) {
	    (void)sprintf(str,
			  "Are you sure you want to remove changes to %s?",
			  MsgGetName(msg));
	    if (!Confirm(msg->scrn[0], str,
			 "OnWindow OnCreating_and_Sending OnSaving_a_draft")) 
		return DELETEABORTED;
	}
	for (i=msg->num_scrns - 1 ; i >= 0 ; i--)
	    (void) SetScrn((Msg) NULL, msg->scrn[i], TRUE);
	return 0;
    }
    if (scrn->msg == msg) return 0;
    if ( scrn->msg && scrn->msg->num_scrns == 0) {
        scrn->msg->scrn[0] = NULL;
        return 0;
    }
    if (scrn->viewwidget == NULL) {
	scrn->msg = msg;
	return 0;
    }
    /*
     * This check has been put here to prevent crashes due to bad msg address.
     * This fix was based on the stack trace from a core file created by non
     * a non reproducable crash. After this fix, it does not crash during soak.
     * This check has to be removed once the actual problem has been traced.
     */
    if (msg == (Msg) 0x1) {
      DEBUG(("msg.c: SetScrn(): Invalid msg address\n"));
      return 0;
    }
    scrn->lastwentbackwards = defDefaultViewBackwards;
    if (scrn->msg) {
	origtoc = scrn->msg->toc;
	num_scrns = scrn->msg->num_scrns;
	for (i=0 ; i<num_scrns ; i++) {
	    if (scrn->msg->scrn[i] == scrn) break;
        }
	if (i >= num_scrns) Punt("Couldn't find scrn in SetScrn!");
	if (num_scrns == 1) {
	    if (!force && MsgChanged(scrn->msg)) {
		(void)sprintf(str,
			      "Are you sure you want to remove changes to %s?",
			      MsgGetName(scrn->msg));
		if (!Confirm(scrn, str,
			 "OnWindow OnCreating_and_Sending OnSaving_a_draft")) 
		    return DELETEABORTED;
	    }
/* Not needed for Motif 1.2
 *	    ClearSource(scrn);
 */
	    if (scrn->msg->temporary) {
		(void) unlink(MsgFileName(scrn->msg));
		TocRemoveMsg(scrn->msg->toc, scrn->msg);
		MsgDestroySource(scrn->msg);
		MsgFree(scrn->msg);
		scrn->msg = NULL;
	    } 
	    else {
		MsgDestroySource(scrn->msg);
            }
	}
	if (scrn->msg) {
            scrn->msg->num_scrns--;
            if ( i == scrn->msg->num_scrns ) {
               scrn->msg->scrn[i] = NULL;
            }
            else {
               scrn->msg->scrn[i] = scrn->msg->scrn[scrn->msg->num_scrns];
            }
        }
    } else origtoc = NULL;

    scrn->msg = msg;
    if (msg == NULL) {
        /* *\ ClearSource(scrn);
        \* */
	EnableProperButtons(scrn);
    } else {

/*      Determine if widget is mapped.  Unmap to avoid double update.
 *      "MsgCreateSource" displays entire msg starting at the top.
 *      "MsgRedisplay" displays again begining with the "From:" line ...
 *      possibly ignoring some routeing info lines.
 *      Prefered method of updating begin display pointer before first
 *      and only display of msg is too much to change now (last build level).
 */

        winfo.map_state == IsUnmapped;
        if ( scrn->viewwidget != NULL ) {
            XGetWindowAttributes( XtDisplay(scrn->viewwidget),
                                  XtWindow(scrn->viewwidget), &winfo );
            if ( winfo.map_state == IsViewable )
                XtUnmapWidget( scrn->viewwidget );
        }


	msg->num_scrns++;
	msg->scrn = (Scrn *) XtRealloc((char *)msg->scrn,
				       (unsigned) sizeof(Scrn)*msg->num_scrns);
	msg->scrn[msg->num_scrns - 1] = scrn;
	if (msg->source == NULL)
	    MsgCreateSource(msg, FALSE,scrn);
	if (msg->source == NULL) {
	    Error(toplevel, EFileCantOpen, MsgFileName(msg));
	    return 1;
	}
	if (scrn->readnum == 0)
/*
	    MsgSetEditable(msg);
*/
	XmTextSetEditable(scrn->viewwidget,True);
        if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
            XmTextSetEditable(scrn->ddifheaders,True);
	MsgSetChanged(msg, False);
	RedisplayMsg(scrn);
	EnableProperButtons(scrn);
	if (scrn->readnum == 1)
	    TocSetCurMsg(msg->toc, msg);
	if (unseenSeqName && msg->toc->incfile)
	    MsgRemoveFromSeq(msg, unseenSeqName);
	TocCheckSeqButtons(msg->toc);

        /* If window was viewable, then unmapped, map it again. */
        if ( winfo.map_state == IsViewable )
            XtMapWidget( scrn->viewwidget );

    }
    ScrnNeedsTitleBarChanged(scrn);
    ScrnNeedsIconNameChanged(scrn);
    if (AutoCommit && origtoc != NULL && (msg == NULL || origtoc != msg->toc)){
	found = FALSE;
	for (i=0 ; i<numScrns && !found ; i++) {
	    scrn = scrnList[i];
	    if (scrn->mapped && (scrn->toc == origtoc ||
			     (scrn->msg != NULL && scrn->msg->toc == origtoc)))
		found = TRUE;
	}
	if (!found) {
	    TocCommitChanges(origtoc);
	}
    }
    return 0;
}



/* Associate the given msg and scrn, asking for confirmation if necessary. */

int MsgSetScrn(msg, scrn)
Msg msg;
Scrn scrn;
{
    return SetScrn(msg, scrn, FALSE);
}


/* Same as above, but with the extra information that the message is actually
   a composition.  (Nothing currently takes advantage of that extra fact.) */

int MsgSetScrnForComp(msg, scrn)
Msg msg;
Scrn scrn;
{
int	status ;

    DEBUG(("@ MsgSetScrnForComp(msg, scrn)\n"));
    status = SetScrn(msg, scrn, FALSE);
    XtEDiskSetChanged(msg->source,True);
    XmTextSetEditable(scrn->viewwidget,True);
    if (scrn->ddifheaders && XtIsManaged(scrn->ddifheaders))
        XmTextSetEditable(scrn->ddifheaders,True);
    return status;
}


/* Associate the given msg and scrn, even if it means losing some unsaved
   changes. */

void MsgSetScrnForce(msg, scrn)
Msg msg;
Scrn scrn;
{
    (void) SetScrn(msg, scrn, TRUE);
}



void MsgRepaintLabels(msg)
Msg msg;
{
    register int i;
    Scrn scrn;
    for (i=0 ; i<msg->num_scrns ; i++) {
	scrn = msg->scrn[i];
	ScrnNeedsTitleBarChanged(scrn);
	ScrnNeedsIconNameChanged(scrn);
    }
}



static void ChangeMsgBuf(msg, ptr, cols)
Msg msg;
char *ptr;
IntRange cols;
{
    Toc toc = msg->toc;
    int i, j, pos;
    int length = cols[1];
    TagRec tag;

    if (length >= msg->length - cols[0])
	length = msg->length - cols[0] - 1;
    if (length <= 0) return;
    if (strncmp(msg->curbuf + cols[0], ptr, length) != 0) {
	(void) strncpy(msg->curbuf + cols[0], ptr, length);
	tag.tagFields.tocNumber = TUGetTocNumber(toc);
	tag.tagFields.msgNumber = TUGetMsgIndex(toc, msg);
	for (i = 0; i < toc->num_scrns; i++) {
	    TocDisableRedisplay(toc->scrn[i]->tocwidget);
	    pos = DXmSvnGetEntryNumber(toc->scrn[i]->tocwidget, tag.tagValue);
	    if (pos != 0) {
		DXmSvnInvalidateEntry(toc->scrn[i]->tocwidget, pos);
	    }
	    TocEnableRedisplay(toc->scrn[i]->tocwidget);
	}
    }
}
    

/* Set the fate of the given message. */

void MsgSetFate(msg, fate, desttoc)
  Msg msg;
  FateType fate;
  Toc desttoc;
{
    register int i;
    char *ptr;
    Scrn scrn;

    if (msg == NULL) return;
    if (msg->toc == NULL) return;
    if (msg->curbuf == msg->origbuf)
	msg->curbuf = MallocACopy(msg->origbuf);

    if (msg == msg->toc->curmsg) ptr = ScanCurrentString;
    else ptr = msg->origbuf + ScanCurrentCols[0];
    ChangeMsgBuf(msg, ptr, ScanCurrentCols);

    if (fate != msg->fate || desttoc != msg->desttoc) {
	if (msg->fate == Fmove)
	    ChangeMsgBuf(msg, msg->origbuf + ScanMoveCols[0], ScanMoveCols);
	else if (msg->fate == Fdelete)
	    ChangeMsgBuf(msg, msg->origbuf + ScanDeleteCols[0],ScanDeleteCols);

	if (fate == Fmove) 
	    ChangeMsgBuf(msg, ScanMovedString, ScanMoveCols);

	if (fate == Fdelete) 
	    ChangeMsgBuf(msg, ScanDeletedString, ScanDeleteCols);
    }

    msg->fate = fate;
    msg->desttoc = desttoc;

    for (i=0 ; i<msg->num_scrns ; i++)
	ScrnNeedsTitleBarChanged(msg->scrn[i]);
    for (i=0 ; i<numScrns ; i++) {
	scrn = scrnList[i];
	if (scrn->mapped && scrn->msg && scrn->msg->toc == msg->toc)
	    EnableProperButtons(scrn);
    }
}


/*
 * Set the fate of some messages.  If exactly one message is specified, then
 * do any appropriate auto-skipping.
 */

void MsgBatchSetFate(mlist, fate, desttoc)
MsgList mlist;
FateType fate;
Toc desttoc;
{
    Msg msg;
    register int i;
    for (i=0 ; i<mlist->nummsgs ; i++)
	MsgSetFate(mlist->msglist[i], fate, desttoc);
    if (mlist->nummsgs == 1) {
	msg = mlist->msglist[0];
	for (i=0 ; i<msg->num_scrns ; i++)
	    SkipToMsg(msg->scrn[i], FALSE);
    }
}



/* Get the fate of this message. */

FateType MsgGetFate(msg, toc)
Msg msg;
Toc *toc;			/* RETURN */
{
    if (toc) *toc = msg->desttoc;
    return msg->fate;
}


/* Make this a temporary message. */

void MsgSetTemporary(msg)
Msg msg;
{
    register int i;
    msg->temporary = TRUE;
    for (i=0 ; i<msg->num_scrns ; i++)
	ScrnNeedsTitleBarChanged(msg->scrn[i]);
}

/* Deterine whether this is a temporary message. */

Boolean MsgGetTemporary(msg)
Msg msg;
{
    return msg != NULL && msg->temporary;
}


/* Make this a permanent message. */

void MsgSetPermanent(msg)
Msg msg;
{
    register int i;
    msg->temporary = FALSE;
    for (i=0 ; i<msg->num_scrns ; i++)
	ScrnNeedsTitleBarChanged(msg->scrn[i]);
}



/* Return the id# of this message. */

int MsgGetId(msg)
Msg msg;
{
    return msg->msgid;
}


/* Return the (original) scanline for this message. */

char *MsgGetScanLine(msg)
Msg msg;
{
    return msg->origbuf;
}



/* Return the toc this message is in. */

Toc MsgGetToc(msg)
Msg msg;
{
    return (msg) ? msg->toc : NULL;
}


/* Set the reapable flag for this msg. */

void MsgSetReapable(msg)
Msg msg;
{
    register int i;
    msg->reapable = TRUE;
    MsgSetChanged(msg, False);
    for (i=0 ; i<msg->num_scrns ; i++)
	EnableProperButtons(msg->scrn[i]);
}



/* Clear the reapable flag for this msg. */

void MsgClearReapable(msg)
Msg msg;
{
    register int i;
    msg->reapable = FALSE;
    for (i=0 ; i<msg->num_scrns ; i++)
	EnableProperButtons(msg->scrn[i]);
}


/* Get the reapable value for this msg.  Returns TRUE iff the reapable flag
   is set AND no changes have been made. */

Boolean MsgGetReapable(msg)
Msg msg;
{
    return msg == NULL || (msg->reapable &&
			   (msg->source == NULL || !MsgChanged(msg)));
}


static void ChangeEditable(msg, value)
Msg msg;
{
    register int i;
    if (msg && msg->source) {
	for (i=0 ; i<msg->num_scrns ; i++) {
	    EnableProperButtons(msg->scrn[i]);
	}
    }
}

/* Make it possible to edit the given msg. */
void MsgSetEditable(msg)
Msg msg;
{
    ChangeEditable(msg, TRUE);
}



/* Turn off editing for the given msg. */

void MsgClearEditable(msg)
Msg msg;
{
    ChangeEditable(msg, FALSE);
}



/* Get whether the msg is editable. */

Boolean MsgGetEditable(msg)
Msg msg;
{
    return msg && msg->source && XtEDiskGetEditable(msg->source);
}


/* Get whether the msg has changed since last saved. */

Boolean MsgChanged(msg)
Msg msg;
{
    return msg && msg->source && XtEDiskChanged(msg->source);
}

void MsgSetChanged(msg, value)
Msg msg;
Boolean value;
{
    if (msg == NULL) return;
    XtEDiskSetChanged(msg->source, value);
}


/* Call the given function when the msg changes. */

void MsgSetCallOnChange(msg, func, param)
Msg msg;
void (*func)();
Opaque param;
{
    DEBUG(("@ MsgSetCallOnChange(msg, func, param)\n"))
    if (msg && msg->source)
	XtEDiskSetCallbackWhenChanged(msg->source, func, param);
}

/* Call no function when the msg changes. */

void MsgClearCallOnChange(msg)
Msg msg;
{
    DEBUG(("@ MsgClearCallOnChange(msg)\n"));
    if (msg && msg->source)
	XtEDiskSetCallbackWhenChanged(msg->source, (void (*)()) NULL,
				      (Opaque) NULL);
}


/*
 * Send (i.e., mail) the given message as is.  First break it up into lines,
 * and copy it to a new file in the process.  The new file is one of 10
 * possible draft files; we rotate amoung the 10 so that the user can have up
 * to 10 messages being sent at once.  (Using a file in /tmp is a bad idea
 * because these files never actually get deleted, but renamed with some
 * prefix.  Also, these should stay in an area private to the user for
 * security.)
 */

void MsgSend2(msg, vw)
Msg msg;
XmTextWidget vw;
{
FILE *from;
FILE *to;
int p, c, l, sendwidth, sendbreakwidth;
Boolean inheader;
Boolean intoorcc = FALSE;
Boolean insubject = FALSE;
Boolean collectaddresses = (msg->anno != NULL && msg->anno->num > 0);
char *ptr, *ptr2, *ptr3, **argv, str[1028];
static int sendcount = -1;
static char *addr = NULL;
static int addrmax = 0;
int addrlen = 0;
Boolean	sendres();		/* PJS.... */

    BeginLongOperation();
    MsgSaveChanges2(msg, vw);
    from = FOpenAndCheck(MsgFileName(msg), "r");
    sendcount = (sendcount + 1) % 10;
    (void) sprintf(str, "%s%d", draftScratchFile, sendcount);
    to = FOpenAndCheck(str, "w");
    sendwidth = defSendLineWidth;
    sendbreakwidth = defBreakSendLineWidth;
    if (MsgGetMsgType(msg) != MTtext) {
	sendwidth = sendbreakwidth = 9999;
    }
    inheader = TRUE;
    while (ptr = ReadLine(from)) {
	if (inheader) {
	    if (strncmpIgnoringCase(ptr, "sendwidth:", 10) == 0) {
		if (atoi(ptr+10) > 0) sendwidth = atoi(ptr+10);
		continue;
	    }
	    if (strncmpIgnoringCase(ptr, "sendbreakwidth:", 15) == 0) {
		if (atoi(ptr+15) > 0) sendbreakwidth = atoi(ptr+15);
		continue;
	    }
	    for (l = 0, ptr2 = ptr ; *ptr2 && !l ; ptr2++)
		l = (*ptr2 != ' ' && *ptr2 != '\t' && *ptr != '-');
	    if (l) {
		    Boolean containsheader;
		    ptr2 = index(ptr, ':');
		    ptr3 = index(ptr, ' ');
		    containsheader =
			(ptr2 != NULL && (ptr3 == NULL || ptr2 < ptr3));
		    if (intoorcc  && containsheader){
		      (void) fprintf(to, "\n", ptr);
		      intoorcc = FALSE;
		    }
		    if (insubject && containsheader) {
		      (void) fprintf(to, "\n", ptr);
		      insubject = FALSE;
		    }

		    if (!intoorcc) {
			if (containsheader && ptr2 - ptr == 2) {
			    if (strncmpIgnoringCase(ptr, "to", 2) == 0 ||
				strncmpIgnoringCase(ptr, "cc", 2) == 0)
				intoorcc = TRUE;
			  }
			else
			    intoorcc = FALSE;
		      }

		    if (!insubject){
		      if (containsheader && ptr2 - ptr == 7) {
			if (strncmpIgnoringCase(ptr, "subject", 7)== 0)
			  insubject = TRUE;
		      }
		      else
			  insubject = FALSE;
		    } 
		    if (intoorcc || insubject) 
		      (void) fprintf(to, "%s ", ptr);
		    else
		      (void) fprintf(to, "%s\n", ptr);	


		    if (intoorcc && collectaddresses) {
			if (ptr2) ptr2++;
			else ptr2 = ptr;
			l = strlen(ptr2);
			if (l + addrlen >= addrmax) {
			    addrmax = addrlen + l + 1;
			    addr = XtRealloc(addr, (unsigned) addrmax);
			}
			bcopy(ptr2, addr + addrlen, l);
			addrlen += l;
		      }
		    continue;
		  }

	    inheader = FALSE;
	    if (intoorcc || insubject)
	      (void) fprintf(to, "\n", ptr);
	    if (sendbreakwidth < sendwidth) sendbreakwidth = sendwidth;
	  }
	do {
	    for (p = c = l = 0, ptr2 = ptr;
		 *ptr2 && c < sendbreakwidth;
		 p++, ptr2++) {
		 if (*ptr2 == ' ' && c < sendwidth)
		     l = p;
		 if (*ptr2 == '\t') {
		     if (c < sendwidth) l = p;
		     c += 8 - (c % 8);
		 }
		 else
		 c++;
	     }
	    if (c < sendbreakwidth) {
		(void) fprintf(to, "%s\n", ptr);
		*ptr = 0;
	    }
	    else
		if (l) {
		    ptr[l] = 0;
		    (void) fprintf(to, "%s\n", ptr);
		    ptr += l + 1;
		}
		else {
		    for (c = 0; c < sendwidth; ) {
			if (*ptr == '\t') c += 8 - (c % 8);
			else c++;
			(void) fputc(*ptr++, to);
		    }
		    (void) fputc('\n', to);
		}
	} while (*ptr);
    }

    (void) myfclose(from);
    (void) myfclose(to);

/* PJS: Added in explicit removal of the verbose and watch arguments to send
 * so that we can catch error reports from the postproc.
 * Also added in OPTIONAL use of the push and nopush options to the postproc,
 * with nopush as the default, so that mail is no longer junked with no
 * warning if there are no smtp servers, for example.
 */
    argv = MakeArgv(6);
    argv[0] = sendCmd;
    argv[1] = "-noverbose";
    argv[2] = "-nowatch";

/*    if (resource_list.bsgend){
 *	argv[3] = "-push";
 *      }
 *    else
 */
	argv[3] = "-nopush";
    argv[4] = str;
    argv[5] = NULL;

/*    if (resource_list.bgsend) { */
/* PJS: Make a fresh copy of the filename, as it is static otherwise... */
/*	ptr = MallocACopy(MakeNewTempFileName());
 *	DoCommandInBackground(argv,(char *)NULL,ptr,(XtWorkProc)sendres,ptr);
 *    }
 *    else {
 */
	ptr = MallocACopy(DoCommandToFile(argv));
	sendres(ptr);
 /*    } */ 

    XtFree((char *) argv);
    if (collectaddresses) {
	Toc toc;
	Msg msg2;
	int i;

	addr[addrlen] = 0;
	while (ptr = index(addr, ',')) *ptr = ' ';
	while (ptr = index(addr, '\n')) *ptr = ' ';
	DEBUG(("To/Cc: %s\n", addr));
	argv = MakeArgv(10 + msg->anno->num);
	i = 0;
	do {
	    toc = NULL;
	    c = 0;
	    argv[c++] = MallocACopy(annoCmd);
	    argv[c++] = NULL;	/* We'll put in foldername later. */
	    for ( ; i<msg->anno->num ; i++) {
		msg2 = MsgFromHandle(msg->anno->list[i]);
		if (msg2) {
		    if (toc == NULL) {
			toc = MsgGetToc(msg2);
			sprintf(str, "+%s", TocGetFolderName(toc));
			argv[1] = MallocACopy(str);
		    }
		    if (MsgGetToc(msg2) != toc) break;
		    sprintf(str, "%d", MsgGetId(msg2));
		    argv[c++] = MallocACopy(str);
		    TocMsgChanged(toc, msg2); /* Done in background, later. */
		}
		MsgFreeHandle(msg->anno->list[i]);
	    }
	    argv[c++] = MallocACopy("-component");
	    argv[c++] = MallocACopy(msg->anno->type);
	    argv[c++] = MallocACopy("-inplace");
	    argv[c++] = MallocACopy("-text");
	    argv[c++] = MallocACopy(addr);
	    argv[c] = 0;
	    if (toc != NULL) {
		DoCommand(argv, (char *) NULL, "/dev/null");
		TocSetCacheValid(toc);
	    }
	    for (c-- ; c>=0 ; c--) XtFree(argv[c]);
	} while (i < msg->anno->num);
	XtFree((char *) argv);
	XtFree((char *) msg->anno->list);
	XtFree((char *) msg->anno);
	msg->anno = NULL;
    }
    EndLongOperation();
}


Boolean
sendres(file)
char	*file;
{
int	length, fid;
char	*result;

    length = GetFileLength(file);
    if (length <= 0) {
	DeleteFileAndCheck(file);
	XtFree(file);
	return True;
    }
    result = XtMalloc((unsigned) length + 1);
    fid = myopen(file, O_RDONLY, 0666);
    if (length != read(fid, result, length)) {
	unlink(file);
	Punt("Couldn't read result from DoCommandToString");
    }
    result[length] = 0;
    DEBUG(("('%s')\n", result));
    (void) myclose(fid);
    DeleteFileAndCheck(file);
    XtFree(file);

    Warning(toplevel,NULL,result);
    XtFree(result);
    return True;
}

/* Make the msg into the form for a generic composition.  Set msg->startPos
 * so that the text insertion point will be placed at the end of the first
 * line (which is usually the "To:" field).
 */


void MsgLoadComposition(msg)
Msg msg;
{
    static char *blankcomp = NULL; /* Array containing comp template */
    static int compsize = 0;
    static XmTextPosition startPos;
    char *file, **argv;
    int fid;
    msg->msgtype = MTtext;
    if (blankcomp == NULL) {
	file = MallocACopy(MakeNewTempFileName());
	argv = MakeArgv(5);
	argv[0] = compCmd;
	argv[1] = "-file";
	argv[2] = file;
	argv[3] = "-nowhatnowproc";
	argv[4] = "-nodraftfolder";
	DoCommand(argv, (char *) NULL, "/dev/null");
	XtFree((char *) argv);
	compsize = GetFileLength(file);
	if (compsize < 0)
	  	Punt("Error in MsgLoadComposition. Check MH profile");
        blankcomp = XtMalloc((unsigned) compsize+1);
        blankcomp[compsize] = '\0';
	fid = OpenAndCheck(file, O_RDONLY, 0666);
	if (compsize != read(fid, blankcomp, compsize))
	    Punt("Error reading in MsgLoadComposition!");
	(void) myclose(fid);
	DeleteFileAndCheck(file);
	XtFree(file);
	startPos = (char *)index(blankcomp, '\n') - blankcomp;
    }
    fid = OpenAndCheck(MsgFileName(msg), O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (compsize != write(fid, blankcomp, compsize))
	Punt("Error writing in MsgLoadComposition!");
    (void) myclose(fid);
    TocSetCacheValid(msg->toc);
    msg->startPos = startPos;
}



/* Load a msg with a template of a reply to frommsg.  Set msg->startPos so
 * that the text insertion point will be placed at the beginning of the
 * message body.
 */

void MsgLoadReply(msg, frommsg)
Msg msg, frommsg;
{
    char **argv;
    char str1[1028], str2[10];
    int c = 0;
    msg->msgtype = MTtext;
    TempMoveDraft();
    argv = MakeArgv(10);
    argv[c++] = replCmd;
    (void) sprintf(str1, "+%s", frommsg->toc->foldername);
    argv[c++] = str1;
    (void) sprintf(str2, "%d", frommsg->msgid);
    argv[c++] = str2;
    argv[c++] = "-nowhatnowproc";
    argv[c++] = "-noinplace";
    argv[c++] = "-nodraftfolder";
    if (defReplyCCAll != DEFAULT) {
	argv[c++] = (defReplyCCAll == TRUE) ? "-cc" : "-nocc";
	argv[c++] = "all";
    }
    if (defReplyCCMe != DEFAULT) {
	argv[c++] = (defReplyCCMe == TRUE) ? "-cc" : "-nocc";
	argv[c++] = "me";
    }
    argv[c] = 0;
    DoCommand(argv, (char *) NULL, "/dev/null");
    XtFree((char *) argv);
    RenameAndCheck(draftFile, MsgFileName(msg));
    RestoreDraft();
    TocSetCacheValid(frommsg->toc); /* If -anno is set, this keeps us from
				       rescanning folder. */
    TocSetCacheValid(msg->toc);
    msg->startPos = GetFileLength(MsgFileName(msg));
    if (defAnnotateReplies) {
	msg->anno = XtNew(AnnoRec);
	msg->anno->type = "Replied";
	msg->anno->num = 1;
	msg->anno->list = (MsgHandle *) XtMalloc(sizeof(MsgHandle));
	msg->anno->list[0] = MsgGetHandle(frommsg);
    }
}



/* Load a msg with a template of forwarding a list of messages.  Set 
 * msg->startPos so that the text insertion point will be placed at the end
 * of the first line (which is usually a "To:" field).
 */

void MsgLoadForward(msg, mlist,scrn)
  Msg msg;
  MsgList mlist;
  Scrn scrn;
{
    char  **argv, str[1028], *filename;
    register int     i;
    if (mlist->nummsgs == 0) return;
    if (mlist->nummsgs == 1 && ( (MsgGetMsgType(mlist->msglist[0]) == MTddif)||
				( MsgGetMsgType(mlist->msglist[0]) == MTdtif)))
	{
	  filename = GetDDIFFileFromMMPtr(GetMMPtr(mlist->msglist[0]));
	  MsgLoadFromFile(msg, filename,scrn);
	  (void) unlink(filename);
	  XtFree(filename);
	  return;
	}

    msg->msgtype = MTtext;
    TempMoveDraft();
    argv = MakeArgv(5 + mlist->nummsgs);
    argv[0] = forwCmd;
    (void) sprintf(str, "+%s", mlist->msglist[0]->toc->foldername);
    argv[1] = MallocACopy(str);
    for (i = 0; i < mlist->nummsgs; i++) {
        (void) sprintf(str, "%d", mlist->msglist[i]->msgid);
        argv[2 + i] = MallocACopy(str);
    }
    argv[2 + i] = "-noinplace";
    argv[3 + i] = "-nowhatnowproc";
    argv[4 + i] = "-nodraftfolder";
    DoCommand(argv, (char *) NULL, "/dev/null");
    for (i = 1; i < 2 + mlist->nummsgs; i++)
        XtFree((char *) argv[i]);
    XtFree((char *) argv);
    RenameAndCheck(draftFile, MsgFileName(msg));
    RestoreDraft();
    TocSetCacheValid(mlist->msglist[0]->toc);
				/* If -anno is set, this keeps us from
				   rescanning folder. */
    TocSetCacheValid(msg->toc);
            MsgCreateSource(msg, TRUE,scrn);
    msg->startPos = (*msg->source->Scan)(msg->source, 0, XmSELECT_LINE,
					 XmsdRight,
					 1, FALSE);

    if (defAnnotateForwards) {
	msg->anno = XtNew(AnnoRec);
	msg->anno->type = "Forwarded";
	msg->anno->num = mlist->nummsgs;
	msg->anno->list = (MsgHandle *)
	    XtMalloc((unsigned) mlist->nummsgs * sizeof(MsgHandle));
	for (i=0 ; i<mlist->nummsgs ; i++)
	    msg->anno->list[i] = MsgGetHandle(mlist->msglist[i]);
    }

    if (scrn->ddifheaders && XtIsManaged(scrn->ddifheaders))
        XmTextSetEditable(scrn->ddifheaders,True);
}


/* Load msg with a copy of frommsg. */

void MsgLoadCopy(msg, frommsg)
Msg msg, frommsg;
{
    char str[500];
    (void)strcpy(str, MsgFileName(msg));
    CopyFileAndCheck(MsgFileName(frommsg), str);
    TocSetCacheValid(msg->toc);
}

/*
 * Load a message with blank headers whose initial contents are the given file.
 * In particular, the file might be a DDIF file.  Return TRUE on success.
 */

Boolean MsgLoadFromFile(msg, filename,scrn)
Msg msg;
char *filename;
Scrn scrn;
{
    int fromfid, tofid;
    char buf[512], *ptr;
    register int length;
    XmTextWidget tw;

    ptr = (char *)NULL;
    fromfid = myopen(filename, O_RDONLY, 0666);
    if (fromfid < 0) return FALSE;
    MsgLoadComposition(msg);
    tofid = OpenAndCheck(MsgFileName(msg), O_RDWR, 0666);
    tw = (XmTextWidget) scrn->viewwidget;
    if (DDIFFileCheck(fromfid)) {
        if (!scrn->ddifpane )
           MakeDDIFWidgets(scrn);
        tw = (XmTextWidget) scrn->ddifheaders;
	length = GetFileLength(MsgFileName(msg));
	ptr = XtMalloc(length + 2);
	if (length != read(tofid, ptr, length))
	    Punt("Error reading in MsgLoadFromFile!");
	ptr[length] = 0;
	length--;
	while (ptr[length] == '\n' || ptr[length] == '-' ||ptr[length] == ' '){
	    ptr[length] = 0;
	    length--;
	}
	(void) strcat(ptr, "\n");

	msg->source = XtCreateEDiskSourceFromString(ptr, TRUE, scrn, tw);
	msg->mm = capsar_create(filename, NULL);
	msg->msgtype = MTddif;
    } else {
	LSeekAndCheck(tofid, 0, L_XTND);
	while (0 < (length = read(fromfid, buf, 512)))
	    WriteAndCheck(tofid, buf, length);
    }
    (void) myclose(fromfid);
    (void) myclose(tofid);
    TocMsgChanged(MsgGetToc(msg), msg);
    if (ptr != NULL)
        XtFree(ptr);
    return TRUE;
}



/* Checkpoint the given message. */

void MsgCheckPoint(msg)
Msg msg;
{
    if (msg && msg->source && msg->msgtype == MTtext) {
	XtEDiskMakeCheckpoint(msg->source);
	TocSetCacheValid(msg->toc);
    }
}


/*
 * XtMalloc a new message record.  The returned msg rec is bzero'd.  Attempts
 * to avoid some of the overhead of normal XtMalloc.  All message records MUST
 * be malloced using this routine.
 */

static Msg FirstFreeMsg = NULL;

#define NUMMALLOC	((int) (1000 / sizeof(MsgRec)))

Msg MsgMalloc()
{
    static MsgRec *msgheap = NULL;
    static int msgptr = 0;
    Msg result;
    if (FirstFreeMsg != NULL) {
	result = FirstFreeMsg;
	FirstFreeMsg = (Msg) (FirstFreeMsg->handle);
	result->handle = (MsgHandle) NULL;
	return result;
    }
    if (msgheap == NULL || msgptr >= NUMMALLOC) {
	if (msgheap == NULL)
	    DEBUG(("Messages currently take %d bytes.\n",sizeof(MsgRec)));
	msgheap = (MsgRec *) XtMalloc((unsigned) (NUMMALLOC * sizeof(MsgRec)));
	bzero((char *) msgheap, NUMMALLOC * sizeof(MsgRec));
	msgptr = 0;
    }
    return &(msgheap[msgptr++]);
}
    

/*
 * Free the storage being used by the given msg.  All message records MUST be
 * freed using this routine.
 */

void MsgFree(msg)
Msg msg;
{
    register int i;
    if (msg->handle) {
	msg->handle->msg = NULL;
    }
    if (msg->curbuf != msg->origbuf) XtFree(msg->curbuf);
    XtFree(msg->origbuf);
    if (msg->mm) capsar_Destroy(msg->mm);
    if (msg->anno) {
	for (i=0 ; i<msg->anno->num ; i++)
	    MsgFreeHandle(msg->anno->list[i]);
	XtFree((char *) msg->anno->list);
	XtFree((char *) msg->anno);
    }

    bzero((char *) msg, sizeof(MsgRec));
    msg->handle = (MsgHandle) FirstFreeMsg;
    FirstFreeMsg = msg;
}



/*
 * Remove the given message from the given sequence.
 */

void MsgRemoveFromSeq(msg, seqname)
Msg msg;
char *seqname;
{
    Sequence seq;
    MsgList mlist;
    register int i;
    if (msg == NULL) return;
    seq = TocGetSeqNamed(msg->toc, seqname);
    if (seq != NULL && seq->mlist != NULL) {
	mlist = seq->mlist;
	for (i=0 ; i<mlist->nummsgs ; i++) {
	    if (msg == mlist->msglist[i]) {
		mlist->nummsgs--;
		for (; i<mlist->nummsgs; i++)
		    mlist->msglist[i] = mlist->msglist[i+1];
		mlist->msglist[mlist->nummsgs] = 0;
		TocWriteSeqLists(msg->toc);
		TocRedoButtonPixmaps(msg->toc);
		return;
	    }
	}
    }
#ifdef OLDANDSLOW
    int i;
    char **argv, str[1028], str2[10];
    if (msg != NULL && seq != NULL && seq->mlist != NULL) {
	for (i=0 ; i<seq->mlist->nummsgs ; i++) {
	    if (msg == seq->mlist->msglist[i]) {
		argv = MakeArgv(7);
		argv[0] = markCmd;
		argv[1] = sprintf(str, "+%s", TocGetFolderName(msg->toc));
		argv[2] = "-sequence";
		argv[3] = seq->name;
		argv[4] = "-delete";
		argv[5] = "-nozero";
		argv[6] = sprintf(str2, "%d", MsgGetId(msg));
		DoCommand(argv, (char *) NULL, "/dev/null");
		XtFree((char *) argv);
		TocReloadSeqLists(msg->toc);
		TocRedoButtonPixmaps(msg->toc);
		break;
	    }
	}
    }
#endif OLDANDSLOW
}



/*
 * Return the source corresponding to this msg.
 */

XmTextSource MsgGetSource(msg)
Msg msg;
{
    return ((msg) ? msg->source : NULL);
}



/*
 * Get a handle on a message. 
 */

MsgHandle MsgGetHandle(msg)
Msg msg;
{
    if (msg->handle == NULL) {
	msg->handle = XtNew(MsgHandleRec);
	msg->handle->refcount = 0;
	msg->handle->msg = msg;
    }
    msg->handle->refcount++;
    return msg->handle;
}


/*
 * Done with a message handle.
 */

void MsgFreeHandle(handle)
MsgHandle handle;
{
    handle->refcount--;
    if (handle->refcount <= 0) {
	if (handle->msg)
	    handle->msg->handle = NULL;
	XtFree((char *) handle);
    }
}

/*
 * Get the msg that a handle is pointing to.
 */

Msg MsgFromHandle(handle)
MsgHandle handle;
{
    return (handle) ? handle->msg : NULL;
}


/*
 * Get a ddif file for the given ddif message.  Returns data in a newly
 * malloced string, or NULL if not a ddif message.
 */

char *MsgGetDDIFFile(msg)
Msg msg;
{
    if (MsgGetMsgType(msg) != MTddif) return NULL;
    return GetDDIFFileFromMMPtr(GetMMPtr(msg));
}



/*
 * Initialize things.  Right now, that just means to initialize capsar.
 */

void InitMsg()
{
#ifndef IGNOREDDIF
/* PJS: Just in case... */
#ifndef NO_LOGGING
#define NO_LOGGING	(0)
#define STDERR_LOGGING  (1)
#endif  NO_LOGGING
    if (debug)
	capsar_set_log_type(STDERR_LOGGING);
    else
	capsar_set_log_type(NO_LOGGING);
#endif IGNOREDDIF
}

static Boolean returnValue;
static Boolean cant_continue;
static void DoImage()
{
    cant_continue = False;
    returnValue = True;
}
static void DoText()
{
    cant_continue = False;
    returnValue = False;
}

 Widget DXMailConfirmBox = NULL;

static Boolean confirmDisplay(msg)
char *msg;
{
    static XtCallbackRec yesCallback[2] ={(XtCallbackProc)DoImage, NULL, NULL};
    static XtCallbackRec noCallback[2] = {(XtCallbackProc)DoText, NULL, NULL};

    static Arg args[] = {
	{XmNmessageString, (XtArgVal) NULL},
	{XmNcancelLabelString, (XtArgVal) NULL},
	{XmNokCallback, (XtArgVal) yesCallback},
	{XmNcancelCallback, (XtArgVal) noCallback},
	{XmNdialogStyle, (XtArgVal) XmDIALOG_APPLICATION_MODAL},
    };
    XEvent event;
    XmString label;
    static Widget confirmBox = (Widget) NULL;
    int i;
    Scrn scrn;

    label = XmStringCreateSimple(msg);
    args[0].value = (XtArgVal) label;

    scrn = NULL;
    for (i = 0; i < numScrns && scrn == NULL; i++) 
	if (scrnList[i]->mapped && strcmp(scrnList[i]->name, "read") == 0)
		scrn = scrnList[i];
    for (i = 0; i < numScrns && scrn == NULL; i++) 
	if (scrnList[i]->mapped)
		scrn = scrnList[i];

    if ( DXMailConfirmBox == (Widget) NULL )
       confirmBox = (Widget) NULL;

    if (confirmBox == (Widget) NULL) {
	args[1].value = (XtArgVal) XmStringCreateSimple("No");
	confirmBox = XmCreateQuestionDialog(scrn->parent, "confirm",
		args, XtNumber(args));
	XtUnmanageChild(XmMessageBoxGetChild(confirmBox, XmDIALOG_HELP_BUTTON));
        DXMailConfirmBox = GetParent(confirmBox);
    } else {
	XtSetValues(confirmBox, args, XtNumber(args));
    }

    XtManageChild(confirmBox);
    XtRealizeWidget(GetParent(confirmBox));
    XtPopup(GetParent(confirmBox), XtGrabNonexclusive);
    XmStringFree(args[0].value);

    cant_continue = True;
    while (cant_continue) {
	XtNextEvent(&event);

#ifdef IGNOREDDIF
	XtDispatchEvent(&event); 
#else
/* Precede all calls to XtDispatchEvent() with a call to XDPSDispatchEvent(): */
	if(!(dps_exists && XDPSDispatchEvent(&event)))
	  XtDispatchEvent(&event);
#endif /* IGNOREDDIF */	

    }
    return returnValue;
}

Boolean checkForPS(file)
FILE *file;
{
    int i;
    char *buf;

    for (i = 0; i < PSLIMIT; i++) {
	if ((buf = ReadLineWithCR(file)) == NULL) break;
	if (strncmp(buf, "%!", 2) == 0) {
	    rewind(file);
	    return True;
	}
    }
    rewind(file);
    return False;
}
static char *splitMessage(in, to)
FILE *in;
char *to;
{
    FILE *out;
    char *buf;
    char *header = XtMalloc(1);
    Boolean foundHeaderEnd = False;
    int headerSize = 1;

    header[0] = '\0';
    out = myfopen(to, "a");
    if (out == NULL) {
	return NULL;
    }

    for(;;) {
	if ((buf = ReadLineWithCR(in)) == NULL) break;
	if (!foundHeaderEnd && strncmp(buf, "%!PS", 4) == 0) {
	    foundHeaderEnd = True;
	}
	if (foundHeaderEnd) {
	    (void) fwrite(buf, 1, strlen(buf), out);
	} else {
	    header = XtRealloc(header, headerSize + strlen(buf));
	    strcat(header, buf);
	    headerSize += strlen(buf);
	}
    }
    (void) myfclose(out);
    return header;
}
void MsgSetMsgText(scrn, msg)
Scrn scrn;
Msg msg;
{
    if (!msg) return;
    if (msg->msgtype == MTtext) return;
    SetScrn(msg, NULL, False);
    msg->msgtype = MTtext;
    SetScrn(msg, scrn, True);
#ifdef notdef
    if (msg->temporary) {
	(void) unlink(MsgFileName(msg));
	msg->temporary = False;
    }
    if (msg->msgtype == MTps) {
	MsgDestroyPS(msg);
	XtDestroyEDiskSource(msg->source);
	msg->source = NULL;
    } else {
	MsgDestroySource(msg);
    }
    msg->msgtype = MTtext;
    
    FreeDDIFInfo(scrn->ddifinfo);
    scrn->ddifinfo = NULL;
    MsgCreateSource(msg, False,scrn);
    RedisplayMsg(scrn);
#endif
}

#define MAXEDIT 10

Custom_Edit custom_editor[MAXEDIT];

static   Scrn cur_scrn_edit;
static   int forkpid;

void ExecCustomEditor(w)
Widget w;
{
  MsgComposeInCustomEditor(w);
}  

static void MsgComposeInCustomEditor(w)
Widget w;
{
    Msg msg;
    char *ptr;
    char *tmp;
    int length;
    char buffer[384], info[512], newbuf[512];
    char **argv;
    int fid;
    int i, j,ret_val;
    struct sigaction *sig_params;
    XmTextPosition position, lastposition;
    char        tmp_name[256];

    cur_scrn_edit = ScrnFromWidget(w);

    for (i=0; i < MAXEDIT; i++)
      if(!  custom_editor[i].edit_pid )
	break;
    if (i >= MAXEDIT){
      Warning(cur_scrn_edit->parent, NULL, "Exceeded MAX allowed edits");
      return ;
    }

    (void) sprintf(custom_editor[i].FileName, "/tmp/dxmailEditXXXXXX");
    
/* *\    Problem with Insight on "mktemp" ... No other reason.
 * *    (void) mktemp(custom_editor[i].FileName);
\* */
    strcpy(tmp_name,custom_editor[i].FileName);
    mktemp(&tmp_name[0]);
    strcpy(custom_editor[i].FileName,tmp_name);

    custom_editor[i].scrn_edit = cur_scrn_edit;
    custom_editor[i].widget_edit = custom_editor[i].scrn_edit->viewwidget;
    custom_editor[i].compose_widget = w;
    custom_editor[i].edit_pid = TRUE;
    TRACE(("@MsgComposeInCustomEditor\n"));
    /*    _XmTextDisableRedisplay(widget_edit, TRUE);  */
    msg = custom_editor[i].scrn_edit->msg;
    if (msg && msg->source) {
      XtEDiskSaveFile(msg->source, custom_editor[i].scrn_edit->viewwidget);
/*
 *      ptr = XtEDiskGetValue(msg->source);
 */
      ptr = XmTextGetString(custom_editor[i].scrn_edit->viewwidget);
      length = strlen(ptr);
      fid = myopen(custom_editor[i].FileName,O_WRONLY|O_TRUNC|O_CREAT,0600); 
      if (write(fid, ptr, sizeof(char)*length ) != length)
        Punt("Error in editor write!");
      XtFree(ptr);
      (void)myclose(fid);

/*
 *    (void) sprintf(buffer, defChoiceEditor,
 *                   DisplayString(theDisplay), custom_editor[i].FileName);
 */

      (void) sprintf(buffer, defChoiceEditor, custom_editor[i].FileName);
      (void) sprintf(newbuf, "%s%s%s%s", "setenv DISPLAY ",
                     DisplayString(theDisplay), "; ", buffer );

#ifdef NOTDEFINED
      FuncChangeEnabled( (XtCallbackProc)ExecCustomEditor , 
			custom_editor[i].scrn_edit, False);
      FuncChangeEnabled( (XtCallbackProc) ExecCloseScrn , 
			custom_editor[i].scrn_edit, False);
      FuncChangeEnabled( (XtCallbackProc)ExecCompReset , 
			custom_editor[i].scrn_edit, False);
      FuncChangeEnabled( (XtCallbackProc)ExecSendDraft,  
			custom_editor[i].scrn_edit, False);
      FuncChangeEnabled( (XtCallbackProc)ExecSaveDraft,  
			custom_editor[i].scrn_edit, False);
#endif
      position = 0;
/*      lastposition = XmTextGetLastPosition(custom_editor[i].widget_edit); */
      lastposition = 0;
      (void)sprintf(info,"\n\n%s\n\n%s %s\n\n[%s]\n","***INFORMATION***",
	      "Draft being edited in", "user specified editor.", buffer);
      XmTextReplace(custom_editor[i].widget_edit, position, lastposition,info);
      XmTextSetEditable(custom_editor[i].widget_edit, False);
      argv = MakeArgv(5);
      argv[0] = "/bin/csh";
      argv[1] =  "-f ";
      argv[2] = "-c ";
      argv[3] = newbuf;
      argv[4] = 0;
      DoCommandInBackground(argv,  NULL,  NULL ,
			    (XtWorkProc) catch_sigchld, (Opaque) i);
      XtFree((char *)argv);
      XtUnmapWidget(custom_editor[i].scrn_edit->parent); 
   }
  }



static void SendbackEditedMessage(child_index)
int child_index;
{
 
  Msg msg;
  int		fd, rd;
  static char	*ptr = NULL;
  static int	ptrlen = 0;
  struct stat	buf;
  char *tmp;
  int length, i;
  char buffer[1028];
  Scrn vscrn;
  FILE *fid;
  XEvent *event;
  char **params;
  XmTextPosition position, lastposition;
  XmTextSource msg_source;

  i = child_index;

#ifdef NOTDEFINED 

  /* commented out since the editor proc unmaps this widget */
  FuncChangeEnabled( ExecCustomEditor , custom_editor[i].scrn_edit, True);
  FuncChangeEnabled( ExecCloseScrn , custom_editor[i].scrn_edit, True);
  FuncChangeEnabled( ExecCompReset , custom_editor[i].scrn_edit, True);
  FuncChangeEnabled( ExecSendDraft,  custom_editor[i].scrn_edit, True);
  FuncChangeEnabled( ExecSaveDraft,  custom_editor[i].scrn_edit, True);

#endif 

  XmTextSetEditable(custom_editor[i].widget_edit, True); 

  if (i >= MAXEDIT){
    Warning(cur_scrn_edit->parent, EFileCantOpen, 
	    "Lost custom editor changes");
    return ;
  }

  fd = OpenAndCheck(  custom_editor[i].FileName, O_RDONLY, 0);
  if (fd == -1) {
    buf.st_size = 0;
  } else {
    stat ( custom_editor[i].FileName, &buf);
  }
  if (buf.st_size <= 0) {
    DEBUG(("Empty source '%s' - setting to the empty string.\n",  custom_editor[i].FileName));
    if (ptrlen == 0) {
      ptr = XtMalloc(1024);
      ptrlen = 1024;
      *ptr = '\0';
    }
  } else {
    if (ptrlen == 0) {
      ptr = XtMalloc((unsigned)buf.st_size + 2);
      ptrlen = buf.st_size;
    } else {
      if (buf.st_size > ptrlen) {
	ptr = XtRealloc(ptr,(unsigned)buf.st_size + 2);
	ptrlen = buf.st_size;
      }
    }
    rd = read(fd, ptr, buf.st_size);
    if (rd != buf.st_size)
      DEBUG(("Failed to read all of the file '%s'!\n",  
	     custom_editor[i].FileName));
    ptr[rd] = '\0';

    if (fd != -1) myclose(fd);
    position = 0;
    XtMapWidget(custom_editor[i].scrn_edit->parent); 
    lastposition = XmTextGetLastPosition(custom_editor[i].widget_edit);
    XmTextReplace(custom_editor[i].widget_edit, position, lastposition, ptr);
    XmTextClearSelection(custom_editor[i].widget_edit);
    XmTextSetTopCharacter(custom_editor[i].widget_edit, 0);
    position += strlen(ptr);
    /* *\ Can't free And reuse.
     * *  XtFree(ptr);
    \* */
    lastposition = position;
  }

  custom_editor[i].FileName[0] = '\0';
  custom_editor[i].edit_pid = FALSE;
  custom_editor[i].widget_edit = NULL;
  custom_editor[i].scrn_edit = NULL;
  custom_editor[i].compose_widget = NULL;
  remove(custom_editor[i].FileName);
  return;
}


int  catch_sigchld(indx)
int indx;
{
  DEBUG(("Sending back edited message indx = %x \n",indx));
  SendbackEditedMessage(indx);
  return True;
}




static int CountLines(source, start, length)
XmTextSource source;
XmTextPosition start;
int length;
{
    XmSourceData data = source->data;
    int num_lines = 1;
    int seg_length;
    char *ptr;

  /* verify that the 'start' and 'length' parameters are reasonable */
    if (start + length > data->length)
       length = data->length - start;
    if (length <= 0) return num_lines;

  /* setup the variables for the search of new lines before the gap */
    ptr = data->ptr + start;
    seg_length = data->gap_start - ptr;
                                         
  /* make sure the segment length is not greater than the length desired */
    if (length < seg_length) seg_length = length;

  /* make sure the segment length is not less than 0, indicating
   * that start begins after gap_start.
   */
    if (seg_length < 0) seg_length = 0;

  /* search up to gap */
    while (seg_length--) {
        if (*ptr++ == '\012') ++num_lines;
    }

  /* check to see if we need more data after the gap */
    if (length > data->gap_start - (data->ptr + start)) {
       length -= data->gap_start - (data->ptr + start);
       ptr += data->gap_end - data->gap_start;

  /* continue search till length is completed */
       while (length--) {
           if (*ptr++ == '\012') ++num_lines;
       }
    }

    return num_lines;
}

