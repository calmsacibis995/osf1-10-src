/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      Notepad -- DECwindows simple out-of-the-box editor.
**
**  AUTHOR:
**
**      Joel Gringorten  - November, 1987
**
**  ABSTRACT:
**
**      Piece Source - This module implements a piece table source with
**  	an infinite undo stack and journalling.  
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**
**
**      V1-001  JMG0001         J. M. Gringorten            23-Oct-1987
**              Initial version.
**
**--
*/
/*  [bl] 25-Oct-93  (Refer Note ootb 128)
*      
*   SetSelection(): XtOwnSelection() has been added to with Convert() as the
*		    conversion callback.
*
*   [bl] 23-Nov-93 (Refer DecNotes ootb_bugs 213)
*
*   PSReplace(): Store the length of data in data->ao->data->length
*                before invoking Replace()
*
*
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: [ps.c,v 1.5 91/08/17 06:00:23 rmurphy Exp ]$";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#include <ctype.h>
#include "notepad.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/ScrolledW.h>
#define BOGUS_MB_MAX
#include <Xm/XmosP.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

 
static void SetSelection();
void PSCountTotalLines();

#define PieceSize(qp)  (qp->endPos - qp->begPos)

typedef struct _ChangeHeader
{
    int begPos;
    int delCnt;
    int insCnt;
} ChangeHeader;

typedef struct _qp
{
    struct _qp *flink, *blink;
    XmTextSourceRec **src;
    int begPos, endPos;
} PieceQueueElement;

typedef struct _PSContextRec
{
    XmTextSource ro;		/* Backpointer to source record. */
    XmTextWidget *widgets;	/* Array of widgets displaying this source. */
    XmTextPosition left, right; /* Left and right extents of selection. */
    char * ptr;			/* Actual string data. */
    char * value;		/* Value of the string data. */
    char * gap_start;		/* Gapped buffer start pointer */
    char * gap_end;		/* Gapped buffer end pointer */
    char * PSWC_NWLN;           /* Holder for char*, short*, int* rep of NWLN */
    int length;			/* Number of chars of data. */
    int maxlength;		/* Space allocated. */
    int old_length;		/* Space allocated for value pointer. */
    int numwidgets;		/* Number of entries in above. */
    int maxallowed;		/* The user is not allowed to grow source */
				/* to a size greater than this. */
    Time prim_time;             /* time of primary selection */
    Boolean hasselection;	/* Whether we own the selection. */
    Boolean editable;		/* Whether we allow any edits. */
/*
** NOTE: above here is a copy of _XmSourceDataRec with one change.  The field
** "source" is renamed "ro" to make it match the conventions in older Notepad
** code.  The name "source" is preempted by stuff below.   rjs 8 Feb 1993
*/
    XmTextSource ao, co, source;
    PieceQueueElement *PTQHead, *PTQTail, *lastAddedPiece, *LocatedPiece;
    XmTextPosition endPos, aoPos, coPos, lastReplaceEndPos, LocatedPiecePos;
    void (*applicationCallback)(); /* tell an application that we've changed */
    FILE *journal;	/* journal FILE -- also flag to indicate journalling */
    int undoing;		/* if set, we're in an undo/redo dialog */
    int inhibitJournal; /* flag to tell replace was called by undo/redo*/
    int undoIndex;	/* file offset of last journal entry that was undone */
    int jnlEOF;		/* last valid position in journal file */
    int changes;	/* used by compression algorithm */
    int _changesUntilCompress;
    PieceQueueElement *compressQP;
} PSContext;

#define COMPRESSABLE_LENGTH 40
#define TEXT_INITIAL_INCREM 64

PSContext *ctx_public;  /* debug hack **************************XXX */

#define EraseInsertionPoint(widget)\
{\
    (*widget->text.output->DrawInsertionPoint)(widget,\
					 widget->text.cursor_position, off);\
}

#define TextDrawInsertionPoint(widget)\
{\
    (*widget->text.output->DrawInsertionPoint)(widget,\
					 widget->text.cursor_position, on);\
}


static PieceQueueElement *getQp()
{
    return((PieceQueueElement *)XtCalloc(sizeof(PieceQueueElement), 1));
}

static PieceQueueElement *remqueue(head, tail, this)
  PieceQueueElement **head, **tail, *this;
{
  PieceQueueElement *prev = this->blink;
  PieceQueueElement *next = this->flink;
	if (this == *head){
	  if(this == *tail) {
	    *head = 0;
	    *tail = 0;
	  } else {
	    *head = next;
	    next->blink = 0;
	  }
	} else {
	  if(this == *tail){
	    *tail = prev;
	    prev->flink = 0;   
	  } else {
	    prev->flink = next;
	    next->blink = prev;
	  }
	}
	this->blink = 0;
	this->flink = 0;
	return (this);
}

static freeQp(this, ctx)
  PieceQueueElement* this;
  PSContext *ctx;
{
    if (ctx->lastAddedPiece == this) ctx->lastAddedPiece = NULL;
    if (ctx->LocatedPiece == this) ctx->LocatedPiece = NULL;
    XtFree((XtPointer)this);
}

/*
 * insert 'new' queue element on the queue.  
 */
static PieceQueueElement *insert(head, tail, new, where)
  PieceQueueElement **head, **tail, *new, *where;
{
	if(!where){
	    new->flink = *head;
	    *head = new;
	} else {
	    new->flink = where->flink;
	    new->blink = where;
	    where->flink = new;
	}
	if(new->flink == 0)
	    *tail = new;
	else
	    new->flink->blink = new;
	return(new);
}

/*
 * Translate a source position into a piece pointer.  Also, return the offset
 * of that position within the piece.  
 */
static PieceQueueElement *locate(ctx, skew, pos)
  XmTextPosition pos;
  int *skew;
  PSContext *ctx;
{
/* referential locality  optimizations go here */
  XmTextPosition t;
  PieceQueueElement *qp;
/*printf("locate %d  %d\n", total++, pos); */
    if ((pos == ctx->endPos) && (ctx->PTQTail)){
	qp = ctx->PTQTail;
	*skew = PieceSize(qp);
        ctx->LocatedPiece = qp;
        ctx->LocatedPiecePos = pos - *skew;
	return qp;
	}
    if(	(ctx->LocatedPiece) && 
	(qp = ctx->LocatedPiece->flink) &&
	(pos >= (t = ctx->LocatedPiecePos + PieceSize(ctx->LocatedPiece))) && 
	(pos < PieceSize(qp) + t)){
	    /* do nothing more! */
    } else {
         t = 0;
        for(qp = ctx->PTQHead; qp; qp = qp->flink){
	    if (t + PieceSize(qp) > pos)
	        break;
	    t += (PieceSize(qp));
        }
    }
    *skew = pos - t;
    ctx->LocatedPiece = qp; 
    ctx->LocatedPiecePos = t;
    return qp;
}

/*
 * Split begin and end pieces and delete if necessary unused pieces. 
 * return queue pointer of piece to insert at. 
 */
static PieceQueueElement *deleteRange(ctx, startPos, endPos)
  PSContext *ctx;
  XmTextPosition startPos, endPos;
{
  void initiateCompressionSequence();
  PieceQueueElement *qp, *beg, *end, *victim;
  XmTextPosition t, range = endPos - startPos;
  int	skew;
    if(endPos == 0)
	return 0;
    if(ctx->_changesUntilCompress && 
			++ctx->changes >= ctx->_changesUntilCompress){
	ctx->changes = 0;
	initiateCompressionSequence(ctx);
    }
    qp = locate(ctx, &skew, startPos);
    t = startPos - skew;
    if (!skew){
	if( t + PieceSize(qp) == endPos){
	    /* desired range is contained in the one piece exactly */
	    beg = qp->blink;
	    ctx->endPos -= (PieceSize(qp));

	    if(qp == ctx->compressQP)  ctx->compressQP = 0;
	    freeQp(remqueue(&ctx->PTQHead, &ctx->PTQTail, qp), ctx);

	    return beg;
	} else if( t + PieceSize(qp) > endPos){
	    /* desired range is front portion of one piece */
	    beg = qp->blink;
	    qp->begPos += range;	  
	    ctx->endPos -= range;
	    return beg;
	}
    } else { 
	if( t + PieceSize(qp) == endPos){
	    /* range is the back portion of one piece */
	    qp->endPos -= range;
	    ctx->endPos -= range;
	    return qp;
	}
    }
    if(skew){
	if( t + PieceSize(qp) < endPos){
	    /* deleted range extends off this piece, so lop off end of it */
	    t += PieceSize(qp);
	    beg = qp->flink;
	    ctx->endPos -= PieceSize(qp) - skew;
	    qp->endPos = qp->begPos + skew;
	} else {
	    /*      enter a new piece by splitting the first piece */
	    beg = insert(&ctx->PTQHead, &ctx->PTQTail, getQp(), qp);
	    beg->endPos = qp->endPos;
	    qp->endPos = qp->begPos + (startPos - t);
	    beg->begPos = qp->endPos;
	    beg->src = qp->src;
            if (!range)
	        return (qp);
            t += PieceSize(qp); /* bump 't' up to beg*/
            if( t + PieceSize(beg) > endPos){
	        /* range size < sizof(beg) */
                ctx->endPos -= (endPos - t);
                beg->begPos += (endPos - t);
	        return qp;
	    }
	}
    } else 
	beg = qp;
/* locate queue entry that contains the last position, deleting others
								 on the way*/
    victim = 0;
    for(qp = beg; qp;  qp = qp->flink){
	if (victim){
	    ctx->endPos -= (PieceSize(victim));

	    if(victim == ctx->compressQP)  ctx->compressQP = 0;
	    freeQp(remqueue(&ctx->PTQHead, &ctx->PTQTail, victim), ctx);

	    victim = 0;
	}
	if( t + (PieceSize(qp)) >= endPos) 
	    break;
	t  += (PieceSize(qp));
	victim = qp;
    }
    if( t + (PieceSize(qp)) == endPos){
	end = qp->blink;
	ctx->endPos -= (PieceSize(qp));
    
        if(qp == ctx->compressQP)  ctx->compressQP = 0;
        freeQp(remqueue(&ctx->PTQHead, &ctx->PTQTail, qp), ctx);

	return (end);
    }
/* implied split and delete first half of last piece */
    ctx->endPos -= (endPos - t);
    qp->begPos += (endPos - t);
    return (qp->blink);
}

DoQ()
{
  XmTextBlockRec t, *text;
  int i;
  char c;
  int end, count;
    PieceQueueElement *qp;
    PSContext *ctx = ctx_public;
    text = &t;
    text->length = 1;
    count = 0;
    for(qp = ctx->PTQHead; qp; qp = qp->flink){
        printf("%d\t%d\t%s\t", qp->begPos, qp->endPos, (*qp->src == ctx-> ao) ?
	 "ao":"ro", *qp->src);
        end = (PieceSize(qp)>20)?(qp->begPos)+20:qp->endPos;
        for(i=qp->begPos; i<end; i++){
	    (*(*qp->src)->ReadSource)(*qp->src, i, i+1, text);
	    c = text->ptr[0];
	    if (isprint(c))
	        putchar(c);
	    else
	        printf("/%d/", c);
        }
    printf("\n");
    count++;
    }
    printf("%d  piece%c\n", count, (count>1)?'s':' ');
}
 
#define RightPiece(qp, s2Pos) 		\
{					\
    s2Pos += (PieceSize(qp));		\
    qp = qp->flink;			\
}
 
#define LeftPiece(qp, s2Pos)		\
{					\
    qp = qp->blink;			\
    if(qp)				\
        s2Pos -= (PieceSize(qp));	\
}

#define Increment(data, position, direction)\
{					\
    if (direction == XmsdLeft) {	\
	if (position > 0)		\
	    position--;			\
    }					\
    else				\
    {					\
	if (position < data->endPos)	\
	    position++;			\
    }					\
}

/*
 * fetch a position for the scanner.  
 */ 
typedef struct _pieceCache
{
	char *ptr;
	XmTextPosition pieceStart;
	XmTextPosition pieceEnd;
} pieceCache;

fillPieceCache(src, piece, pos, dir)
  XmTextSource src;
  pieceCache *piece;
  XmTextPosition pos;
  XmTextScanDirection dir;
{
  XmTextBlockRec text;
    if(dir == XmsdRight){
	(*src->ReadSource)(src, pos, MAXINT, &text);
        piece->ptr = text.ptr;
	piece->pieceStart = pos;
	piece->pieceEnd = pos + (text.length - 1);
    } else {
        (*src->ReadSource)(src, 0, pos, &text); /*left hand read!*/
        piece->ptr = text.ptr;
        piece->pieceStart = pos;
        piece->pieceEnd = pos + text.length - 1;
    }
}

static char Look(data, position, direction, piece)
PSContext *data;
XmTextPosition position;
XmTextScanDirection direction;
pieceCache *piece;
{
    
    XmTextBlockRec text;

    if (direction == XmsdLeft) {
	if (position == 0)
	    return(0);
	else
	    (*data->source->ReadSource)(data->source, position-1, position,&text);
	    return(*text.ptr);
    } else {
	if (position == data->endPos)
	    return(0);
	else {
            if(position > piece->pieceEnd)
		fillPieceCache(data->source, piece, position, direction);
	}
        return (piece->ptr[position - piece->pieceStart]);
    }
}

static getLastPosition(ctx)
  PSContext *ctx;
{
/*
  int t;
  PieceQueueElement *qp;
    t = 0;
    for(qp = ctx->PTQHead; qp; qp = qp->flink){
        t += (PieceSize(qp));
    }
    if (t != ctx->endPos){
        printf("piece queue length discrepancy %d <> %d\n", t, ctx->endPos);
        ctx->endPos = t;
    }
    return t;
*/
    return ctx->endPos;
}

/*
 * 	The scan operator
 */
static XmTextPosition PSScan(source, pos, sType, dir, count, include)
XmTextSource source;
XmTextPosition pos;
XmTextScanType sType;
XmTextScanDirection dir;
int count;
Boolean include;
{
    PSContext* data = (PSContext *)source->data;
    XmTextPosition position;
    int i, whiteSpace;
    char c;
    int ddir = (dir == XmsdRight) ? 1 : -1;
    pieceCache piece;

    position = pos;
    switch (sType) {
	case XmSELECT_POSITION: 
	    if (!include && count > 0)
		count -= 1;
	    for (i = 0; i < count; i++) {
		Increment(data, position, dir);
	    }
	    break;
	case XmSELECT_WHITESPACE: 
	case XmSELECT_WORD:
	    fillPieceCache(source, &piece, position, dir);
	    for (i = 0; i < count; i++) {
		whiteSpace = -1;
		while (position >= 0 && position <= data->endPos) {
		    c = Look(data, position, dir, &piece);
		    if ((c == ' ') || (c == '\t') || (c == '\n')){
		        if (whiteSpace < 0) whiteSpace = position;
		    } else if (whiteSpace >= 0)
			break;
		    position += ddir;
		}
	    }
	    if (!include) {
		if(whiteSpace < 0 && dir == XmsdRight)
		    whiteSpace = data->endPos;
		position = whiteSpace;
	    }
	    break;
	case XmSELECT_LINE: 
	    fillPieceCache(source, &piece, position, dir);
	    for (i = 0; i < count; i++) {
		while (position >= 0 && position <= data->endPos) {
		    if (Look(data, position, dir, &piece) == '\n')
			break;
		    if(((dir == XmsdRight) && (position == data->endPos)) || 
			(dir == XmsdLeft) && ((position == 0)))
			break;
		    Increment(data, position, dir);
		}
		if (i + 1 != count)
		    Increment(data, position, dir);
	    }
	    if (include) {
	    /* later!!!check for last char in file # eol */
		Increment(data, position, dir);
	    }
	    break;
	case XmSELECT_ALL: 
	    if (dir == XmsdLeft)
		position = 0;
	    else
		position = getLastPosition(data);
    }
    if (position < 0) position = 0;
    if (position > data->endPos) position = data->endPos;
    return(position);
}

static Boolean Compress(closure)
Opaque closure;
{
    PieceQueueElement *qp, *deleteMe;
    PSContext *ctx = (PSContext *) closure;
    int i,n, tempPos;
    XmTextBlockRec text;

    qp = ctx->compressQP;
    if(qp == ctx->PTQTail || qp == 0)
    {
	return TRUE;
    }
    i = 0;
    while(PieceSize(qp) <= COMPRESSABLE_LENGTH)
    {
	if(qp == ctx->lastAddedPiece)
	{
	    ctx->compressQP = qp->flink;
	    return FALSE;
	}
	i++;
	qp = qp->flink;
	if(!qp) break;
    }
    if( i < 2)
    {
	ctx->compressQP = qp->flink;
	return FALSE;
    }
    qp = ctx->compressQP;
    text.format = FMT8BIT;
    tempPos = ctx->coPos;

/*fprintf(stderr,"compressing %d\n", i);*/
    for(n=0; n<i; n++)
    {
	(*(*qp->src)->ReadSource)(*qp->src, qp->begPos, qp->endPos, &text);
	(*ctx->co->Replace)
	    (ctx->widgets[0], NULL, &ctx->coPos, &ctx->coPos, &text, True);
	ctx->coPos += text.length;
	deleteMe = qp;
	qp = qp->flink;
	if(n != 0) freeQp(remqueue(&ctx->PTQHead, &ctx->PTQTail, deleteMe), ctx);
    }

    ctx->compressQP->begPos = tempPos;
    ctx->compressQP->endPos = ctx->coPos;
    ctx->compressQP->src = &ctx->co;
    ctx->compressQP = qp;
    return FALSE;
}

static void initiateCompressionSequence(ctx)
  PSContext *ctx;
{
    ctx->compressQP = ctx->PTQHead;
    XtAddWorkProc(Compress, (Opaque)ctx);
}

/*
 * 	Read
 */ 
static int PSread(src, start, end, text)
XmTextSource src;
XmTextPosition start, end;
XmTextBlock text;
{
    int pieceSize, skew;
    PSContext *ctx = (PSContext *)src->data;  
    PieceQueueElement *qp;
    XmTextPosition endPos;

    if (start == end){
	text->length = 0;
	return(end);
    }

    qp = locate(ctx, &skew, start);

    if (end > ctx->endPos) end = ctx->endPos;
    if (!qp){
	text->length = 0;
	return (end);
    }

#if 0
    (*qp->src)->data->length = ctx->length;
#endif

    pieceSize = (PieceSize(qp)) - skew;
    if (pieceSize == 0){ 
	text->length = 0;
	return (end);
   }
    endPos = MIN((qp->begPos+skew+(end-start)), qp->endPos);
    (*(*qp->src)->ReadSource)(*qp->src, (qp->begPos)+skew, endPos, text);
    return(start+text->length); /* might not be koshur for non text srcs */
}

static resetTextHighlighting(data)
 PSContext *data;
{
  int i;
    for (i=0 ; i<data->numwidgets ; i++) {
	_XmTextDisableRedisplay(data->widgets[i], TRUE);
	if (data->hasselection)
	    XmTextSetHighlight(data->widgets[i], data->left, data->right,
				XmHIGHLIGHT_NORMAL);
    }
}

static fixupTextAndSelection(data, start,end,  delta)
  PSContext *data;
  XmTextPosition start, end;
  int delta;
{
  int i;
     if (data->hasselection) {
	if (data->left > start || (data->left == start && delta > 0))
	    data->left += delta;
	if (data->right > start || (data->right == start && delta < 0))
	    data->right += delta;
	if (data->left >= data->right) data->hasselection = FALSE;
    }
    for (i=0 ; i<data->numwidgets ; i++) {
	_XmTextInvalidate(data->widgets[i], start, end, delta);
	if (data->hasselection)
	    XmTextSetHighlight(data->widgets[i], data->left, data->right,
				XmHIGHLIGHT_SELECTED);
	_XmTextEnableRedisplay(data->widgets[i]);
    }
    if(data->applicationCallback)
        (*data->applicationCallback)();
}

static jnlWrite(file, buf, amount)
  FILE *file;
  int amount;
  char *buf;
{
  int status;
    status =  fwrite(buf, 1, amount, file);
    if (status < amount){
        perror("write");
	printf("journal write failed.  wrote %d, tried %d\n", status, amount);
    }
}

static int JOURNAL_EOF = -1;

static wrtDelayJnl(data)
  PSContext *data;
{
  XmTextBlockRec text;
  ChangeHeader ch;
  int backOffset;
  PieceQueueElement *qp = data->lastAddedPiece;
    if(!qp)
	return;
    fseek((data->journal), data->jnlEOF, 0);
/*printf("delay wrt Journal at %d\n", ftell(data->journal));  */
    ch.begPos = data->lastReplaceEndPos - PieceSize(qp);
    ch.delCnt = 0;
    ch.insCnt = PieceSize(qp);
    jnlWrite(data->journal, &ch, sizeof(ch));
    (*(*qp->src)->ReadSource)(*qp->src, qp->begPos, qp->endPos, &text);
    jnlWrite(data->journal, text.ptr, ch.insCnt);
    backOffset = sizeof(ch)+ch.insCnt;
    jnlWrite(data->journal, &backOffset, 4);
    data->jnlEOF = ftell(data->journal);
    jnlWrite(data->journal, &JOURNAL_EOF, 4); 
    fflush(data->journal);
}

static wrtDeleteJnl(data, start, end)
  PSContext *data;
  XmTextPosition start, end;
{
  ChangeHeader ch;
  XmTextBlockRec text;
  int amount = end-start;
  int backOffset;
  XmTextSourceRec *source = data->source;
  XmTextPosition pos;
    if(!amount)
	return;
    fseek((data->journal), data->jnlEOF, 0);
/*printf("delete Journal at %d\n", ftell(data->journal)); */
    ch.begPos = start;
    ch.delCnt = amount;
    ch.insCnt = 0;
    jnlWrite(data->journal,&ch, sizeof(ch));
    for(pos = start; pos < end;){
        pos = (*source->ReadSource)(source, pos, end, &text);
        if(text.length == 0)
            break;
        jnlWrite(data->journal, text.ptr, text.length);
    }
    backOffset = sizeof(ch)+ch.delCnt;
    jnlWrite(data->journal, &backOffset, 4);
    data->jnlEOF = ftell(data->journal);
    jnlWrite(data->journal, &JOURNAL_EOF, 4); 
    fflush(data->journal);
}

flushUndoneChanges(data)
  PSContext *data;
{
    data->undoing = FALSE;
    fseek(data->journal, data->undoIndex, 0);
    data->jnlEOF = data->undoIndex;
    jnlWrite(data->journal, &JOURNAL_EOF, 4); 
    fflush(data->journal);
}

#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
#ifdef ShouldWordWrap
#undef ShouldWordWrap
#endif
#define ShouldWordWrap(data, widget)	(data->wordwrap && \
       (!(data->scrollhorizontal && \
       (XtClass(XtParent(widget)) == xmScrolledWindowWidgetClass))) \
       && widget->text.edit_mode != (int)XmSINGLE_LINE_EDIT && !data->resizewidth)
#endif

static Boolean PSShouldWordWrap(widget)
XmTextWidget widget;
{
    OutputData data = widget->text.output->data;
    return (ShouldWordWrap(data, widget));
}

static Boolean PSTextScrollable(widget)
XmTextWidget widget;
{
    OutputData data = widget->text.output->data;
    return (data->scrollvertical &&
	    widget->text.edit_mode != (int) XmSINGLE_LINE_EDIT &&
	    XtClass(XtParent(widget)) == xmScrolledWindowWidgetClass);
}

void PSTextSetSource(widget, source, top_character, cursor_position)
Widget widget;
PSContext *source;
XmTextPosition top_character;
XmTextPosition cursor_position;
{
#if 0
    XmTextWidget tw = (XmTextWidget) widget;
    XmTextBlockRec block;
    Boolean changed_dest_visible = False;

    EraseInsertionPoint(tw);
    if (source == NULL)
    {
       _XmWarning(widget, MESSAGE2);
       return;
    }

   /* zero out old line table */
    block.ptr = NULL;
    block.length = 0;
    _XmTextUpdateLineTable(widget, 0, 0, &block, False);

    (*tw->text.source->RemoveWidget)(tw->text.source, tw);
    tw->text.source = (XmTextSource) source;
    tw->text.cursor_position = cursor_position;

    _XmTextMovingCursorPosition (tw, cursor_position);
    _XmTextResetClipOrigin (tw, cursor_position, False);

    top_character = (*source->Scan)
	(source, top_character, XmSELECT_LINE, XmsdLeft, 1, FALSE);

    tw->text.new_top = top_character;
    tw->text.top_character = 0;
    _XmTextInvalidate(tw, top_character, top_character, NODELTA);
    XmTextSetHighlight(widget, 0, MAXINT, XmHIGHLIGHT_NORMAL);
    (*tw->text.source->AddWidget)(tw->text.source, tw);
#else
    XmTextSetSource (widget, source, top_character, cursor_position);
    XmTextSetHighlight(widget, 0, MAXINT, XmHIGHLIGHT_NORMAL);
#endif
}

/*
 *	the Replace operator
 */
static XmTextStatus PSReplace(tw, event, start, end, text, mvcb)
XmTextWidget tw;
XEvent *event;
XmTextPosition *start, *end;
XmTextBlock text;
Boolean mvcb; 		/* Call_callbacks flag */
{
    int i, delta, delLength = *end - *start;
    PSContext *data = (PSContext *)tw->text.source->data;  
    PieceQueueElement *here;
    XmTextStatus status;

    if ((!data->editable) || (*start > data->endPos))
		return EditError;
    if(data->undoing && !data->inhibitJournal)
	flushUndoneChanges(data);
    data->LocatedPiece = 0;
    resetTextHighlighting(data);
    delta = text->length - delLength;
  /* write the data to the append source */
    if(text->length)
    {
	XmTextSource	src = tw->text.source;
	tw->text.source = data->ao;
        data->ao->data->length = data->length;

	status = (*data->ao->Replace)
	    (tw, event, &data->aoPos, &data->aoPos, text, mvcb);

	tw->text.source = src;
	data->length = data->ao->data->length;
/*	_XmStringSourceSetGappedBuffer(data, *start); */

	if(status != EditDone)
	{
	    printf ("PS Panic - can't write to Append Source!!!\n");
	    /* why isn't there a return (status) here?  rjs 9 Feb 1993 */
	}
    }

    /* if contiguous typing, then just extend open piece */
    if((!delLength) &&
	((*start) == data->lastReplaceEndPos) &&
	(data->lastAddedPiece))
    {
	here = data->lastAddedPiece;
        if(!((here->endPos == data->aoPos) && (here->src == &data->ao)))
	    printf("Replace optimize panic 1 \n");
	here->endPos += text->length;
    }
    else
    {
  /* wasn't contguous, so flush out previous contiguous change to journal */
        if(!data->inhibitJournal && data->lastReplaceEndPos > 0)
	    wrtDelayJnl(data);
	data->lastReplaceEndPos = -1;
  /* write out a journal record for a delete range, and delete the range*/
	if(!data->inhibitJournal)
	    wrtDeleteJnl(data, *start, *end);
        here = deleteRange(data, *start, *end);
  /* if nothing to add, then fix up screen and split */

        if(text->length == 0)
	{
            fixupTextAndSelection(data, *start, *end, delta);
	    for (i = 0; i < data->numwidgets; i++)
	    {
		if (data->widgets[0] && PSTextScrollable(data->widgets[0]) &&
		    !PSShouldWordWrap(data->widgets[0]) &&
		    text->length != 0)
		{
		    PSCountTotalLines (tw->text.source, *start, text->length);
		}
	    }
	    return (EditDone);
        }
  /* else add a new piece and map the data into it */
	here = insert(&data->PTQHead, &data->PTQTail, getQp(), here);
	data->lastAddedPiece = here;
        here->begPos = data->aoPos;
        here->endPos = data->aoPos + text->length;
        here->src = &data->ao;
    } 
  /* fix up screen, and tidy up */
    data->lastReplaceEndPos = *start + text->length;
    data->aoPos += text->length;
    data->endPos += text->length;
    fixupTextAndSelection(data, *start, *end,  delta);
    for (i = 0; i < data->numwidgets; i++)
    {
	if (data->widgets[0] && PSTextScrollable(data->widgets[0]) &&
	    !PSShouldWordWrap(data->widgets[0]) &&
	    text->length != 0)
	{
	    PSCountTotalLines (tw->text.source, *start, text->length);
	}
    }
    return (EditDone);
}

static PSsetLastPos(src, lastPos)
  XmTextSourceRec *src;
  XmTextPosition lastPos;
{
    PSContext *ctx = (PSContext *)src->data;
    ctx->endPos = lastPos;
}

static InitPieceTable(ctx)
  PSContext *ctx;
{
  XmTextSourceRec **ro = &(ctx->ro);
  PieceQueueElement *qp;
  XmTextPosition beg, end;
    beg = (*(*ro)->Scan)(*ro, 0, XmSELECT_ALL, XmsdLeft,  0,0);
    end = (*(*ro)->Scan)(*ro, 0, XmSELECT_ALL, XmsdRight, 0,0);
    if(beg != end){
        qp = insert(&ctx->PTQHead, &ctx->PTQTail, getQp(), 0);
        qp->begPos = beg;
        qp->endPos = end;
        qp->src = ro;
        ctx->endPos = PieceSize(qp);
    }
}

static Boolean Convert(w, seltype, desiredtype, type, value, length, format)
Widget w;
Atom *seltype;
Atom *desiredtype;
Atom *type;
XtPointer *value;
unsigned long *length;
int *format;
{
    XmTextWidget  widget = (XmTextWidget) w;
    PSContext *data = (PSContext *) widget->text.source->data;
    XmTextBlockRec block;
    XmTextPosition pos;
    int index;
    char *buf;
    Atom *a;
    Atom target;
    if (*seltype != XA_PRIMARY) return FALSE;
    
    if (*desiredtype == XA_STRING) {
        *type = (Atom) FMT8BIT;	
        *format = 8;
        if (data == NULL || !data->hasselection) { 
		return FALSE;
		}
        *length = data->right - data->left;
        buf = XtMalloc((unsigned) *length + 1);
        pos = data->left;    
        index = 0;
        while(pos < data->right){
	    pos = (*widget->text.source->ReadSource)(widget->text.source, pos, 
					data->right, &block);
           (void) memcpy(&buf[index], block.ptr, block.length);
	    index += block.length;
        }
        *value = buf;
     

        return TRUE;
    } else if (*desiredtype ==  
       (target = XInternAtom(XtDisplay(widget), "TARGETS", FALSE))) {
       /* I do hope that TARGETS gets built it... */
       *format = 32;
       a = (Atom *)XtMalloc((unsigned) (2*sizeof(Atom)));
       *value = (caddr_t)a;
       *a++ = XA_STRING;
       *a = target;
       *length = (2*sizeof(Atom)) >> 2; /*convert to work count */
       *type = XA_ATOM; /* or do we need a "list of atoms" type? */
       return TRUE;
    } else return FALSE;
}

/*ARGSUSED*/
static void LoseSelection(w, selection)
Widget w;
Atom *selection;
{
    XmTextWidget widget = (XmTextWidget) w;
    PSContext *data = (PSContext *)widget->text.source->data;
    Time	timestamp;
    timestamp = XtLastTimestampProcessed(XtDisplay(w));
    if (data && data->hasselection)
	(*data->source->SetSelection)(data->source, 1, -999, timestamp);
}

static void AddWidget(source, widget)
XmTextSource source;
XmTextWidget widget;
{
    Time timestamp; 
    PSContext *data = (PSContext *)source->data;
    data->numwidgets++;
    data->widgets = (XmTextWidget *)
	XtRealloc((char *) data->widgets,
		  (unsigned) (sizeof(XmTextWidget) * data->numwidgets));
    data->widgets[data->numwidgets - 1] = widget;
    timestamp = XtLastTimestampProcessed(XtDisplay(widget));
    if (data->hasselection && data->numwidgets == 1)
     {
	XtOwnSelection
	(
	    (Widget) data->widgets[0], XA_PRIMARY,
		timestamp, Convert, LoseSelection, 
			(XtSelectionDoneProc) NULL);
     }
    if (data->numwidgets > 1 &&
	PSTextScrollable(widget) &&
	!PSShouldWordWrap(widget))
	PSCountTotalLines(source, 0, data->endPos);
}

static void RemoveWidget(source, widget)
XmTextSource source;
XmTextWidget widget;
{
    PSContext *data = (PSContext *)source->data;
    Time	timestamp;
    int i;

    timestamp = XtLastTimestampProcessed(XtDisplay(widget));
    for (i=0 ; i<data->numwidgets ; i++)
    {
	if (data->widgets[i] == widget)
	{
	    data->numwidgets--;
	    data->widgets[i] = data->widgets[data->numwidgets];
	    if (i == 0 && data->numwidgets > 0 && data->hasselection)
		if (!XtOwnSelection((Widget) data->widgets[0], XA_PRIMARY,
				    CurrentTime, Convert, LoseSelection,
				    (XtSelectionDoneProc) NULL))
		    SetSelection (source, 1, 0, timestamp);
	    return;
	}
    }
}

static Boolean GetSelection(source, left, right)
XmTextSource source;
XmTextPosition *left, *right; 
{
    PSContext *data = (PSContext *)source->data;
    if (data->hasselection && data->left < data->right) {
	*left = data->left;
	*right = data->right;
	return TRUE;
    }
    data->hasselection = FALSE;
    return FALSE;
}

/**************************************************************************
* This function is called when the user does a selection. The function
* invokes XtOwnSelection() to grab the selection. Convert() is registered
* as a callback which will be called when XtGetSelectionValue() is 
* invoked by Cut or Copy option.
***************************************************************************/
/*
 * Function:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Notes:
 */
static void SetSelection(source, left, right, time)
XmTextSource source;
XmTextPosition left, right; 
Time time;
/*
 * Function: SetSelection()
 *           
 * Inputs:  text source, right and left position of the text selected
 *          and time of last event. 
 *
 * Outputs: Returns nothing
 *
 * Notes:   This function is invoked when the user makes a selection.
 *	    It sets the selection by calling SetSelection() of the
 *          widget and grabs the selection XA_PRIMARY by invoking 
 *	    XtOwnSelection(), passing Convert() as a callback procedure to
 *	    invoked by Cut and Copy operations
 */
{
    PSContext	*data = (PSContext *)source->data;
    (*data->ao->SetSelection) (source, left, right, time);
    if (data->hasselection && data->numwidgets == 1)  
     {
	XtOwnSelection
	(
	    (Widget) data->widgets[0], XA_PRIMARY,
		time, Convert, LoseSelection, 
			(XtSelectionDoneProc) NULL);
     }
}

static int CountLines(source, start, length)
  XmTextSource source;
  XmTextPosition start;
  long length;
{ 
    PSContext *data = (PSContext *)source->data;
    XmTextPosition position = start;
    int numlines = 0;
    int i;
    pieceCache piece;
    XmTextScanDirection dir = XmsdRight;
    char c;

    fillPieceCache(source, &piece, position, dir);
    for (position = start; position < start + length; position++) {
	c = Look(data, position, dir, &piece);
	if (c == '\0') return numlines;
	if (c == '\n') numlines++;
    }
    return (numlines);
}

void PSCountTotalLines(source, start, length)
XmTextSource source;
XmTextPosition start;
long length;
{
    XmSourceData data = source->data;
    int num_lines = 0;
    int i;

    if (length > 0) {
	num_lines = CountLines(source, start, length);
	for (i = 0; i < data->numwidgets; i++) {
	    XmTextWidget tw = (XmTextWidget) data->widgets[i];
	    tw->text.total_lines += num_lines;
     	} 
    } else if (length < 0) {
	length = -length;
	num_lines = CountLines(source, start, length);
	for (i = 0; i < data->numwidgets; i++) {
	    XmTextWidget tw = (XmTextWidget) data->widgets[i];
	    tw->text.total_lines -= num_lines;
	    if (tw->text.total_lines < 1)
		tw->text.total_lines = 1;
	}
    } else {
	length = ((XmTextWidget)data->widgets[0])->text.last_position;
	start = 0;
	num_lines = CountLines(source, start, length);
	for (i = 0; i < data->numwidgets; i++) {
	    XmTextWidget tw = (XmTextWidget) data->widgets[i];
	    tw->text.total_lines = num_lines;
        }
    }
}

/*** Public routines ***/
XmTextSource PSourceCreate (ro, ao, co, jnlFile, _changesUntilCompress)
XmTextSource ro, ao, co;
FILE *jnlFile;
int _changesUntilCompress;
{
    /*
    ** This routine needs to be cleaned up for wchars.  It should really
    ** look at the SourceCreate routine in Xm/textstrso.c.
    */
    XmTextSource src;
    PSContext *ctx;
    int     i;
    int	    char_size;

    switch (MB_CUR_MAX)
    {
    case 1:
    case 2:
    case 4:
	{
	    char_size = MB_CUR_MAX;
	    break;
	}
    case 3:
	{
	    char_size = 4;
	    break;
	}
    default:
	char_size = 1;
    }

    src = (XmTextSource) XtMalloc(sizeof(XmTextSourceRec));
    src->AddWidget = (AddWidgetProc) AddWidget;
    src->CountLines = (CountLinesProc) CountLines;
    src->RemoveWidget = (RemoveWidgetProc) RemoveWidget;
    src->ReadSource = (ReadProc) PSread;
    src->Replace = (ReplaceProc) PSReplace; 
    src->Scan = (ScanProc) PSScan;
    src->GetSelection = (GetSelectionProc) GetSelection;
    src->SetSelection = (SetSelectionProc) SetSelection;

    ctx = (PSContext *)(XtCalloc(sizeof(PSContext),1));
    ctx->source = src;
    ctx->length = 0;
    ctx->maxlength = TEXT_INITIAL_INCREM;
    ctx->ptr = XtCalloc((unsigned) ctx->maxlength,(unsigned) char_size);
    ctx->numwidgets = 0;
    ctx->widgets = (XmTextWidget *) XtMalloc((unsigned) sizeof(XmTextWidget));
    ctx->hasselection = FALSE;
    ctx->left = ctx->right = 0;
    ctx->editable = TRUE;
    ctx->maxallowed = MAXINT;
    ctx->ro = ro;
    ctx->ao = ao;
    ctx->co = co;
    ctx->journal = jnlFile;
    ctx->jnlEOF = 0;
    ctx->compressQP = 0;
    ctx->_changesUntilCompress = _changesUntilCompress;
    InitPieceTable(ctx);
    src->data = (XmSourceDataRec *) ctx;
    ctx->gap_start = ctx->ptr + (ctx->length * char_size);
    ctx->gap_end = ctx->ptr + ((ctx->maxlength - 1) * char_size);
ctx_public = ctx;
    return src;
}
 
PSsetROsource(src, ro)
  XmTextSourceRec *src, *ro;
{
    PSContext *ctx = (PSContext *)src->data;
    ctx->ro = ro;
}

PSourceDestroy(src)
  XmTextSourceRec *src;
{
PieceQueueElement *qp, *qp_victim;
    PSContext *ctx = (PSContext *)src->data;
    qp = ctx->PTQHead;
    while(qp){
	qp_victim = qp;
	qp = qp->flink;
	XtFree((XtPointer)qp_victim);
    }
    close(ctx->journal);
    _XmStringSourceDestroy(src);
}

/*
 *  Application callback is intended for applications that want to
 *  know if the source has ever changed.  
 */
PSsetApplicationCallback(src, callback)
  XmTextSourceRec *src;
  void (*callback)();
{
  PSContext   *ctx = (PSContext *)src->data;
    ctx->applicationCallback = callback;
}

/*
 * 	Force PS to break it's current piece.  
 */
PSbreakInput(src)
  XmTextSourceRec *src;
{
  PSContext   *ctx = (PSContext *)src->data;
	ctx->lastReplaceEndPos = -1;
}

Boolean PSourceGetEditable(source)
XmTextSource source;
{
    return source->data->editable;
}

void PSourceSetEditable(source, editable)
XmTextSource source;
Boolean editable;
{
    source->data->editable = editable;
}

static int redo(src, jnlFile)
  FILE *jnlFile;
  XmTextSource src;
{
/* test code.  rewrite this XXX */
    PSContext   *data= (PSContext *)src->data;
    int jptr, status = FALSE;
    int checkCnt;
    int backPtr;
    ChangeHeader ch;
    XmTextBlockRec text;
    XmTextPosition temp;

    jptr = ftell(jnlFile);
    if(fread(&ch, 1, sizeof(ch), jnlFile))
    {
        if(ch.begPos == JOURNAL_EOF)
	{
            data->jnlEOF = jptr;
	    return status;
        }
        text.ptr = XtMalloc(ch.delCnt+ch.insCnt);
	if (text.ptr == NULL) return (status);

	/*
	** Bogus loop allows various error conditions to exit with a break
	** statement.
	*/
	do
	{
	    if (!fread (text.ptr, ch.delCnt+ch.insCnt, 1, jnlFile)) break;
#ifdef DEBUG
printf("redo at %d - %d, %d, %d \n",jptr, ch.begPos, ch.insCnt, ch.delCnt);
#endif
            if (!fread(&backPtr, 4, 1, jnlFile)) break;

	    if (backPtr != (sizeof(ch) + ch.delCnt + ch.insCnt))
	    {
		printf("corrupted journal file??\n");
		break;
	    }

	    if(!(ch.delCnt|ch.insCnt)) break;

	    text.format = FMT8BIT;
	    text.length = ch.insCnt;
	    data->inhibitJournal = TRUE;
	    temp = ch.begPos + ch.delCnt;
	    (*src->Replace)
	    (
		data->widgets[0],
		NULL,
		&ch.begPos, &temp,
		&text,
		True
	    );
	    data->inhibitJournal = FALSE;
	    status = TRUE;
	}
	while (False);

	XtFree(text.ptr);
    }
    return status;
}

int PSRedo(src)
    XmTextSource src;
{
  int status;
  PSContext   *data= (PSContext *)src->data;
    if(!data->undoing)
	return FALSE;
    fseek(data->journal, data->undoIndex, 0);
    status = redo(src, data->journal);
    PSbreakInput(src);
    if(status)
	data->undoIndex = ftell(data->journal);
    return status;
}

int PSUndo(src)
XmTextSource src;
{
    PSContext   *data = (PSContext *)src->data;
    int  backptr, jptr, status, next;
    FILE *jnlFile;
    ChangeHeader ch;
    XmTextBlockRec text;
    XmTextPosition temp;

    if((data->lastReplaceEndPos > 0) && (!data->undoing))
    {
        wrtDelayJnl(data);
        data->lastReplaceEndPos = -1;
    }

    jnlFile = data->journal;
    if(data->undoing)
	jptr = data->undoIndex;
    else
    {
	fseek(jnlFile, data->jnlEOF, 0); 
	jptr = data->jnlEOF;
	data->undoing = TRUE;
    }
    jptr -= 4;
    if(jptr < 0)
    {
	return FALSE;
    }
    fseek(jnlFile, jptr, 0);
    status = fread(&backptr, 1, 4, jnlFile);  
    jptr -=  backptr;
    if(jptr < 0)
    {
	return FALSE;
    }
    next = jptr;
    fseek(jnlFile, jptr, 0);
    status = fread(&ch, 1, sizeof(ch),  jnlFile);
/*printf("undo at %d - %d, %d, %d \n",jptr, ch.begPos, ch.insCnt, ch.delCnt);*/
    if(ch.insCnt + ch.delCnt + sizeof(ch) != backptr)
    {
	printf("corrupted journal file???\n");
	return FALSE;
    }
    data->undoIndex = next;
    text.ptr = XtMalloc(ch.delCnt+ch.insCnt);
    text.format = FMT8BIT;
    text.length = ch.delCnt;
    status = fread(text.ptr, 1,  ch.insCnt + ch.delCnt, jnlFile);
    data->inhibitJournal = TRUE;

    temp = ch.begPos + ch.insCnt;
    (*src->Replace) (data->widgets[0], NULL, &ch.begPos, &temp, &text, True);

    data->inhibitJournal = FALSE;
    data->journal = jnlFile;
    XtFree(text.ptr);
    PSbreakInput(src);
}

int PSrecover(src, jnlFile)
  XmTextSource src;
  FILE *jnlFile;
{
    while(redo(src, jnlFile));
}

