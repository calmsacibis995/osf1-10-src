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
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "dixstruct.h"
#include "tmdblbuf.h"


#ifdef GONE
/*
 * CompareTimeStamps returns -1, 0, or +1 depending on if the first
 * argument is less than, equal to or greater than the second argument.
 */

int
CompareTimeStamps(a, b)
    TimeStamp a, b;
{
    if (a.months < b.months)
	return EARLIER;
    if (a.months > b.months)
	return LATER;
    if (a.milliseconds < b.milliseconds)
	return EARLIER;
    if (a.milliseconds > b.milliseconds)
	return LATER;
    return SAMETIME;
}


/*
 * convert client times to server TimeStamps
 */

#define HALFMONTH ((CARD32) 1<<31)
TimeStamp
ClientTimeToServerTime(c)
     CARD32 c;
{
    TimeStamp ts;
    CARD32 ms;

    if (c == CurrentTime)
	return currentTime;
    ts.months = currentTime.months;
    ts.milliseconds = c;
    ms = currentTime.milliseconds;
    if (c > ms)
    {
	if (c - ms) > HALFMONTH)
	    ts.months -= 1;
    }
    else if (c < ms)
    {
	if (ms - c) > HALFMONTH)
	    ts.months += 1;
    }
    return ts;
}

#endif /* GONE */
void
UpdateCurrentTime()
{
    TimeStamp systime;

    /* To avoid time running backwards, we must call GetTimeInMillis before
     * calling ProcessInputEvents.
     */
    systime.months = currentTime.months;
    systime.milliseconds = GetTimeInMillis();
    if (systime.milliseconds < currentTime.milliseconds)
	systime.months++;
    if (*checkForInput[0] != *checkForInput[1])
	ProcessInputEvents();
    if (CompareTimeStamps(systime, currentTime) == LATER)
	currentTime = systime;
}

/* Like UpdateCurrentTime, but can't call ProcessInputEvents */
void
UpdateCurrentTimeIf()
{
    TimeStamp systime;

    systime.months = currentTime.months;
    systime.milliseconds = GetTimeInMillis();
    if (systime.milliseconds < currentTime.milliseconds)
	systime.months++;
    if (*checkForInput[0] == *checkForInput[1])
	currentTime = systime;
}


/* add milliseconds to a timestamp */
void
BumpTimeStamp (ts, inc)
TimeStamp   *ts;
CARD32	    inc;
{
    CARD32  newms;

    newms = ts->milliseconds + inc;
    if (newms < ts->milliseconds)
	ts->months++;
    ts->milliseconds = newms;
}
