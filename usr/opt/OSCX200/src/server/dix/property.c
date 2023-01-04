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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/* $XConsortium: property.c,v 5.7 92/10/19 16:10:58 rws Exp $ */

#include "X.h"
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xproto.h"
#include "windowstr.h"
#include "propertyst.h"
#include "dixstruct.h"

extern void (*ReplySwapVector[]) ();
extern void CopySwap16Write(), CopySwap32Write(), Swap32Write();
extern int WriteToClient();

/*****************************************************************
 * Property Stuff
 *
 *    ChangeProperty, DeleteProperty, GetProperties,
 *    ListProperties
 *
 *   Properties below to windows.  A allocate slots each time
 *   a property is added.  No fancy searching done.
 *
 *****************************************************************/

#ifdef notdef
static void
PrintPropertys(pWin)
    WindowPtr pWin;
{
    PropertyPtr pProp;
    register int j;

    pProp = pWin->userProps;
    while (pProp)
    {
        ErrorF(  "%x %x\n", pProp->propertyName, pProp->type);
        ErrorF("property format: %d\n", pProp->format);
        ErrorF("property data: \n");
        for (j=0; j<(pProp->format/8)*pProp->size; j++)
           ErrorF("%c\n", pProp->data[j]);
        pProp = pProp->next;
    }
}
#endif

int
ProcRotateProperties(client)
    ClientPtr client;
{
    int     i, j, delta;
    REQUEST(xRotatePropertiesReq);
    WindowPtr pWin;
    /* The data in the protocol stream is CARD32, while Atom is long unsigned.
     * These may be different sizes. */
    register    CARD32 * atoms;
    PropertyPtr * props;               /* array of pointer */
    PropertyPtr pProp;
    xEvent event;

    REQUEST_FIXED_SIZE(xRotatePropertiesReq, stuff->nAtoms << 2);
    UpdateCurrentTime();
    pWin = (WindowPtr) LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    if (!stuff->nAtoms)
	return(Success);
    atoms = (CARD32 *) & stuff[1];
    props = (PropertyPtr *)ALLOCATE_LOCAL(stuff->nAtoms * sizeof(PropertyPtr));
    if (!props)
	return(BadAlloc);
    for (i = 0; i < stuff->nAtoms; i++)
    {
        if (!ValidAtom((Atom)atoms[i]))
        {
            DEALLOCATE_LOCAL(props);
	    client->errorValue = (Atom) atoms[i];
            return BadAtom;
        }
        for (j = i + 1; j < stuff->nAtoms; j++)
            if (atoms[j] == atoms[i])
            {
                DEALLOCATE_LOCAL(props);
                return BadMatch;
            }
        pProp = wUserProps (pWin);
        while (pProp)
        {
            if (pProp->propertyName == (Atom) atoms[i])
                goto found;
	    pProp = pProp->next;
        }
        DEALLOCATE_LOCAL(props);
        return BadMatch;
found: 
        props[i] = pProp;
    }
    delta = stuff->nPositions;

    /* If the rotation is a complete 360 degrees, then moving the properties
	around and generating PropertyNotify events should be skipped. */

    if ( (stuff->nAtoms != 0) && (abs(delta) % stuff->nAtoms) != 0 ) 
    {
	while (delta < 0)                  /* faster if abs value is small */
            delta += stuff->nAtoms;
    	for (i = 0; i < stuff->nAtoms; i++)
 	{
	    /* Generate a PropertyNotify event for each property whose value
		is changed in the order in which they appear in the request. */
 
 	    event.u.u.type = PropertyNotify;
            event.u.property.window = pWin->drawable.id;
    	    event.u.property.state = PropertyNewValue;
	    event.u.property.atom = props[i]->propertyName;	
	    event.u.property.time = currentTime.milliseconds;
	    DeliverEvents(pWin, &event, 1, (WindowPtr)NULL);
	
            props[i]->propertyName = (Atom) atoms[(i + delta) % stuff->nAtoms];
	}
    }
    DEALLOCATE_LOCAL(props);
    return Success;
}

int 
ProcChangeProperty(client)
    ClientPtr client;
{	      
    WindowPtr pWin;
    char format, mode;
    unsigned long len;
    int sizeInBytes;
    int totalSize;
    register int err;
    REQUEST(xChangePropertyReq);

    REQUEST_AT_LEAST_SIZE(xChangePropertyReq);
    UpdateCurrentTime();
    format = stuff->format;
    mode = stuff->mode;
    if ((mode != PropModeReplace) && (mode != PropModeAppend) &&
	(mode != PropModePrepend))
    {
	client->errorValue = mode;
	return BadValue;
    }
    if ((format != 8) && (format != 16) && (format != 32))
    {
	client->errorValue = format;
        return BadValue;
    }
    len = stuff->nUnits;
    if (len > ((0xffffffff - sizeof(xChangePropertyReq)) >> 2))
	return BadLength;
    sizeInBytes = format>>3;
    totalSize = len * sizeInBytes;
    REQUEST_FIXED_SIZE(xChangePropertyReq, totalSize);

    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
	return(BadWindow);
    if (!ValidAtom((Atom)stuff->property))
    {
	client->errorValue = (Atom)stuff->property;
	return(BadAtom);
    }
    if (!ValidAtom((Atom)stuff->type))
    {
	client->errorValue = (Atom)stuff->type;
	return(BadAtom);
    }

    err = ChangeWindowProperty(pWin, (Atom)stuff->property, 
	 			(Atom)stuff->type, (int)format,
			        (int)mode, (long)len, (pointer)&stuff[1], TRUE);
    if (err != Success)
	return err;
    else
	return client->noClientException;
}

int
ChangeWindowProperty(pWin, property, type, format, mode, len, value, sendevent)
    WindowPtr	pWin;
    Atom	property, type;
    int		format, mode;
    unsigned long len;
    pointer	value;
    Bool	sendevent;
{
    PropertyPtr pProp;
    xEvent event;
    int sizeInBytes;
    int totalSize;
    pointer data;

    sizeInBytes = format>>3;
    totalSize = len * sizeInBytes;

    /* first see if property already exists */

    pProp = wUserProps (pWin);
    while (pProp)
    {
	if (pProp->propertyName == property)
	    break;
	pProp = pProp->next;
    }
    if (!pProp)   /* just add to list */
    {
	if (!pWin->optional && !MakeWindowOptional (pWin))
	    return(BadAlloc);
        pProp = (PropertyPtr)xalloc(sizeof(PropertyRec));
	if (!pProp)
	    return(BadAlloc);
        data = (pointer)xalloc(totalSize);
	if (!data && len)
	{
	    xfree(pProp);
	    return(BadAlloc);
	}
        pProp->propertyName = property;
        pProp->type = type;
        pProp->format = format;
        pProp->data = data;
	if (len)
	    bcopy((char *)value, (char *)data, totalSize);
	pProp->size = len;
        pProp->next = pWin->optional->userProps;
        pWin->optional->userProps = pProp;
    }
    else
    {
	/* To append or prepend to a property the request format and type
		must match those of the already defined property.  The
		existing format and type are irrelevant when using the mode
		"PropModeReplace" since they will be written over. */

        if ((format != pProp->format) && (mode != PropModeReplace))
	    return(BadMatch);
        if ((pProp->type != type) && (mode != PropModeReplace))
            return(BadMatch);
        if (mode == PropModeReplace) 
        {
	    if (totalSize != pProp->size * (pProp->format >> 3))
	    {
	    	data = (pointer)xrealloc(pProp->data, totalSize);
	    	if (!data && len)
		    return(BadAlloc);
            	pProp->data = data;
	    }
	    if (len)
		bcopy((char *)value, (char *)pProp->data, totalSize);
	    pProp->size = len;
    	    pProp->type = type;
	    pProp->format = format;
	}
	else if (len == 0)
	{
	    /* do nothing */
	}
        else if (mode == PropModeAppend)
        {
	    data = (pointer)xrealloc(pProp->data,
				     sizeInBytes * (len + pProp->size));
	    if (!data)
		return(BadAlloc);
            pProp->data = data;
	    bcopy((char *)value,
		  &((char *)data)[pProp->size * sizeInBytes], 
		  totalSize);
            pProp->size += len;
	}
        else if (mode == PropModePrepend)
        {
            data = (pointer)xalloc(sizeInBytes * (len + pProp->size));
	    if (!data)
		return(BadAlloc);
	    bcopy((char *)pProp->data, &((char *)data)[totalSize], 
		  (int)(pProp->size * sizeInBytes));
            bcopy((char *)value, (char *)data, totalSize);
	    xfree(pProp->data);
            pProp->data = data;
            pProp->size += len;
	}
    }
    event.u.u.type = PropertyNotify;
    event.u.property.window = pWin->drawable.id;
    event.u.property.state = PropertyNewValue;
    event.u.property.atom = pProp->propertyName;
    event.u.property.time = currentTime.milliseconds;
    DeliverEvents(pWin, &event, 1, (WindowPtr)NULL);

    return(Success);
}

DeleteProperty(pWin, propName)
    WindowPtr pWin;
    Atom propName;
{
    PropertyPtr pProp, prevProp;
    xEvent event;

    if (!(pProp = wUserProps (pWin)))
	return(Success);
    prevProp = (PropertyPtr)NULL;
    while (pProp)
    {
	if (pProp->propertyName == propName)
	    break;
        prevProp = pProp;
	pProp = pProp->next;
    }
    if (pProp) 
    {		    
        if (prevProp == (PropertyPtr)NULL)      /* takes care of head */
        {
            if (!(pWin->optional->userProps = pProp->next))
		CheckWindowOptionalNeed (pWin);
        }
	else
        {
            prevProp->next = pProp->next;
        }
	event.u.u.type = PropertyNotify;
	event.u.property.window = pWin->drawable.id;
	event.u.property.state = PropertyDelete;
        event.u.property.atom = pProp->propertyName;
	event.u.property.time = currentTime.milliseconds;
	DeliverEvents(pWin, &event, 1, (WindowPtr)NULL);
	xfree(pProp->data);
        xfree(pProp);
    }
    return(Success);
}

DeleteAllWindowProperties(pWin)
    WindowPtr pWin;
{
    PropertyPtr pProp, pNextProp;
    xEvent event;

    pProp = wUserProps (pWin);
    while (pProp)
    {
	event.u.u.type = PropertyNotify;
	event.u.property.window = pWin->drawable.id;
	event.u.property.state = PropertyDelete;
	event.u.property.atom = pProp->propertyName;
	event.u.property.time = currentTime.milliseconds;
	DeliverEvents(pWin, &event, 1, (WindowPtr)NULL);
	pNextProp = pProp->next;
        xfree(pProp->data);
        xfree(pProp);
	pProp = pNextProp;
    }
}

/*****************
 * GetProperty
 *    If type Any is specified, returns the property from the specified
 *    window regardless of its type.  If a type is specified, returns the
 *    property only if its type equals the specified type.
 *    If delete is True and a property is returned, the property is also
 *    deleted from the window and a PropertyNotify event is generated on the
 *    window.
 *****************/

int
ProcGetProperty(client)
    ClientPtr client;
{
    PropertyPtr pProp, prevProp;
    unsigned long n, len, ind;
    WindowPtr pWin;
    xGetPropertyReply reply;
    REQUEST(xGetPropertyReq);

    REQUEST_SIZE_MATCH(xGetPropertyReq);
    if (stuff->delete)
	UpdateCurrentTime();
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (pWin)
    {
	if (!ValidAtom((Atom)stuff->property))
	{
	    client->errorValue = (Atom)stuff->property;
	    return(BadAtom);
	}
	if ((stuff->delete != xTrue) && (stuff->delete != xFalse))
	{
	    client->errorValue = stuff->delete;
	    return(BadValue);
	}
	if ((stuff->type == AnyPropertyType) || ValidAtom((Atom)stuff->type))
	{
	    pProp = wUserProps (pWin);
            prevProp = (PropertyPtr)NULL;
            while (pProp)
            {
	        if (pProp->propertyName == stuff->property) 
	            break;
		prevProp = pProp;
		pProp = pProp->next;
            }
	    reply.type = X_Reply;
	    reply.sequenceNumber = client->sequence;
            if (pProp) 
            {

		/* If the request type and actual type don't match. Return the
		property information, but not the data. */

                if ((stuff->type != pProp->type) &&
		    (stuff->type != AnyPropertyType))
		{
		    reply.bytesAfter = pProp->size;
		    reply.format = pProp->format;
		    reply.length = 0;
		    reply.nItems = 0;
		    reply.propertyType = pProp->type;
		    WriteReplyToClient(client, sizeof(xGenericReply), &reply);
		    return(Success);
		}

	    /*
             *  Return type, format, value to client
             */
		n = (pProp->format/8) * pProp->size; /* size (bytes) of prop */
		ind = stuff->longOffset << 2;        

               /* If longOffset is invalid such that it causes "len" to
                        be negative, it's a value error. */

		if (n < ind)
		{
		    client->errorValue = stuff->longOffset;
		    return BadValue;
		}

		len = min(n - ind, 4 * stuff->longLength);

		reply.bytesAfter = n - (ind + len);
		reply.format = pProp->format;
		reply.length = (len + 3) >> 2;
		reply.nItems = len / (pProp->format / 8 );
		reply.propertyType = pProp->type;

                if (stuff->delete && (reply.bytesAfter == 0))
                { /* send the event */
		    xEvent event;
		
		    event.u.u.type = PropertyNotify;
		    event.u.property.window = pWin->drawable.id;
		    event.u.property.state = PropertyDelete;
		    event.u.property.atom = pProp->propertyName;
		    event.u.property.time = currentTime.milliseconds;
		    DeliverEvents(pWin, &event, 1, (WindowPtr)NULL);
		}

		WriteReplyToClient(client, sizeof(xGenericReply), &reply);
		if (len)
		{
		    switch (reply.format) {
		    case 32: client->pSwapReplyFunc = CopySwap32Write; break;
		    case 16: client->pSwapReplyFunc = CopySwap16Write; break;
		    default: client->pSwapReplyFunc = (void (*) ())WriteToClient; break;
		    }
		    WriteSwappedDataToClient(client, len, pProp->data + ind);
		}

                if (stuff->delete && (reply.bytesAfter == 0))
                { /* delete the Property */
                    if (prevProp == (PropertyPtr)NULL) /* takes care of head */
		    {
                        if (!(pWin->optional->userProps = pProp->next))
			    CheckWindowOptionalNeed (pWin);
		    }
	            else
                        prevProp->next = pProp->next;
		    xfree(pProp->data);
                    xfree(pProp);
		}
	    }
            else 
	    {   
                reply.nItems = 0;
		reply.length = 0;
		reply.bytesAfter = 0;
		reply.propertyType = None;
		reply.format = 0;
		WriteReplyToClient(client, sizeof(xGenericReply), &reply);
	    }
            return(client->noClientException);

	}
        else
	{
	    client->errorValue = stuff->type;
            return(BadAtom);
	}
    }
    else            
        return (BadWindow); 
}

int
ProcListProperties(client)
    ClientPtr client;
{
    Atom *pAtoms, *temppAtoms;
    xListPropertiesReply xlpr;
    int	numProps = 0;
    WindowPtr pWin;
    PropertyPtr pProp;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow(stuff->id, client);
    if (!pWin)
        return(BadWindow);

    pProp = wUserProps (pWin);
    while (pProp)
    {        
        pProp = pProp->next;
	numProps++;
    }
    if (numProps)
	/* Atom is represented by CARD32 in protocol stream */
        if(!(pAtoms = (Atom *)ALLOCATE_LOCAL(numProps * sizeof(CARD32))))
            return(BadAlloc);

    xlpr.type = X_Reply;
    xlpr.nProperties = numProps;
    xlpr.sequenceNumber = client->sequence;
    if ( sizeof(Atom) != sizeof(CARD32) ) 
    {
	CARD32 * temppAtoms32;
	temppAtoms32 = (CARD32 *)pAtoms;

        xlpr.length = (numProps * sizeof(CARD32)) >> 2;
        pProp = wUserProps (pWin);
        temppAtoms = pAtoms;
        while (pProp)
        {
	    *temppAtoms32++ = (CARD32)pProp->propertyName;
	    pProp = pProp->next;
        }
    }
    else {
        xlpr.length = (numProps * sizeof(Atom)) >> 2;
        pProp = wUserProps (pWin);
        temppAtoms = pAtoms;
        while (pProp)
        {
	    *temppAtoms++ = pProp->propertyName;
	    pProp = pProp->next;
        }
    }
    WriteReplyToClient(client, sizeof(xGenericReply), &xlpr);
    if (numProps)
    {
        client->pSwapReplyFunc = Swap32Write;
        WriteSwappedDataToClient(client, numProps * sizeof(CARD32), pAtoms);
        DEALLOCATE_LOCAL(pAtoms);
    }
    return(client->noClientException);
}
#ifdef MODE_SWITCH
/* 
 * This routine manages the property _DEC_KEYMAP_MODE. Currently (5/91)
 * this property is used to notify the keyboard state change from LDM on to 
 * LDM off.  If the caller specifies a non-zero pWin, we have to remember it
 * and create the atom.   If zero pWin is passed, then just update the 
 * the property value.
 */ 
#define DEC_KEYMAP_MODE "_DEC_KEYMAP_MODE"    /* user-defined property name*/

int
SetKeymapModeProperty(pWin, keycode, type, state)
    WindowPtr  pWin;
    short   keycode, type, state;
{
    static ATOM	     dec_keymap_mode = None;
    static WindowPtr pScreen0_window = NULL;
    struct {
	   short keycode, type, state;
	   } data;

    if (pWin != (WindowPtr)0){
	dec_keymap_mode = MakeAtom (DEC_KEYMAP_MODE, strlen(DEC_KEYMAP_MODE), 1);
	pScreen0_window = pWin;
    }

    data.keycode    = keycode;
    data.type	    = type;
    data.state	    = state;

    return SetProperty
           (pScreen0_window, dec_keymap_mode, dec_keymap_mode, 16, 3, 
            &data);
}


/* Set the type, format and value of a property.  If it doesn't
 * exist, create it.  Note that it is perfectly legal to have
 * zero length data.
 */

int
SetProperty(pWin, property, type, format, len, data)
    WindowPtr pWin;
    ATOM property, type;
    unsigned int format, len;
    pointer data;
{
    PropertyPtr pProp;
    int sizeInBytes;
    xEvent event;

    /* Validate the arguments. */

    if (!pWin)
	return BadWindow;

    if (!(ValidAtom(property)  && ValidAtom(type)))
	return BadAtom;

    if ((format != 8) && (format != 16) && (format != 32))
        return BadValue;

    sizeInBytes = format>>3;

    /* first see if property already exists */

    pProp = wUserProps(pWin);

    while (pProp)
    {
	if (pProp->propertyName == property) 
	    break;
	pProp = pProp->next;
    }

    if (!pProp)
    {
	/* It doesn't exist.  Add it to the list.  Starting by
	 * allocating the necessary memory.
	 */

	if (!pWin->optional && !MakeWindowOptional (pWin))
            return BadAlloc;
        pProp = (PropertyPtr )xalloc(sizeof(PropertyRec));
	if (!pProp)
	    return BadAlloc;

	/* Allocated the Property structure.  Now initialize it. */

	pProp->propertyName = property;
	pProp->type = type;
	pProp->format = format;
	pProp->size = len;

	/* Was any data specified? */

	if (len)
	{
	    if (len && !(pProp->data = (pointer)xalloc(sizeInBytes  * len)))
	    {
		Xfree(pProp);
		return BadAlloc;
	    }

	    bcopy(data, pProp->data, len * sizeInBytes);
        }
	else
	    pProp->data = (pointer)0;

	/* Prepend the property to the list of properties. */

	pProp->next = pWin->optional->userProps;
	pWin->optional->userProps = pProp;
    }
    else
    {
	/* The property already exists.  Change its value to the
	 * requested value (as in PropModeReplace).
	 *
	 * If the current data buffer is not the size we need, then
	 * replace it with one the size that we need.
	 */

	if (len)
	{
	    if (((pProp->size * (pProp->format >> 3)) != sizeInBytes))
	    {
		pointer new_data = (pointer)xalloc(sizeInBytes * len);
		
		if (!new_data)
		    return BadAlloc;

		if (pProp->data)
		    Xfree((char *) pProp->data);

		pProp->data = new_data;
	    }

	    /* Update the data. */

	    bcopy(data, pProp->data, len * sizeInBytes);
	}
	else
	{
	    if (pProp->data)
		Xfree((char *) pProp->data);

	    pProp->data = (pointer)0;
	}

	/* Update the property. */

	pProp->size = len;
	pProp->type = type;
	pProp->format = format;
    }

    /* Now notify any interested clients of the change in the property. */

    event.u.u.type = PropertyNotify;
    event.u.property.window = pWin->drawable.id;
    event.u.property.state = PropertyNewValue;
    event.u.property.atom = pProp->propertyName;
    event.u.property.time = currentTime.milliseconds;
    DeliverEvents(pWin, &event, 1, (WindowPtr)NULL);

    return Success;
}
#endif
