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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/lib/xtrap/XEMsgUtils.c,v 1.1.2.2 92/02/06 11:56:32 Jim_Ludwig Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
/*
 *
 *  CONTRIBUTORS:
 *
 *      Dick Annicchiarico
 *      Robert Chesler
 *      Dan Coutu
 *      Gene Durso
 *      Marc Evans
 *      Alan Jamison
 *      Mark Henry
 *      Ken Miller
 *
 */
/*
 * This file contains international messaging utilities for reporting
 * text.  XTrap is initially take advantage of only a subset of this
 * module.
 */
#include <stdio.h>

#ifdef FUNCTION_PROTOS
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef vms
#include <descrip.h>
static int      cat_handle;             /* Catalog handle */
#else   /* vms */
#include <nl_types.h>
extern nl_catd  catopen();
extern int      catclose();
extern char     *catgets();
static char     *def_message = "Undefined error condition";
static nl_catd  cat_handle;             /* Catalog handle */
#endif  /* vms */


static int      cat_set;                /* Catalog message base */

#define MAX_CAT_DIR     16      /* Max size of X/Open message directive */
#define MAX_CAT_TEXT   256      /* Max size of X/Open message text */
#define MAX_ID          64      /* Max size of any identifier */
#define MAX_MSG_IDENT   32      /* Max size of message identifier */
#define MAX_MSG_TEXT   256      /* Max size of VMS message text */
#define MAX_FMT_TEXT   512      /* Max size of formatted output string */

#define MSG_OPTS    0xF         /* %FACIL-S-IDENT, Text */


/*
 *  m e s s a g e _ s e t
 *
 *  Function:   Sets the message file context used by all the primitive
 *              messaging routines.  A null parameter will leave the
 *              respective message variable unchanged.  Management of
 *              multiple message files is left to the caller.  If one
 *              is careless, it is possible to open many files and lose
 *              track of which files were opened.  Only useful when using
 *              ULTRIX catalog files.  Null operations when called from VMS.
 */
void msg_set
#ifdef FUNCTION_PROTOS
(
    int  msg_handle,
    int  msg_set
)
#else
(msg_handle, msg_set)
    int  msg_handle;             /* Catalog handle */
    int  msg_set;                /* Message set/base */
#endif
{
    if (msg_handle != 0)
    {
#ifndef VMS
        cat_handle = (nl_catd)msg_handle;
#else
        cat_handle = msg_handle;        /* Just an int on vms */
#endif

    }
    if (msg_set != 0)
        cat_set = msg_set;
}

/*
 *  m e s s a g e _ o p e n
 *
 *  Function:   Opens message database.  Note that for ULTRIX the
 *              catalog defaults to message set starting at 1.
 *
 *  Return Values:
 *      1                   Always returned for VMS.
 *      Catalog Handle      When CatOpen is successful for ULTRIX.
 *      -1                  When Failing to open Catalog file.
 */

msg_open
#ifdef FUNCTION_PROTOS
(
    char *catalog_filename,
    int  *catalog_handle
)
#else
(catalog_filename,catalog_handle)
    char *catalog_filename;
    int  *catalog_handle;
#endif

#ifdef VMS
                    /* m e s s a g e _ o p e n  (VMS) */
{
    return(1);
}
#else
                    /* m e s s a g e _ o p e n  (non-VMS) */
{
    cat_set = 1;
    cat_handle = catopen(catalog_filename, 0);
    *catalog_handle = (int)cat_handle;
    if ((int)cat_handle == -1)
        return(0);

    return (1);
}
#endif

/*
 *  m e s s a g e _ c l o s e
 *
 *  Function:   Closes message database.
 *
 *  Return Values:
 *       0     When CatClose (and msg_close) is successful.
 *      -1     When Failing to Close Catalog file.
 */

msg_close
#ifdef FUNCTION_PROTOS
(void)
#else
()
#endif

#ifdef VMS
                    /* m e s s a g e _ c l o s e  (VMS) */
{
    return(1);
}

#else
                    /* m e s s a g e _ c l o s e  (non-VMS) */
{
    int status;
    status = catclose(cat_handle);
    if (status != 0)
        return (status);

    return (1);
}

#endif

/*
 *  m e s s a g e _ f o r m a t
 *
 *  Function:   Fetches message from database and formats message.
 *
 *  Inputs:     mtext - Address of buffer for formatted message
 *              msgid - message ID
 *              ap - Pointer to a variable parameter list of type va_list.
 *
 *  Outputs:    mtext
 */

int msg_format
#ifdef FUNCTION_PROTOS
(   char *mtext,
    long msgid,
    va_list ap
)
#else
(mtext, msgid, ap)
    char    *mtext;     /* Formatted return string */
    long    msgid;      /* Message id */
    va_list ap;
#endif

#ifdef VMS
                    /* m e s s a g e _ f o r m a t  (VMS) */
{
    struct dsc$descriptor
                    ctrstr,     /* Descriptor for stored text of message */
                    outbuf;     /* Descriptor for formatted text of message */
    long            flags;      /* Message flags */
    unsigned short  outlen;     /* Length of formatted message */
    long            status;
    char            msg_text[MAX_MSG_TEXT];

    ctrstr.dsc$w_length     = sizeof(msg_text)-1;
    ctrstr.dsc$b_dtype      = DSC$K_DTYPE_T;    /* Text */
    ctrstr.dsc$b_class      = DSC$K_CLASS_S;    /* Static */
    ctrstr.dsc$a_pointer    = msg_text;         /* Point at local buffer */

    flags = MSG_OPTS;           /* %FAC-S-IDENT, Text */

    status = SYS$GETMSG(msgid, &ctrstr.dsc$w_length, &ctrstr, flags, 0);
    if ((status & 1) == 0)
    {
        /*  Get the NOMESSAGE system text */
        status = SYS$GETMSG(0, &ctrstr.dsc$w_length, &ctrstr, flags, 0);
    }

    outbuf.dsc$w_length     = MAX_FMT_TEXT-1;   /* Assume user buf fits max */
    outbuf.dsc$b_dtype      = DSC$K_DTYPE_T;    /* Text */
    outbuf.dsc$b_class      = DSC$K_CLASS_S;    /* Static */
    outbuf.dsc$a_pointer    = mtext;            /* Point at user buffer */

    /*  Format the message string.
     *  Note that the SYS$FAOL call could accvio if the
     *  directives of the message fetched does not match
     *  the ap parameters passed in.
     *  Don't know how to prevent or detect this situation.
     */
    status = SYS$FAOL(&ctrstr, &outlen, &outbuf, ap);
    if ((status & 1) == 0)
    {
        mtext[0] = '\0';
        return(status);
    }
    mtext[outlen] = '\0';

    return(1);
}

#else
                   /* m e s s a g e _ f o r m a t  (non-VMS) */
{
    char *msg_text;     /* Ptr to message text (storage owned by catgets) */

    /*
     *  This always will be successful.
     *  If the given message Id can't be found,
     *  the function returns a pointer to a string
     *  that says the message was invalid.
     */
    msg_text = catgets(cat_handle, cat_set, msgid, def_message);

    /*  Format the message string returned by catgets() */
    vsprintf(mtext, msg_text, ap);

    return(1);
}

#endif

/*
 *  m e s s a g e _ s p r i n t
 *
 *  Function:   Fetches message from database and formats message.
 *
 *  Inputs:     mtext - Address of buffer for formatted message
 *              msgid - message ID
 *              variable arguments - string format parameters
 *
 *  Outputs:    mtext and status of 1
 *              or no mtext and status not equal to 1.
 */

int msg_sprint
#ifdef FUNCTION_PROTOS
(   char *mtext,
    long msgid,
    ...
)
#else
(mtext, msgid, va_alist)
    char    *mtext;         /* Formatted return string */
    long    msgid;          /* Message id */
    va_dcl                  /* Variable argument list pointer */
#endif
                    /* m e s s a g e _ s p r i n t */
{
    int status;
    va_list ap;

#ifdef FUNCTION_PROTOS
    va_start(ap,msgid);
#else
    va_start(ap);
#endif

    status = msg_format(mtext, msgid, ap);
    va_end(ap);

    return(status);
}

/*
 *  m e s s a g e _ p r i n t
 *
 *  Function:   Fetches message from database, then formats and prints message.
 *
 *  Inputs:     msgid - message ID
 *              variable arguments - string format parameters
 *
 *  Outputs:    message printed to stderr.
 */

int msg_print
#ifdef FUNCTION_PROTOS
(   long msgid,
    ...
)
#else
(msgid, va_alist)
    long msgid;                  /* Message id */
    va_dcl
#endif
                    /* m e s s a g e _ p r i n t */
{
    int     status;
    char    mtext[MAX_FMT_TEXT];    /* Formatted message text */
    va_list ap;                     /* Pointer to variable argument list */

#ifdef FUNCTION_PROTOS
    va_start(ap,msgid);
#else
    va_start(ap);
#endif

    status = msg_format(mtext, msgid, ap);
    if (status == 1)
        printf("%s\n", mtext);

    va_end(ap);

    return(status);
}

/*
 *  m e s s a g e _ f p r i n t
 *
 *  Function:   Fetches message from database, then formats and prints message.
 *
 *  Inputs:     fid - file handle of file for output message
 *              msgid - message ID
 *              variable arguments - string format parameters
 *
 *  Outputs:    message printed to file indicated by fid.
 */

int msg_fprint
#ifdef FUNCTION_PROTOS
(   FILE *fid,
    long msgid,
    ...
)
#else
(fid, msgid, va_alist)
    FILE    *fid;                   /* File handle */
    long    msgid;                  /* Message id */
    va_dcl
#endif
                    /* m e s s a g e _ f p r i n t */
{
    int status;
    va_list ap;
    char    mtext[MAX_FMT_TEXT];     /* Formatted message text */

#ifdef FUNCTION_PROTOS
    va_start(ap,msgid);
#else
    va_start(ap);
#endif

    status = msg_format(mtext, msgid, ap);
    if (status == 1)
        fprintf(fid, "%s\n", mtext);

    va_end(ap);

    return(status);
}

/*
 *  m e s s a g e _ b u i l d
 *
 *  Function:   Allocates a text buffer, fetches message from the database
 *              and formats the message.
 *
 *  Inputs:     msgid - message ID
 *              variable arguments - string format parameters
 *
 *  Outputs:    Allocated and formatted mtext string or 0.
 */

char * msg_build
#ifdef FUNCTION_PROTOS
(   long msgid,
    ...
)
#else
(msgid, va_alist)
    long    msgid;          /* Message id */
    va_dcl                  /* Variable argument list pointer */
#endif
                    /* m e s s a g e _ s p r i n t */
{
    int status;
    va_list ap;
    char *mtext = (char *)calloc(MAX_FMT_TEXT,sizeof(char));

#ifdef FUNCTION_PROTOS
    va_start(ap,msgid);
#else
    va_start(ap);
#endif

    status = msg_format(mtext, msgid, ap);
    if (status != 1)
    {
        cfree(mtext);
        return(0);
    }
    va_end(ap);

    return(mtext);
}
