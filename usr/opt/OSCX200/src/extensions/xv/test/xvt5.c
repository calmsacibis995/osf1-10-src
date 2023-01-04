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
**   xvt5.c --- Xv test program 5
**   
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   28.05.91 Carver
**     - origional
**       
*/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xvlib.h>


main(argc, argv)
     int         argc;
     char        *argv[];
{
  char chr;
  int ii, jj, status, in_motion;
  unsigned int mask;
  int screen;
  int dx, dy, dw, dh, px, py, cx, cy, vx, vy, vw, vh;
  int rx,ry,wx,wy;
  int delta;

  Display *dpy;
  Visual *vis,*def_vis;
  XVisualInfo *p_vis_info, vis_info_tmpl;
  unsigned long vis_id;
  unsigned long depth;
  XGCValues gc_attr;
  GC gc,stillgc;
  XSetWindowAttributes win_attr;
  Window root,main_win,child;
  Pixmap mskpix;
  Colormap cmap;
  XEvent event;
  XColor scolor,ecolor;

  unsigned int version, revision;
  unsigned int event_base;
  unsigned int error_base;
  unsigned int major_opcode;
  unsigned int nAdaptors;
  XvAdaptorInfo *pAdaptors, *pAdaptor;
  XvEncodingInfo *pEncoding;
  XvFormat *pFormat;
  unsigned long port;
  unsigned long enc_id;
  XvEvent *pe;
  float rate;
  int video;

  printf("\n  Welcome to Xv test program #5\n\n");
  printf("  This program uses the mouse to move and size the video image.\n");
  printf("  After video appears click in video window and move the mouse\n");
  printf("  to move the video image.  Click on side grips to resize the\n");
  printf("  video image.  Press ^c exit.\n");

  printf("\n> Press return to continue...");
  chr = getc(stdin);

  dpy = XOpenDisplay(0);
  if (!dpy)
    {
      printf("\n  Couldn't open display\n");
      printf("\n  Xv test program #5 terminated\n");
      exit();
    }

  root = XDefaultRootWindow(dpy);
  screen = XDefaultScreen(dpy);

/*  XSynchronize(dpy, True); */

  status = XvQueryExtension(dpy, &version, &revision,
			    &major_opcode, &event_base, &error_base);

  if (status != Success) 
    {
      printf("\n  Xv video extension not available\n");
      printf("\n  Xv test program #5 terminated\n");
      exit();
    }

  XvQueryAdaptors(dpy, root, &nAdaptors, &pAdaptors);

  printf("\n  Xv V%01d.%d\n", version, revision);

  if (!nAdaptors)
    {
      printf("\n  Your display has no video adaptors\n");
      printf("\n  Xv test program #5 terminated\n");
      exit();
    }

 if (!Setup(argc, argv, nAdaptors, pAdaptors, &port, &depth, &vis_id))
   {
     printf("\n  Setup failed.\n");
     printf("\n  Xv test program #5 terminated\n");
     exit();
   }


  vis_info_tmpl.visualid = vis_id;
  p_vis_info = XGetVisualInfo(dpy, VisualIDMask, &vis_info_tmpl, &ii);
  if (!p_vis_info)
    {
      printf("      Error: Couldn't find visual ");
      printf("#%x listed for adaptor.\n", pFormat->visual_id);
      printf("\n  Xv test program #5 terminated\n");
      return;
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
  win_attr.event_mask = ExposureMask | VisibilityChangeMask | 
    ButtonPressMask | ButtonReleaseMask | KeyPressMask;
  win_attr.border_pixel = scolor.pixel;

  main_win = XCreateWindow(dpy, root, 0, 0, 672, 512, 0, 
			   depth, InputOutput, vis,
			   CWColormap | CWBackPixel | CWEventMask |
			   CWBorderPixel,
			   &win_attr);

  XMapWindow(dpy, main_win);

  printf("\n  Waiting for window to become visible...\n");
  while (1)
    {
      XNextEvent(dpy, &event);
      if (event.type == VisibilityNotify) break;
    }

  printf("\n  SelectNotify on window\n");
  XvSelectVideoNotify(dpy, main_win, True);

  gc_attr.foreground = 0xffffff;
  gc = XCreateGC(dpy, main_win, 
		 GCForeground,
		 &gc_attr);

  printf("\n  PutVideo\n\n");
  XvPutVideo(dpy, port, main_win, gc, 0, 0, 
	     640, 480, 16, 16, 640, 480);

  delta = 16;

  dx = 16;
  dy = 16;
  dw = 640;
  dh = 480;

  while (1)
    {
      XNextEvent(dpy, &event);

      if (event.type == KeyPress)
	{
	  if ((XLookupKeysym(&event.xkey,0) == 'c') && 
	      (event.xkey.state & ControlMask))
	    {
	      break;
	    }
	}
      else if ((event.type == ButtonPress) && 
	       (event.xbutton.button == Button1))
	{

	  if ((event.xbutton.x > dx - delta) && 
	      (event.xbutton.x < dx + delta) &&
	      (event.xbutton.y > dy - delta) && 
	      (event.xbutton.y < dy + delta))
	    {
	      px = event.xbutton.x;
	      py = event.xbutton.y;
	      while (1)
		{
		  XQueryPointer(dpy,main_win,&root,&child,&rx,&ry,
				&wx,&wy,&mask);
		  
		  if (!(mask & Button1Mask)) break;
		  
		  cx = wx - px;
		  cy = wy - py;

		  if (dw - cx > 640) cx = 640 - dw;
		  if (dw - cx < 0) cx = dw;
		  if (dh - cy > 640) cy = 640 - dh;
		  if (dh - cy < 0) cy = dh;

		  if (cx || cy || !video)
		    XvPutVideo(dpy, port, main_win, gc, 0, 0, 
			       640, 480, dx + cx, dy + cy, dw - cx, dh - cy);

		  if (cx > 0)
		    {
		      XClearArea(dpy,main_win,dx,dy,cx,dh,False);
		    }
		  
		  if (cy > 0)
		    {
		      XClearArea(dpy,main_win,dx,dy,dw,cy,False);
		    }

		  dx += cx;
		  dy += cy;
		  dw -= cx;
		  dh -= cy;
		  px = wx;
		  py = wy;
		}
	    }
	  else if ((event.xbutton.x > dx) && 
		   (event.xbutton.y > dy) &&
		   (event.xbutton.x < dx + dw) && 
		   (event.xbutton.y < dy + dh))
	    {
	      px = event.xbutton.x;
	      py = event.xbutton.y;
	      while (1)
		{
		  XQueryPointer(dpy,main_win,&root,&child,&rx,&ry,
				&wx,&wy,&mask);

		  if (!(mask & Button1Mask)) break;
		  
		  cx = wx - px;
		  cy = wy - py;

		  if (cx || cy || !video)
		    XvPutVideo(dpy, port, main_win, gc, 0, 0, 
			       640, 480, dx + cx, dy + cy, dw, dh);

		  if (cx > 0)
		    {
		      XClearArea(dpy,main_win,dx,dy,cx,dh,False);
		    }
		  else if (cx < 0)
		    {
		      XClearArea(dpy,main_win,dw+dx+cx-1,dy,-cx+2,dh,False);
		    }
		  
		  if (cy > 0)
		    {
		      XClearArea(dpy,main_win,dx,dy,dw,cy,False);
		    }
		  else if (cy < 0)
		    {
		      XClearArea(dpy,main_win,dx,dh+dy+cy-1,dw,-cy+1,False);
		    }
		  dx = dx + cx;
		  dy = dy + cy;
		  px = wx;
		  py = wy;
		}
	    }
	  else
	    {
	      if (!video) XvPutVideo(dpy, port, main_win, gc, 0, 0, 
				     640, 480, dx, dy, dw, dh);

	    }
	}
      else if (event.type == event_base)
	{
	  pe = (XvEvent *)&event;
	  if (pe->xvvideo.drawable == main_win)
	    {
	      if (pe->xvvideo.reason == XvStarted)
		{
		  printf("  Video started on port %d\n", pe->xvvideo.port_id);
		  video = True;
		}
	      if (pe->xvvideo.reason == XvStopped)
		{
		  printf("  Video stopped on port %d\n", pe->xvvideo.port_id);
		  video = False;
		}
	      if (pe->xvvideo.reason == XvPreempted)
		{
		  printf("  Video preempted on port %d\n", pe->xvvideo.port_id);
		  video = False;
		}
	      if (pe->xvvideo.reason == XvHardError)
		{
		  printf("  Video error on port %d\n", pe->xvvideo.port_id);
		  video = False;
		}
	      if (pe->xvvideo.reason == XvBusy)
		{
		  printf("  Port %d is grabbed by another client\n", 
			 pe->xvvideo.port_id);
		  XBell(dpy, 100);
		  if (pe->xvvideo.drawable == main_win)
		    video = False;
		}
	    }
	  else
	    {
	      printf("  Wrong video notify window!!!\n");
	    }
	}
    }

  printf("\n  Xv test program #5 finished\n");

}


Setup(argc, argv, nAdaptors, pAdaptors, p_port, p_depth, p_vis_id)
     int         argc;
     char        *argv[];
     unsigned long nAdaptors;
     XvAdaptorInfo *pAdaptors;
     unsigned long *p_port;
     unsigned long *p_depth;
     unsigned long *p_vis_id;
{
  int ii, jj;
  XvAdaptorInfo *pAdaptor;
  XvFormat *pFormat;
  int adaptor, port, depth, visual_id;

  adaptor = port = depth = visual_id = -1;

  /* LOOK THROUGH COMMAND LINE ARGUMENTS */

  for ( ii = 1; ii < argc; ii++ )
    {
      if (strcmp( argv[ii], "-adaptor") == 0)
	{
	    if(++ii < argc)
	      adaptor = atoi(argv[ii]);
            else
	      UseMsg(argv);
	}
      else if (strcmp( argv[ii], "-port") == 0)
	{
	    if(++ii < argc)
	      port = atoi(argv[ii]);
            else
	      UseMsg(argv);
	}
      else if (strcmp( argv[ii], "-depth") == 0)
	{
	    if(++ii < argc)
	      depth = atoi(argv[ii]);
            else
	      UseMsg(argv);
	}
      else if (strcmp( argv[ii], "-visual") == 0)
	{
	    if(++ii < argc)
	      visual_id = atoi(argv[ii]);
            else
	      UseMsg(argv);
	}
      else if (strcmp( argv[ii], "-help") == 0)
	{
	  UseMsg(argv);
	}
    }

  if (adaptor < 0)
    {
      pAdaptor = pAdaptors;
    }
  else
    {
      if (adaptor > nAdaptors)
	{
	  printf("\n  Adaptor #%d doesn't exist.\n", adaptor);
	  return False;
	}
      pAdaptor = pAdaptors+(adaptor-1);
    }

  if (port < 0)
    {
      port = pAdaptor->base_id;
    }
  else
    {
      if (port > pAdaptor->num_ports)
	{
	  printf("\n  Port #%d doesn't exist for adaptor #%d.\n", 
		 port, adaptor);
	  return False;
	}
    }

  pFormat = pAdaptor->formats;

  if (depth < 0)
    {
      depth = pFormat->depth;
    }
  else
    {
      for (ii=0; ii<pAdaptor->num_formats; ii++)
	{
	  if (pFormat->depth == depth) break;
	  pFormat++;
	}

      if (ii >= pAdaptor->num_formats)
	{
	  printf("\n  Depth %d not supported by adaptor.\n", depth);
	  return False;
	}
    }

  if (visual_id < 0)
    {
      visual_id = pFormat->visual_id;
    }
  else
    {

      pFormat = pAdaptor->formats;

      for (ii=0; ii<pAdaptor->num_formats; ii++)
	{
	  if ((pFormat->visual_id == visual_id) && pFormat->depth == depth)
	    break;
	  pFormat++;
	}

      if (ii >= pAdaptor->num_formats)
	{
	  printf("\n  Visual-id %d at depth %d not supported by adaptor.\n", 
		 depth, visual_id);
	  return False;
	}
    }

  *p_port = port;
  *p_depth = depth;
  *p_vis_id = visual_id;

  return True;
}

UseMsg(argv)
     char        *argv[];
{

  printf("use: %s [option]\n", argv[0]);
  printf("\t-adaptor #                       adaptor number\n");
  printf("\t-port #                          port (XID)\n");
  printf("\t-depth #                         drawable depth (planes)\n");
  printf("\t-visual #                        drawable visual (id)\n");
  exit();
}
