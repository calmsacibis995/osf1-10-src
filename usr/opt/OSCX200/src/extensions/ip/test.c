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
/******************************
 * test.c:
 *	program to test shared-memory IPC routines.
 * RCS:
 * 
 ******************************/


#include <stdio.h>
#include <malloc.h>
#include <strings.h>
#include <math.h>
#include "ip.h"

CARD random();

#define PRINTF if (verbose) printf

#define REPLY_HEADER 0xfacade

/* return a uniformly distributed integer between min <= i <= max */
#define URAND(min, max) ((min) + random() % ((max) - (min) + 1))

static IPDataPtr pcurBuf;
static CARD curBufSize;
static CARD npkt = 0;
static CARD verbose;
static int sleepTime = 0;

/******************************
 * eqlblk():
 *	Test to see if a block of data is equal to the
 *	same value.
 ******************************/
static int
eqlblk(pdata, nwords)
IPDataPtr pdata;
CARD nwords;
{
    register int val = *pdata;
    register int i;

    for (i=0; i<nwords; i++) if (*pdata++ != val) return FALSE;
    return TRUE;
} /* end eqlblk() */

/******************************
 * PrintData():
 *
 ******************************/
#define WORDSPERROW 8

static void
PrintData(pdata, nwords)
IPDataPtr pdata;
CARD nwords;
{
    int c, r;
    int nrows = nwords / WORDSPERROW;
    int remcols = nwords % WORDSPERROW;

    if (!verbose) return;
#define FMT1 "%8d "
    for (r=0; r<nrows; r++) {
	printf("     ");
	for (c=0; c<WORDSPERROW; c++) printf(FMT1, *pdata++);
	printf("\n");
    }
    if (remcols) {
	printf("     ");
	for (c=0; c<remcols; c++) printf(FMT1, *pdata++);
	printf("\n");
    }
#undef FMT1
} /* end PrintData() */


/******************************
 * afail():
 ******************************/
static int
afail(chan, nwords, param)
ChannelPtr chan;
CARD nwords;
int param;
{
    printf("...called afail, chan=%8.8x, nwords=%d, param=%d\n",
	chan, nwords, param);
    return IP_SUCCESS;
} /* end afail() */


/******************************
 * afail_poll():
 ******************************/
static int
afail_poll(chan, nwords, param)
ChannelPtr chan;
CARD nwords;
int param;
{
    int sa;

    while ((sa = ipSpaceAvailable(chan)) < nwords) {
	if (sleepTime >= 0) sleep(sleepTime);
    }
    return IP_RETRY;
} /* end afail_poll() */


/******************************
 * rfail():
 ******************************/
static int
rfail(chan, nwords, param)
ChannelPtr chan;
CARD nwords;
int param;
{
    printf("...called rfail, chan=%8.8x, nwords=%d, param=%d\n",
	chan, nwords, param);
    return IP_SUCCESS;
} /* end rfail() */


/******************************
 * rfail_poll():
 ******************************/
static int
rfail_poll(chan, nwords, param)
ChannelPtr chan;
CARD nwords;
int param;
{
    int actual_words;
    int status;

    while (TRUE) {
	IPDataPtr pdata;
	status = ipReceiveData(chan, &actual_words, &pdata);
	if (status == IP_SUCCESS && actual_words >= nwords) return IP_RETRY;
	if (sleepTime >= 0) sleep(sleepTime);
    } /* end while */
} /* end rfail_poll() */


/******************************
 * send():
 ******************************/
static void
send(chan)
ChannelPtr chan;
{
    int status;
    printf("---> send current data\n", curBufSize);
    status = ipSendData(chan);
    if (status) {
	printf("     succeeded\n");
    } else {
	printf("     failed, status=%d\n", status);
    }
} /* end send() */

/******************************
 * receive():
 ******************************/
static void
receive(chan)
ChannelPtr chan;
{
    int nwords;
    IPDataPtr pdata;
    int status;

    printf("---> receive new data\n");
    status = ipReceiveData(chan, &nwords, &pdata);
    if (status == IP_SUCCESS) {
	printf("     succeeded, read %d words at %8.8x\n",
		nwords, pdata);
	PrintData(pdata, nwords);
    } else {
	printf("     failed, status=%d\n", status);
    }
} /* end receive() */


/******************************
 * receiveATL()
 ******************************/
static int
receiveATL(chan, nwords, pactual_words, pdata)
ChannelPtr chan;
int nwords;
int *pactual_words;
IPDataPtr *pdata;
{
    int status;

    printf("---> receive at least %d words\n", nwords);
    status = ipReceiveAtLeastData(chan, nwords, pactual_words, pdata);
    if (status == IP_SUCCESS) {
	printf("     succeeded, read %d words at %8.8x\n",
		*pactual_words, pdata);
	PrintData(*pdata, *pactual_words);
    } else {
	printf("     failed.\n");
    }
    return status;
} /* end receiveATL() */


/******************************
 * allocate()
 ******************************/
static void
allocate(chan, n)
ChannelPtr chan;
CARD n;
{
    int status;
    int i;

    curBufSize = n;
    printf("---> allocate %d, init to %d.\n", curBufSize, ++npkt);
    status = ipAllocateData(chan, curBufSize, &pcurBuf);
    if (status == IP_SUCCESS) {
	printf("     succeeded, pcurBuf=%8.8x\n", pcurBuf);
	for (i=0; i<curBufSize; i++) pcurBuf[i] = npkt;
    } else {
	printf("     failed, status=%d\n", status);
    }
} /* end allocate() */


/******************************
 * main():
 *
 ******************************/
main(argc,argv)
int argc;
char **argv;
{
    char line[80];
    CARD type, mode, buf0size, buf1size;
    ChannelPtr chan = (ChannelPtr) malloc (sizeof(Channel));

    verbose = (argc == 2);

    while( (printf("> "), gets(line)) != NULL) {
	switch (line[0]) {
	case 'C':			/* Create */
	 {
	    int status;

	    sscanf( &line[2], "%d %d %d %d\n", &type, &mode, &buf0size,
			&buf1size);
	    status = ipCreateChannel(chan, type, mode, buf0size, buf1size, 0);
	    printf("---> create type=%d mode=%d buf0size=%d buf1size=%d\n",
		type, mode, buf0size, buf1size);
	    if (status == IP_SUCCESS) {
		FILE *fp;
		fp = fopen("mongo.tmp", "w");
		fwrite(chan, sizeof(Channel), 1, fp);
		fclose(fp);
		printf("     succeeded, smid=%d, channel struct file=%s\n",
			chan->smid, "mongo.tmp");
	    } else {
		printf("     failed, status=%d\n", status);
	    }
	    ipSetAllocCallback(chan, afail, 1);
	    ipSetReceiveCallback(chan, rfail, 2);
	    break;
	  }
	case 'A':			/* Attach */
	  {
	    char *fname;
	    char *pidx;
	    int status;
	    FILE *fp;

	    fname = &line[2];
	    if (pidx = index(fname, ' ')) *pidx = '\0';
	    printf("---> attach %s\n", fname);
	    if ((fp = fopen(fname, "r")) == NULL) {
		printf("     couldn't open file %s\n", fname);
	    } else {
		chan = (ChannelPtr) malloc(sizeof(Channel));
		fread(chan, sizeof(Channel), 1, fp);
		fclose(fp);
	    }
	    if ((status = ipAttachChannel(chan)) == IP_SUCCESS) {
		printf("     succeeded, status=%d\n", status);
		ipSetAllocCallback(chan, afail, 3);
		ipSetReceiveCallback(chan, rfail, 4);
	    } else {
		printf("     failed, status=%d\n", status);
	    }
	    break;
	  } /* end case */
	case 'D':			/* destroy */
	  {
	    int status;

	    printf("---> destroy\n");
	    if ((status = ipDestroyChannel(chan)) == IP_SUCCESS) {
		printf("     succeeded, status=%d\n", status);
	    } else {
		printf("     failed, status=%d\n", status);
	    }
	    break;
	  } /* end case */
	case 'a':			/* allocate */
	  {
	    int n;

	    sscanf( &line[2], "%d\n", &n);
	    allocate(chan, n);
	    break;
	  } /* end case */
	case 'i':			/* initialize */
	  {
	    int i;
	    int val;

	    sscanf( &line[2], "%d\n", &val);
	    printf("---> init buf to %d.\n", val);
	    for (i=0; i<curBufSize; i++) pcurBuf[i] = val;
	    break;
	  } /* end case */
	case 'q':
	case 'Q':
	    exit(0);
	    break;
	case 's':			/* send */
	  {
	    send(chan);
	    break;
	  } /* end case */
	case 'r':			/* receive */
	  {
	    receive(chan);
	    break;
	  } /* end case */
	case 'R':			/* receive at least */
	  {
	    int nwords;
	    int actual_words;
	    IPDataPtr pdata;
	    int status;

	    sscanf(&line[2], "%d", &nwords);
	    status = receiveATL(chan, nwords, &actual_words, &pdata);
	  } /* end case */
	case 'f':			/* free */
	  {
	    int nwords;
	    int status;

	    sscanf(&line[2], "%d", &nwords);
	    printf("---> free %d words\n", nwords);
	    if ( (status = ipFreeData(chan, nwords)) == IP_SUCCESS ) {
		printf("     succeeded, freed %d words\n", nwords);
	    } else {
		printf("     failed, status=%d\n", status);
	    }
	    break;
	  } /* end case 'f' */
	case 't':			/* test */
	  {
	    char testtype = line[2];

	    switch(testtype) {
	    case 's':
	      {
		int bufsize = ipSendBufferSize(chan);
		if (bufsize < 0) {
		    printf("send test failed, bufsize=%d\n", bufsize);
		    break;
		}

		printf("---> send test, buffer size=%d\n", bufsize);
		ipSetAllocCallback(chan, afail_poll, 11);
		srandom(1);		/* init random # generator */
		while (TRUE) {
		    int n = URAND(1, bufsize / 2);

		    allocate(chan, n);
		    *pcurBuf = n;
		    send(chan);
		}
		break;
	      }
	    case 'r':
	      {
		int nwords;
		int actual_words;
		int status;
		IPDataPtr pdata;
		int save_verbose;

		printf("---> receive test\n");
		ipSetReceiveCallback(chan, rfail_poll, 12);

		while (TRUE) {
		   int nthisrec;

		    status = receiveATL(chan, 1, &actual_words, &pdata);
		    while (pdata && actual_words) {
			nthisrec = *pdata;
			if (!eqlblk(pdata+1, nthisrec-1)) {
			    printf("block not equal at %8.8x\n", pdata);
			    save_verbose = verbose;
			    verbose = 1;
			    PrintData(pdata, nthisrec);
			    verbose = save_verbose;
			}
			pdata += nthisrec;
			actual_words -= nthisrec;
			ipFreeData(chan, nthisrec);
		    } /* end while */
		} /* end while */
		break;
	      }
	    case 'S':
	      {
		int bufsize = ipSendBufferSize(chan);
		int actual_words;
		int status;
		IPDataPtr pdata;

		if (bufsize < 0) {
		    printf("send test failed, bufsize=%d\n", bufsize);
		    break;
		}

		printf("---> SEND/receive test, buffer size=%d\n", bufsize);
		ipSetAllocCallback(chan, afail_poll, 21);
		ipSetReceiveCallback(chan, rfail_poll, 22);
		srandom(1);		/* init random # generator */
		while (TRUE) {
		    int n = URAND(1, bufsize / 2);
		    int want_reply = URAND(1, 100) > 90 ? TRUE : FALSE;

		    allocate(chan, n);
		    *pcurBuf = (want_reply) ? -n : n;
		    send(chan);

		    if (want_reply) {
			printf("SEND/receive: expecting reply\n");
			status = receiveATL(chan, 10, &actual_words, &pdata);
			if (status != IP_SUCCESS) {
			    printf("SEND/receive: reply failed, status=%d\n",
				status);
			    break;
			}
			if (*pdata != REPLY_HEADER) {
			    printf("SEND/receive: token %8.8x wrong\n",
				*pdata);
			    break;
			} else {
			    printf("SEND/receive: reply OK\n");
			    ipFreeData(chan, 10);
			}
		    } /* end if */
		} /* end while */
		break;
	      }
	    case 'R':
	      {
		int nwords;
		int actual_words;
		int status;
		IPDataPtr pdata;
		int save_verbose;

		printf("---> send/RECEIVE test\n");
		ipSetAllocCallback(chan, afail_poll, 23);
		ipSetReceiveCallback(chan, rfail_poll, 24);

		while (TRUE) {
		   int nthisrec;
		   int reply_requested = FALSE;

		    status = receiveATL(chan, 1, &actual_words, &pdata);
		    while (pdata && actual_words) {
			if ((nthisrec = *pdata) < 0) {
			    reply_requested = TRUE;
			    nthisrec = -nthisrec;
			}
			if (!eqlblk(pdata+1, nthisrec-1)) {
			    printf("block not equal at %8.8x\n", pdata);
			    save_verbose = verbose;
			    verbose = 1;
			    PrintData(pdata, nthisrec);
			    verbose = save_verbose;
			}
			pdata += nthisrec;
			actual_words -= nthisrec;
			ipFreeData(chan, nthisrec);
			if (reply_requested) {
			    printf("send/RECEIVE: sending reply\n");
			    allocate(chan, 10);
			    *pcurBuf = REPLY_HEADER;
			    send(chan);
			}
		    } /* end while */
		} /* end while */
		break;
	      }
	    default:
		break;
	    } /* end switch */
	    break;
	  } /* end case 't' */
	case 'v':
	    sscanf(&line[2], "%d\n", &verbose);
	    printf("---> verbose=%d\n", verbose);
	    break;
	case 'z':
	    sscanf(&line[2], "%d\n", &sleepTime);
	    printf("---> sleepTime=%d\n", sleepTime);
	    break;
	case '?':
	    printf("sendBd: lw=%d  lr=%d  lres=%d  hw=%d\n",
		chan->sendBd->lastWritten, chan->sendBd->lastRead,
		chan->sendBd->lastReserved, chan->sendBd->highWater);
	    printf("receiveBd: lw=%d  lr=%d  lres=%d  hw=%d\n",
		chan->receiveBd->lastWritten,
		chan->receiveBd->lastRead,
		chan->receiveBd->lastReserved,
		chan->receiveBd->highWater);
	    break;
	default:
	    printf("?\n");
	} /* end switch() */
    } /* end while */
} /* end main() */
