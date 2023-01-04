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
static char rcsid[] = "$Header: /usr/sde/osf1/rcs/x11/src/extensions/ip/ip.c,v 1.1.4.3 1993/08/26 20:49:39 Dave_Hill Exp $";
#endif

/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: /usr/sde/osf1/rcs/x11/src/extensions/ip/ip.c,v 1.1.4.3 1993/08/26 20:49:39 Dave_Hill Exp $
 *  ip.c:
 *	Shared-Memory IPC package
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include "ip.h"

#ifdef IP_STATS
int ipnEnter;
int ipnLeave;
int ipnConflict;
int ipnMaxWait1;
int ipnMaxWait2;
int ipnBal = 0;
int ipnReq = 0;
int ipnFreed = 0;
static int ipnAlloc = 0;
static int ipnSent = 0;
#endif IP_STATS

#define VALIDATE_CHAN(chan, cmode)		\
    if ((chan) == NULL) {			\
	return IP_BADCHAN;			\
    } else if ((cmode)      != IP_EITHER &&	\
	       (chan)->mode != (cmode) &&	\
	       (chan)->mode != IP_BOTH) {	\
	return IP_BADMODE;			\
    } else if (!CHANNEL_OK(chan)) {		\
	return IP_SHUTDOWN;			\
    }

#define PRF(S)	S
#ifndef BUG
#define BUG(S)
#endif


int
ipNoOp()
{
    return IP_SUCCESS;
} /* end ipNoOp() */

#define PERMS 0666			/* for now */

int
ipCreateChannel(chan, type, mode, buf0size, buf1size, privsize)
ChannelPtr chan;
int type;
int mode;
Card buf0size;		/* buffer size in 32-bit words */
Card buf1size;		/* buffer size in 32-bit words */
Card privsize;		/* private area is 32-bit words */
{
    Card nbytes;
    int smid;
    IPDataPtr segaddr;
    MemHdrPtr base;

    if (type != IP_FULLDUP && type != IP_HALFDUP) return IP_BADTYPE;

    if (mode != IP_SENDER && mode != IP_RECEIVER &&
	mode != IP_BOTH) return IP_BADMODE;

    nbytes = sizeof (MemHdr) + ((buf0size +buf1size +privsize) << 2);

    if ( (smid = shmget( IPC_PRIVATE, nbytes,
			IPC_CREAT | IPC_EXCL | PERMS)) == -1 ) {
	return IP_SHMERR;
    }
    if ( (segaddr = (IPDataPtr) shmat(smid, 0, 0)) == (IPDataPtr) -1 ) {
	return IP_SHMERR;
    }

    /* Warning: subsequent code assumes segment initially zeroed. */
    bzero(segaddr, nbytes);

    chan->whoami = IP_CREATOR;
    chan->size = nbytes >> 2;
    chan->type = type;
    chan->mode = mode;
    chan->smid = smid;
    chan->base = (MemHdrPtr) segaddr;
    chan->allocFail = ipNoOp;
    chan->receiveFail = ipNoOp;
    chan->base->cStatus = IP_ALIVE;

    base = chan->base;

    base->bd0.size = buf0size;
    base->bd0.offset = sizeof(MemHdr) >> 2;
    base->bd0.lastWritten = 0;
    base->bd0.lastRead = 0;
    base->bd0.lastReserved = 0;
    base->bd0.highWater = 0;
#if 0
    base->bd0.turn = IP_CREATOR;
    base->bd0.usingBuf[IP_CREATOR] = FALSE;
    base->bd0.usingBuf[IP_ATTACHER] = FALSE;
#else
    base->bd0.nextRead = 0;
    base->bd0.version = IP_VERSION;
#endif
    chan->pbuf0 = segaddr + base->bd0.offset;

    if (chan->mode == IP_SENDER) {
	chan->sendBd = &base->bd0;
	chan->sendBuf = chan->pbuf0;
	chan->receiveBd = NULL;
	chan->receiveBuf = NULL;
    } else if (chan->mode == IP_RECEIVER) { 
	chan->receiveBd = &base->bd0;
	chan->receiveBuf = chan->pbuf0;
	chan->sendBd = NULL;
	chan->sendBuf = NULL;
    } else {					/* mode == IP_BOTH */
	/* init buffer descriptor 1 */
	base->bd1.size = buf1size;
	base->bd1.offset = base->bd0.offset + base->bd0.size;
	base->bd1.lastWritten = 0;
	base->bd1.lastRead = 0;
	base->bd1.lastReserved = 0;
	base->bd1.highWater = 0;
#if 0
	base->bd1.turn = IP_ATTACHER;
	base->bd1.usingBuf[IP_CREATOR] = FALSE;
	base->bd1.usingBuf[IP_ATTACHER] = FALSE;
#else
	base->bd1.nextRead = 0;
	base->bd1.version = IP_VERSION;
#endif
	chan->pbuf1 = segaddr + base->bd1.offset;

	chan->sendBd = &base->bd0;
	chan->sendBuf = chan->pbuf0;
	chan->receiveBd = &base->bd1;
	chan->receiveBuf = chan->pbuf1;

    }

    if (privsize > 0) {
	base->privd.size = privsize;
	base->privd.offset = base->bd0.offset + base->bd0.size +
		base->bd1.size;
	chan->private = segaddr + base->privd.offset;
    } else
	chan->private = NULL;

    ipInitHash();

#ifdef IP_STATS
    ipnAlloc = 0;
    ipnSent = 0;
    ipnFreed = 0;
    ipnEnter = 0;
    ipnLeave = 0;
    ipnConflict = 0;
    ipnMaxWait1 = 0;
    ipnMaxWait2 = 0;
    fprintf(stderr, "IPChannel created.\n");
#endif IP_STATS

    return IP_SUCCESS;

} /* end ipCreateChannel() */



/******************************
 * ipAttachChannel():
 *	It is assumed that
 *	   --- the attaching process is different from the creating process
 *	   --- the channel structure is a copy of the creating process'
 ******************************/
int
ipAttachChannel(chan)
ChannelPtr chan;
{
    int smid;
    IPDataPtr segaddr;
    struct shmid_ds sds;

    if (chan == NULL) return IP_BADCHAN;	/* invalid channel */

    if ( (segaddr = (IPDataPtr) shmat(chan->smid, 0, 0)) == (IPDataPtr) -1 ) {
	return IP_SHMERR;
    }

    shmctl(chan->smid, IPC_STAT, &sds);
    sds.shm_perm.mode = 0;			/* for security */
    shmctl(chan->smid, IPC_SET, &sds);
    /* mark close on client exit */
    shmctl(chan->smid, IPC_RMID, NULL);

    chan->whoami = IP_ATTACHER;
    switch(chan->mode) {		/* struct initialized by creator */
    case IP_SENDER:
	chan->mode = IP_RECEIVER;
	break;
    case IP_RECEIVER:
	chan->mode = IP_SENDER;
	break;
    } /* end switch() */

    chan->base = (MemHdrPtr) segaddr;
    chan->base->aStatus = IP_ALIVE;

    chan->pbuf0 = segaddr + chan->base->bd0.offset;

    if (chan->mode == IP_SENDER) {
	chan->sendBd = &chan->base->bd0;
	chan->sendBuf = chan->pbuf0;
	chan->receiveBd = NULL;
	chan->receiveBuf = NULL;
    } else if (chan->mode == IP_RECEIVER) { 
	chan->receiveBd = &chan->base->bd0;
	chan->receiveBuf = chan->pbuf0;
	chan->sendBd = NULL;
	chan->sendBuf = NULL;
    } else {					/* mode == IP_BOTH */
	chan->pbuf1 = segaddr + chan->base->bd1.offset;
	chan->sendBd = &chan->base->bd1;
	chan->sendBuf = chan->pbuf1;
	chan->receiveBd = &chan->base->bd0;
	chan->receiveBuf = chan->pbuf0;
    }

    if (chan->private != NULL)
	chan->private = segaddr + chan->base->privd.offset;

#ifdef IP_STATS
    ipnAlloc = 0;
    ipnSent = 0;
    ipnFreed = 0;
    ipnEnter = 0;
    ipnLeave = 0;
    ipnConflict = 0;
    ipnMaxWait1 = 0;
    ipnMaxWait2 = 0;
#endif IP_STATS

    if (CHANNEL_OK(chan))
	return IP_SUCCESS;
    else
	return IP_SHUTDOWN;

} /* end ipAttachChannel() */


/******************************
 * ipAllocateData():
 *	Reserve space in the Ring Buffer for later transmission
 *	to the receiver by a send call.  This code assumes that
 *	an allocate calls alternate with send calls (i.e. two
 *	successive allocates is illegal).
 ******************************/
int
ipAllocateData(chan, nwords, pdata)
ChannelPtr chan;
Card nwords;
IPDataPtr *pdata;
{
    BufDescPtr bd;
    int lastRead;
    int status;

#ifdef IP_STATS
    if (ipnAlloc != ipnSent) {
	fprintf(stderr, "ipAllocateData(): ipnAlloc=%d ipnSent=%d\n",
		ipnAlloc, ipnSent);
    }
#endif IP_STATS

retry:					/* ??? */
    VALIDATE_CHAN(chan, IP_SENDER);
    bd = chan->sendBd;
    if (nwords > bd->size) {
	*pdata = NULL;			/* ??? for now */
#ifdef IP_STATS
	fprintf(stderr, "ipAllocateData(): BADSIZE 1\n");
#endif IP_STATS
	return IP_BADSIZE;
    }

    /*
     * We need to get a static copy of the lastRead index,
     * as it could change at any time (including wrapping).
     * Nothing bad will happen, however, if lastRead changes
     * after its value is fixed: the max allocation possible
     * will simply be more conservative.
     */
    lastRead = bd->lastRead;
    if (lastRead <= bd->lastWritten) {
	if (nwords <= bd->size - bd->lastWritten) {
	    *pdata = chan->sendBuf + bd->lastWritten;
	    bd->lastReserved += nwords;
	    status = IP_SUCCESS;
	} else if (nwords < lastRead) {		/* wraparound to front */
	    *pdata = chan->sendBuf;		/* but leave a spacer word */
	    bd->highWater = bd->lastWritten;
	    bd->lastWritten = 0;
	    bd->lastReserved = nwords;
#ifdef IP_STATS
	    if (bd->lastReserved >= lastRead) abort();
#endif
	    status = IP_SUCCESS;
	} else {				/* no space available */
	    status = (*chan->allocFail)(chan, nwords, chan->allocFailParam);
	    if (status == IP_RETRY) goto retry;
	}
    } else {
	if (nwords < lastRead - bd->lastWritten) {
	    *pdata = chan->sendBuf + bd->lastWritten;
	    bd->lastReserved += nwords;
#ifdef IP_STATS
	    if(bd->lastReserved >= lastRead) abort();
#endif
	    status = IP_SUCCESS;
	} else {
	    status = (*chan->allocFail)(chan, nwords, chan->allocFailParam);
	    if (status == IP_RETRY) goto retry;
	}
    }

    /* return NULL if callbacks failed */
    if (status != IP_SUCCESS) {
#ifdef IP_STATS
	fprintf(stderr, "ipAllocateData(): BADSIZE 2\n");
#endif IP_STATS
	*pdata = NULL;
    }
#ifdef IP_STATS
    if (status == IP_SUCCESS) ipnAlloc++;
#endif IP_STATS

    return status;
} /* end ipAllocateData() */


/******************************
 * ipUnallocateData():
 *	unreserve some previously reserved data.
 ******************************/
int
ipUnallocateData(chan, nwords)
ChannelPtr chan;
Card nwords;
{
    BufDescPtr bd;

    VALIDATE_CHAN(chan, IP_SENDER);
    bd = chan->sendBd;

    if (nwords >= 0 && bd->lastReserved - bd->lastWritten >= nwords) {
	bd->lastReserved -= nwords;
	return IP_SUCCESS;
    } else {
#ifdef IP_STATS
	fprintf(stderr, "ipUnallocateData(): BADSIZE\n");
#endif IP_STATS
	return IP_BADSIZE;
    }
} /* end ipUnallocateData() */


/******************************
 * ipUnallocateAndSendData():
 *	unreserve some previously reserved data.
 ******************************/
int
ipUnallocateAndSendData(chan, nwords)
ChannelPtr chan;
Card nwords;
{
    BufDescPtr bd;

    VALIDATE_CHAN(chan, IP_SENDER);
    bd = chan->sendBd;

#ifdef IP_STATS
    if (ipnAlloc != ipnSent+1) {
	fprintf(stderr, "ipUnallocateAndSendData(): ipnAlloc=%d  ipnSent=%d\n",
		ipnAlloc, ipnSent);
    }
#endif IP_STATS

    if (nwords >= 0 && bd->lastReserved - bd->lastWritten >= nwords) {
	bd->lastReserved -= nwords;
	bd->lastWritten = bd->lastReserved;
#ifdef IP_STATS
	ipnSent++;
#endif IP_STATS
	return IP_SUCCESS;
    } else {
#ifdef IP_STATS
	fprintf(stderr, "ipUnallocateAndSendData(): BADSIZE\n");
#endif IP_STATS
	return IP_BADSIZE;
    }
} /* end ipUnallocateAndSendData() */


/******************************
 * ipSpaceAvailable():
 *	Return a lower bound on the number of words allocatable
 *	by sender for this channel.
 ******************************/
int
ipSpaceAvailable(chan)
ChannelPtr chan;
{
    BufDescPtr bd;
    IPDataPtr pbuf;
    int lastRead;

    VALIDATE_CHAN(chan, IP_SENDER);
    bd = chan->sendBd;
    lastRead = bd->lastRead;

    if (lastRead <= bd->lastWritten) {
	return _Max( (bd->size - bd->lastWritten), (lastRead - 1) );
    } else
	return lastRead - bd->lastWritten - 1;

} /* end ipSpaceAvailable() */


/******************************
 * ipSendData():
 ******************************/
int
ipSendData(chan)
ChannelPtr chan;
{
    BufDescPtr bd;

#ifdef IP_STATS
    if (ipnAlloc != ipnSent+1) {
	fprintf(stderr, "ipSendData(): ipnAlloc=%d  ipnSent=%d\n",
		ipnAlloc, ipnSent);
    }
#endif IP_STATS
    VALIDATE_CHAN(chan, IP_SENDER);
    bd = chan->sendBd;

    bd->lastWritten = bd->lastReserved;
#ifdef IP_STATS
    ipnSent++;
#endif IP_STATS
    return IP_SUCCESS;
} /* end ipSendData() */


/******************************
 * ipSendAndAllocateData():
 ******************************/
int
ipSendAndAllocateData(chan, nwords, pdata)
ChannelPtr chan;
Card nwords;
IPDataPtr *pdata;
{
    int status = ipSendData(chan);
    if (status == IP_SUCCESS)
	return ipAllocateData(chan, nwords, pdata);
    else
	return status;
} /* end ipSendAndAllocateData() */


/******************************
 * ipReceiveData():
 *	Get a pointer to the first unread word of data.  Note
 *	that this call does *not* advance the nextRead index.
 ******************************/
int
ipReceiveData(chan, pnwords, pdata)
ChannelPtr chan;
Card *pnwords;
IPDataPtr *pdata;
#ifndef notdef
{
    int avail;
    return ipReceiveDataAvail(chan, pnwords, pdata, &avail);
}

int
ipReceiveDataAvail(chan, pnwords, pdata, pavail)
ChannelPtr chan;
Card *pnwords;
IPDataPtr *pdata;
Card *pavail;
#endif
{
    register BufDescPtr bd;
    register int lastWritten;

    VALIDATE_CHAN(chan, IP_RECEIVER);
    bd = chan->receiveBd;
    lastWritten = bd->lastWritten;

    if (bd->nextRead <= lastWritten)
	*pnwords = lastWritten - bd->nextRead;
    else {
	/*
	 * It may be assumed that the highwater can't change
	 * because the write pointer has already wrapped
	 */
	if (bd->nextRead == bd->highWater) {
	    bd->nextRead = 0;		/* wrap read pointer */
	    *pnwords = lastWritten;
	} else {
	    *pnwords = bd->highWater - bd->nextRead;
	}
    }

    *pdata = chan->receiveBuf + bd->nextRead;

#ifndef notdef
    if (bd->lastRead <= lastWritten) {
	*pavail = _Max( (bd->size - lastWritten), (bd->lastRead - 1) );
    } else
	*pavail = bd->lastRead - lastWritten - 1;
#endif
    BUG(
	fprintf(stderr,
		"ipRc r%d t%d w%d h%d n%d\n", bd->lastRead, bd->nextRead,
		lastWritten, bd->highWater, *pnwords);
	);

    return IP_SUCCESS;
} /* end ipReceiveData() */


/******************************
 * ipReceiveAtLeastData():
 ******************************/
int
ipReceiveAtLeastData(chan, nwords, pactual_words, pdata)
ChannelPtr chan;
Card nwords;
Card *pactual_words;
IPDataPtr  *pdata;
{
    BufDescPtr bd;
    IPDataPtr pbuf;
    int lastWritten;
    int status;

retry:
    VALIDATE_CHAN(chan, IP_RECEIVER);
    bd = chan->receiveBd;
    lastWritten = bd->lastWritten;

    pbuf = chan->receiveBuf + bd->nextRead;

    *pactual_words = (bd->nextRead <= lastWritten) ?
			lastWritten - bd->nextRead :
			bd->highWater - bd->nextRead ;

    if (nwords <= *pactual_words)
	status = IP_SUCCESS;
    else {
	status = (*chan->receiveFail)(chan, nwords, chan->receiveFailParam);
	if (status == IP_RETRY) goto retry;
    }
    *pdata = (status == IP_SUCCESS) ? pbuf : NULL; /* ??? for now */
    return status;
} /* end ipReceiveAtLeastData() */


/******************************
 * ipAdvanceRdPtr():
 *	Like ipFreeData(), but does not make it visible to the sender.
 ******************************/
int
ipAdvanceRdPtr(chan, nwords)
ChannelPtr chan;
Card nwords;
{
    register BufDescPtr bd;
    register int lastWritten;
    int wordsAvail;

    VALIDATE_CHAN(chan, IP_RECEIVER);
    bd = chan->receiveBd;
    lastWritten = bd->lastWritten;

#ifdef IP_STATS
    ipnBal += nwords;
    ipnReq += 1;
#endif IP_STATS

    if (nwords == 0) {
#ifdef IP_STATS
	fprintf(stderr, "ipAdvancePtr(): BADSIZE 1\n");
	abort();
#endif IP_STATS
	return IP_SUCCESS;
    }

    if (bd->nextRead <= lastWritten) {
	wordsAvail = lastWritten - bd->nextRead;
	if (nwords > wordsAvail) {
#ifdef IP_STATS
	    fprintf(stderr, "ipAdvancePtr(): BADSIZE 2\n");
	    abort();
#endif IP_STATS
	    return IP_BADSIZE;
	}
	bd->nextRead  = bd->nextRead + nwords;
    } else {
	wordsAvail = bd->highWater - bd->nextRead;
	if (nwords > wordsAvail) {
#ifdef IP_STATS
	    fprintf(stderr, "ipAdvancePtr(): BADSIZE 3\n");
	    abort();
#endif IP_STATS
	    return IP_BADSIZE;
	}
	bd->nextRead  = bd->nextRead + nwords;
	if (bd->nextRead == bd->highWater)
	    bd->nextRead = 0;		/* wrap read pointer */
    }

    BUG(
	fprintf(stderr,
		"ipAd r%d t%d w%d h%d n%d\n", bd->lastRead, bd->nextRead,
		lastWritten, bd->highWater, nwords);
	);

    return IP_SUCCESS;
} /* end ipAdvanceRdPtr() */


/******************************
 * ipFreeData():
 *	Advance the lastRead index, freeing buffer space for the
 *	sender to write new data into.  Assumes that nextRead
 *	stays between lastRead and lastWritten.
 ******************************/

int
ipFreeData(chan, nwords)
ChannelPtr chan;
Card nwords;
{
    BufDescPtr bd;
    int wordsAvail;
    int lastWritten;

    VALIDATE_CHAN(chan, IP_RECEIVER);
    bd = chan->receiveBd;
    lastWritten = bd->lastWritten;

#ifdef IP_STATS
    ipnFreed++;
    if ((ipnBal -= nwords) < 0) abort();
    if ((ipnReq -= 1) < 0) abort();
#endif IP_STATS

    if (nwords == 0) {
#ifdef IP_STATS
	fprintf(stderr, "ipFreeData(): BADSIZE 1\n");
	abort();
#endif IP_STATS
	return IP_SUCCESS;
    }

    if (bd->lastRead <= lastWritten) {
	wordsAvail = lastWritten - bd->lastRead;
	if (nwords > wordsAvail) {
#ifdef IP_STATS
	    fprintf(stderr, "ipFreeData(): BADSIZE 2\n");
	    abort();
#endif IP_STATS
	    return IP_BADSIZE;
	}
	bd->lastRead = bd->lastRead + nwords;
	bd->nextRead = bd->lastRead;	/* maintain invariance */
    } else {
	if (bd->lastRead == bd->highWater) {
	    /*
	     * How did we get here?  The previous free left
	     * lastRead == lastWritten; then the client wraps lastWritten
	     * and sets highWater.  highWater can't change while we have
	     * lastRead pegged to it, so it's safe now to wrap lastRead.
	     */
	    bd->lastRead = 0;
	    bd->nextRead = bd->lastRead;	/* maintain invariance */
	    wordsAvail = lastWritten;
	} else {
	    wordsAvail = bd->highWater - bd->lastRead;
	}
	if (nwords > wordsAvail) {
#ifdef IP_STATS
	    fprintf(stderr, "ipFreeData(): BADSIZE 3\n");
	    abort();
#endif IP_STATS
	    return IP_BADSIZE;
	}
	bd->lastRead += nwords;
	if (bd->lastRead == bd->highWater)
	    bd->lastRead = 0;		 /* wrap free pointer */
	bd->nextRead = bd->lastRead;	/* maintain invariance */
    } /* end if */

    BUG(
	fprintf(stderr,
		"ipFr r%d t%d w%d h%d n%d\n", bd->lastRead, bd->nextRead,
	       lastWritten, bd->highWater, nwords);
	);

    return IP_SUCCESS;
} /* end ipFreeData() */

/**************************************************************
 *
 * Like ipFreeData() except this wraps free pointer internally.
 *
 **************************************************************/
int
ipBulkFreeData(chan, nwords)
ChannelPtr chan;
Card nwords;
{
    BufDescPtr bd;
    int wordsAvail;
    int lastWritten;

    VALIDATE_CHAN(chan, IP_RECEIVER);
    bd = chan->receiveBd;
    lastWritten = bd->lastWritten;

#ifdef IP_STATS
    ipnFreed++;
    if ((ipnBal -= nwords) < 0) abort();
    if ((ipnReq -= 1) < 0) abort();
#endif IP_STATS

    if (nwords == 0) {
#ifdef IP_STATS
	fprintf(stderr, "ipFreeData(): BADSIZE 1\n");
	abort();
#endif IP_STATS
	return IP_SUCCESS;
    }

    if (bd->lastRead <= lastWritten) {
	wordsAvail = lastWritten - bd->lastRead;
	if (nwords > wordsAvail) {
#ifdef IP_STATS
	    fprintf(stderr, "ipFreeData(): BADSIZE 2\n");
	    abort();
#endif IP_STATS
	    return IP_BADSIZE;
	}
	bd->lastRead += nwords;
    } else {
	if (bd->lastRead == bd->highWater) {
	    /*
	     * How did we get here?  The previous free left
	     * lastRead == lastWritten; then the client wraps lastWritten
	     * and sets highWater.  highWater can't change while we have
	     * lastRead pegged to it, so it's safe now to wrap lastRead.
	     */
	    if (nwords > lastWritten) {
#ifdef IP_STATS
		fprintf(stderr, "ipFreeData(): BADSIZE 3\n");
		abort();
#endif IP_STATS
		return IP_BADSIZE;
	    }
	    bd->lastRead = nwords;
	}
	else {
	    wordsAvail = bd->highWater - bd->lastRead;
	    if (nwords > wordsAvail) {
		nwords -= wordsAvail;
		if (nwords > lastWritten) {
#ifdef IP_STATS
		    fprintf(stderr, "ipFreeData(): BADSIZE 4\n");
		    abort();
#endif IP_STATS
		    return IP_BADSIZE;
		}
		bd->lastRead = nwords;
	    }
	    else
		bd->lastRead += nwords;
	}
	/* does this need to be here, or 1 line up??? */
	if (bd->lastRead == bd->highWater)
	    bd->lastRead = 0;		 /* wrap free pointer */
    }

    BUG(
	fprintf(stderr,
		"ipFr r%d t%d w%d h%d n%d\n", bd->lastRead, bd->nextRead,
	       lastWritten, bd->highWater, nwords);
	);

    return IP_SUCCESS;
}


/******************************
 * ipSetAllocCallback():
 ******************************/
int
ipSetAllocCallback(chan, allocFailProc, allocFailParam)
ChannelPtr chan;
int (*allocFailProc)();
long allocFailParam;
{
    if (chan == NULL) return IP_BADCHAN;

    chan->allocFail = allocFailProc;
    chan->allocFailParam = allocFailParam;
    return IP_SUCCESS;
} /* end ipSetAllocCallback() */

 /******************************
 * ipSetReceiveCallback():
 ******************************/
int
ipSetReceiveCallback(chan, receiveFailProc, receiveFailParam)
ChannelPtr chan;
int (*receiveFailProc)();
long receiveFailParam;
{
    if (chan == NULL) return IP_BADCHAN;

    chan->receiveFail = receiveFailProc;
    chan->receiveFailParam = receiveFailParam;
    return IP_SUCCESS;
} /* end ipSetReceiveCallback() */

/******************************
 * ipDestroyChannel():
 *	It is assumed that
 *
 ******************************/
int
ipDestroyChannel(chan)
ChannelPtr chan;
{
    if (chan == NULL) return IP_BADCHAN;	/* invalid channel */
    switch (chan->whoami) {
    case IP_CREATOR:
	if (!chan->base->cStatus) return IP_SHUTDOWN;
	shmctl(chan->smid, IPC_RMID, NULL);
	chan->base->cStatus = FALSE;
	break;
    case IP_ATTACHER:
	if (!chan->base->aStatus) return IP_SHUTDOWN;
	chan->base->aStatus = FALSE;
	break;
    } /* end switch() */
    
    /* 
     * shmctl(chan->smid, IPC_RMID, 0);
     */
    if (shmdt(chan->base) == 0) {
	chan->base = 0;
	return IP_SUCCESS;
    } else {
	chan->base = 0;
	return IP_SHMERR;
    }
} /* end ipDestroyChannel() */


/******************************
 * ipSendBufferSize():
 *
 ******************************/
int
ipSendBufferSize(chan)
ChannelPtr chan;
{
    VALIDATE_CHAN(chan, IP_SENDER);
    return chan->sendBd->size;
} /* end ipSendBufferSize() */


/******************************
 * ipReceiveBufferSize():
 *
 ******************************/
int
ipReceiveBufferSize(chan)
ChannelPtr chan;
{
    VALIDATE_CHAN(chan, IP_RECEIVER);
    return chan->receiveBd->size;
} /* end ipReceiveBufferSize() */


/******************************
 * ipGetPrivateBuffer():
 *
 ******************************/
int
ipPrivateBuffer(chan, pnwords, pdata)
ChannelPtr chan;
int *pnwords;
IPDataPtr *pdata;
{
    VALIDATE_CHAN(chan, IP_EITHER);
    if (chan->private) {
	*pnwords = chan->base->privd.size;
	*pdata = chan->private;
	return IP_SUCCESS;
    } else {
	*pnwords = 0;
	*pdata = NULL;	/* ??? for now */
	return IP_NOPRIVATE;
    }
} /* end ipPrivateBuffer() */
