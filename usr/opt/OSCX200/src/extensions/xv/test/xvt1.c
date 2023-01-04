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
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/*
** File: 
**
**   xvt1.c --- Xv test program 1
**   
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   24.01.91 Carver
**     - version 1.4 upgrade
**       
*/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xvlib.h>


main()

{
  char chr;
  int ii, jj, kk, ll, status, mask, nitems;
  unsigned int actual_w, actual_h;
  int screen;

  Display *dpy;
  Visual *vis,*def_vis;
  XVisualInfo *p_vis_info, vis_info_temp;
  XGCValues gc_attr;
  GC gc;
  XSetWindowAttributes win_attr;
  Window root,main_win;
  Colormap cmap;
  XEvent event;
  XColor scolor,ecolor;

  unsigned int version, revision;
  unsigned int major_opcode;
  unsigned int event_base;
  unsigned int error_base;
  unsigned int nAdaptors, nEncodings;
  XvAdaptorInfo *pAdaptors, *pAdaptor;
  XvEncodingInfo *pEncoding, *pEncodings;
  XvFormat *pFormat;
  XvPortID port;
  XvEncodingID encoding, old_encoding;
  XvEvent *pe;
  float rate;
  Atom encoding_atom;

  printf("\n  Welcome to Xv test program #1\n\n");
  printf("  This program invokes all the Xv functions that are supported\n");
  printf("  by your display.  It will list the available adaptors and\n");
  printf("  their corresponding information.  It will create a window for \n");
  printf("  the display of video and still images.  Then for \n");
  printf("  each adaptor it will get/put a still image into the bottom \n");
  printf("  window and a video image into the top. \n");

  printf("\n> Press return to continue...");
  chr = getc(stdin);

  dpy = XOpenDisplay(0);
  if (!dpy)
    {
      printf("\n  Couldn't open display\n");
      printf("\n  Xv test program #1 terminated\n");
      exit();
    }

  root = XDefaultRootWindow(dpy);
  screen = XDefaultScreen(dpy);

  XSynchronize(dpy, True);

  printf("\n  QueryExtension Request\n");

  status = XvQueryExtension(dpy, &version, &revision,
			    &major_opcode, &event_base, &error_base);

  if (status != Success) 
    {
      printf("\n  Xv video extension not available\n");
      printf("\n  Xv test program #1 terminated\n");
      exit();
    }

  printf("    Version = %d\n    Revision = %d\n", version, revision);
  printf("    Opcode = %d\n    Event Base = %d\n    Error Base = %d\n",
	 major_opcode, event_base, error_base);

  printf("\n");

  printf("\n  QueryAdaptors Request\n");

  status = XvQueryAdaptors(dpy, root, &nAdaptors, &pAdaptors);

  if (status != Success) 
    {
      printf("\n  XvQueryAdaptors failed with code %d\n", status);
      printf("\n  Xv test program #1 terminated\n");
      exit();
    }

  if (!nAdaptors)
    {
      printf("\n  Your display has no video adaptors\n");
      printf("\n  Xv test program #1 terminated\n");
      exit();
    }

  printf("\n");
  printf("  Number of Adaptors: %d\n", nAdaptors);

  printf("\n");

  pAdaptor = pAdaptors;
  for (ii=0; ii<nAdaptors; ii++)
    {
      printf("  Adaptor Info #%d\n", ii+1);
      printf("    name:                 %s\n",pAdaptor->name);
      printf("    type:                 %1x (hex)\n", pAdaptor->type);
      printf("    base_id:              %x (hex)\n", pAdaptor->base_id);
      printf("    num_ports:            %d\n", pAdaptor->num_ports);
      printf("    num_formats:          %d\n", pAdaptor->num_formats);
      printf("\n");

      vis_info_temp.visualid = 0;

      pFormat = pAdaptor->formats;
      for (jj=0; jj<pAdaptor->num_formats; jj++)
	{
	  printf("\n");
	  printf("    Format #%d\n", jj+1);
	  printf("      visual_id:          %x\n",pFormat->visual_id);
	  printf("      depth:              %d\n",pFormat->depth);
	  pFormat++;
	}
      
      pAdaptor++;

      printf("\n> Press <ret> to continue ");
      printf("or s<ret> to skip remaining adaptors...");
      chr = getc(stdin);
      if (chr == 's') break;

    }

  encoding_atom = XInternAtom(dpy,"XV_ENCODING",False);

  pAdaptor = pAdaptors;
  ii=0;
  while (ii<nAdaptors)
    {
      printf("\n  Testing adaptor #%d\n", ii+1);

      printf("\n  XvQueryEncodings\n");

      XvQueryEncodings(dpy, pAdaptor->base_id, &nEncodings, &pEncodings);

      pEncoding = pEncodings;
      for (jj=0; jj<nEncodings; jj++)
	{
	  printf("    Encoding Info #%d\n", jj+1);
	  printf("      encoding_id:        %d\n", pEncoding->encoding_id);
	  printf("      name:               %s\n", pEncoding->name);
	  printf("      width:              %d\n", pEncoding->width);
	  printf("      height:             %d\n", pEncoding->height);

	  rate = pEncoding->rate.numerator;
	  rate = rate / pEncoding->rate.denominator;
	  printf("      rate:               %.2f\n", rate);

	  if (strcmp(pEncoding->name, "ntsc-svideo") == 0)
	    encoding = pEncoding->encoding_id;

	  pEncoding++;
	}

      pFormat = pAdaptor->formats;

      jj=0;
      while (jj<pAdaptor->num_formats)
	{
	  printf("\n    Testing format #%d\n", jj+1);

	  vis_info_temp.visualid = pFormat->visual_id;
	  p_vis_info = XGetVisualInfo(dpy, VisualIDMask, 
				      &vis_info_temp, &nitems);
	  if (!p_vis_info)
	    {
	      printf("      Error: Couldn't find visual ");
	      printf("#%x listed for adaptor.\n", pFormat->visual_id);
	      break;
	    }
	  vis = p_vis_info->visual;

	  def_vis = XDefaultVisual(dpy,screen);
	  if (vis->visualid == def_vis->visualid)
	    cmap = XDefaultColormap(dpy,screen);
	  else
	    cmap = XCreateColormap(dpy, root, vis, AllocNone);

	  XAllocNamedColor(dpy, cmap, "midnight blue", &scolor, &ecolor);

	  win_attr.colormap = cmap;
	  win_attr.background_pixel = scolor.pixel;
	  win_attr.event_mask = ExposureMask;
	  win_attr.backing_store = Always;
	  win_attr.border_pixel = scolor.pixel;

	  main_win = XCreateWindow(dpy, root, 0, 0, 672, 966, 0, 
				   pFormat->depth, InputOutput, vis,
				   CWColormap | CWBackPixel | CWEventMask |
				   CWBackingStore | CWBorderPixel,
				   &win_attr);

	  XMapWindow(dpy, main_win);

	  printf("\n      Waiting for window to become visible...\n");
	  while (1)
	    {
	      XNextEvent(dpy, &event);
	      if (event.type == Expose) break;
	    }

	  printf("\n      SelectVideoNotify on window\n");
	  XvSelectVideoNotify(dpy, main_win, True);

	  gc_attr.foreground = scolor.pixel;
	  gc = XCreateGC(dpy, main_win, GCForeground, &gc_attr);

	  port = pAdaptor->base_id;

	  kk=0;
	  while (kk<pAdaptor->num_ports)
	    {
	      printf("\n      Testing port #%x (hex)\n", port);

	      printf("\n        Enable PortNotify events\n");
	      XvSelectPortNotify(dpy, port, True);

	      XvGetPortAttribute(dpy, port, 
				 encoding_atom, (int *)&old_encoding);

	      printf("\n        GetPortAttribute: encoding = %d\n", 
		     old_encoding);

	      printf("\n        SetPortAttribute: encoding = %d\n", encoding);
	      
	      XvSetPortAttribute(dpy, port, encoding_atom, encoding);

	      printf("\n        Waiting for port notify event ...\n");

	      while (1)
		{
		  XNextEvent(dpy, &event);
		  if (event.type == event_base + XvPortNotify)
		    {
		      pe = (XvEvent *)&event;

		      if (pe->xvport.attribute == encoding_atom)
			printf("          Port encoding changed to %d\n", 
			       pe->xvport.value);
		      else
			printf("          Got unexpected port notify event\n");
		      break;
		    }
		}

	      printf("\n        Disable PortNotify events\n");
	      XvSelectPortNotify(dpy, port, False);

	      XvGetPortAttribute(dpy, port, encoding_atom, (int *)&encoding);

	      printf("\n        GetPortAttribute: encoding = %d\n", encoding);
	      printf("\n        SetPortAttribute: encoding = %d\n", 
		     old_encoding);

	      XvSetPortAttribute(dpy, port, encoding_atom, old_encoding);

	      printf("\n        QueryBestSize\n");
	      printf("          motion:           True\n");
	      printf("          width:            %d\n", 640);
	      printf("          height:           %d\n", 480);

	      XvQueryBestSize(dpy, port, True, 640, 480, 640, 480, 
			      &actual_w, &actual_h);

	      printf("\n        QueryBestSize Reply\n");
	      printf("          actual width:      %d\n", actual_w);
	      printf("          actual height:     %d\n", actual_h);
	      
	      printf("\n        GrabPort\n");

	      status = XvGrabPort(dpy, port, CurrentTime);
	      if (status == Success)
		{
		  printf("          Port Grabbed!\n");
		}
	      else if (status == XvAlreadyGrabbed)
		printf("        Port Already Grabbed!\n");
	      else if (status == XvInvalidTime)
		printf("          Invalid Grab Time!\n");

	      printf("\n        UngrabPort\n");
	      XvUngrabPort(dpy, port, CurrentTime);

	      if (pAdaptor->type & XvInputMask)
		{
		  printf("\n        PutVideo\n");
		  XvPutVideo(dpy, port, main_win, gc, 0, 0, 
			     640, 480, 16, 0, 640, 480);
		  
		  printf("\n        Waiting for video Started event ...\n");
		  while (1)
		    {
		      XNextEvent(dpy, &event);
		      if (event.type == event_base + XvVideoNotify)
			{
			  pe = (XvEvent *)&event;
			  if (pe->xvvideo.reason == XvStarted)
			    {
			      printf("          Video started\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvBusy)
			    {
			      printf("          Port busy\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvHardError)
			    {
			      printf("          Port error\n");
			      break;
			    }
			}
		    }

		  printf("\n> Press <ret> to PutStill...");
		  chr = getc(stdin);

		  printf("\n        PutStill\n");
		  XvPutStill(dpy, port, main_win, gc, 0, 0, 
			     640, 480, 16, 483, 640, 480);

		  printf("\n> Press <ret> to StopVideo...");
		  chr = getc(stdin);

		  printf("\n        StopVideo\n");
		  XvStopVideo(dpy, port, main_win);

		  printf("\n        Waiting for video Stopped event ...\n");
		  while (1)
		    {
		      XNextEvent(dpy, &event);
		      if (event.type == event_base)
			{
			  pe = (XvEvent *)&event;
			  if (pe->xvvideo.reason == XvStopped)
			    {
			      printf("          Video stopped\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvPreempted)
			    {
			      printf("          Video preempted\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvHardError)
			    {
			      printf("          Video hard error\n");
			      break;
			    }
			}
		    }
		}

	      if (pAdaptor->type & XvOutputMask)
		{
		  printf("\n        GetVideo\n");
		  XvGetVideo(dpy, port, main_win, gc, 0, 0, 
			     640, 480, 16, 0, 640, 480);
		  
		  printf("\n        Waiting for video Started event ...\n");
		  while (1)
		    {
		      XNextEvent(dpy, &event);
		      if (event.type == event_base)
			{
			  pe = (XvEvent *)&event;
			  if (pe->xvvideo.reason == XvStarted)
			    {
			      printf("          Video started\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvBusy)
			    {
			      printf("          Port busy\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvHardError)
			    {
			      printf("          Port error\n");
			      break;
			    }
			}
		    }

		  printf("\n> Press <ret> to GetStill...");
		  chr = getc(stdin);

		  printf("\n        GetStill\n");
		  XvGetStill(dpy, port, main_win, gc, 0, 0, 
			     640, 480, 16, 483, 640, 480);

		  printf("\n> Press <ret> to StopVideo...");
		  chr = getc(stdin);

		  printf("\n        StopVideo\n");
		  XvStopVideo(dpy, port, main_win);

		  printf("\n        Waiting for video Stopped event ...\n");
		  while (1)
		    {
		      XNextEvent(dpy, &event);
		      if (event.type == event_base)
			{
			  pe = (XvEvent *)&event;
			  if (pe->xvvideo.reason == XvStopped)
			    {
			      printf("          Video stopped\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvPreempted)
			    {
			      printf("          Video preempted\n");
			      break;
			    }
			  if (pe->xvvideo.reason == XvHardError)
			    {
			      printf("          Video hard error\n");
			      break;
			    }
			}
		    }
		}

	      port++;

	      kk++;

	      if (kk<pAdaptor->num_ports)
		{
		  printf("\n> Press <ret> to continue ");
		  printf("or s<ret> to skip remaining ports...");
		  chr = getc(stdin);
		  if (chr == 's') break;
		}

	    }

	  XDestroyWindow(dpy, main_win);
	  XFreeGC(dpy, gc);

	  pFormat++;

	  jj++;

	  if (jj<pAdaptor->num_formats)
	    {
	      printf("\n> Press <ret> to continue ");
	      printf("or s<ret> to skip remaining formats...");
	      chr = getc(stdin);
	      if (chr == 's') break;
	    }
	}

      pAdaptor++;

      ii++;

      if (ii<nAdaptors)
	{
	  printf("\n> Press <ret> to continue ");
	  printf("or s<ret> to skip remaining adaptors...");
	  chr = getc(stdin);
	  if (chr == 's') break;
	}
    }

  printf("\n  Xv test program #1 finished\n");

}
