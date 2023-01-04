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
**   xvt2.c --- Xv test program 2
**   
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   05.15.91 Carver
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


main(argc, argv)
     int         argc;
     char        *argv[];
{
  int mask;
  int scr_no;
  char chr;
  int ii, jj, kk, ll, status, grabbed;
  int xx,yy;

  Display *dpy;
  unsigned long screen;
  Visual *vis, *def_vis;
  XVisualInfo *p_vis_info, vis_info_tmpl;
  unsigned long vis_id;
  unsigned long depth;
  XEvent event;
  Colormap cmap, def_cmap;
  XColor scolor,ecolor;
  XSetWindowAttributes win_attr;
  Window still_wins[3][4];
  Window root,main_win,video_win;
  Pixmap p1,p2;
  GC gc;
  XGCValues gc_vals;

  unsigned int evb, erb, mop;
  unsigned int version, revision;
  XvPortID port;
  unsigned int nAdaptors;
  XvAdaptorInfo *pAdaptors;
  XvEvent *pe;
  XvEncodingID enc_id;
  Bool video = False;

  printf("\n  Welcome to Xv test program #2\n\n");
  printf("  This program is just a little something I whipped up.  It\n");
  printf("  plays live video into the top main window and allows the\n");
  printf("  user to use the mouse to capture a template of scaled stills\n");

  printf("\n> Press return to continue...");
  chr = getc(stdin);

  dpy = XOpenDisplay(0);
  if (!dpy)
    {
      printf("\n  Couldn't open display\n");
      printf("\n  Xv test program #2 terminated\n");
      exit();
    }

  screen = XDefaultScreen(dpy);
  def_vis = XDefaultVisual(dpy, screen);
  def_cmap = XDefaultColormap(dpy, screen);
  root = XDefaultRootWindow(dpy);

  status = XvQueryExtension(dpy, &version, &revision, &mop, &evb, &erb);

  if (status != Success)
    {
      printf("\n  The Xv Extension is not installed.\n");
      printf("\n  Xv test program #2 terminated.\n");
      exit();
    }

  printf("\n  Xv V%01d.%d\n", version, revision);

  XvQueryAdaptors(dpy, root, &nAdaptors, &pAdaptors);

  if (!nAdaptors)
    {
      printf("\n  Your display has no video adaptors.\n");
      printf("\n  Xv test program #2 terminated.\n");
      exit();
    }

  if (!Setup(argc, argv, nAdaptors, pAdaptors, 
	    &port, &depth, &vis_id))
    {
      printf("\n  Setup failed.\n");
      printf("\n  Xv test program #3 terminated\n");
      exit();
    }
  
  vis_info_tmpl.visualid = vis_id;
  p_vis_info = XGetVisualInfo(dpy, VisualIDMask, &vis_info_tmpl, &ii);
  if (!p_vis_info)
    {
      printf("Error: Couldn't find visual to use.\n");
      return;
    }
  vis = p_vis_info->visual;
  
  cmap = XCreateColormap(dpy, root, vis, AllocNone);
  
  XAllocNamedColor(dpy, cmap, "midnight blue", &scolor, &ecolor);

  win_attr.background_pixel = scolor.pixel;
  win_attr.event_mask = ExposureMask | ButtonPressMask | 
    KeyPressMask | VisibilityChangeMask;
  win_attr.colormap = cmap;
  win_attr.border_pixel = scolor.pixel;

  main_win = XCreateWindow(dpy, root, 0, 0, 720, 980, 0, 
			   depth, InputOutput, vis, 
			   CWBackPixel | CWEventMask | 
			   CWBorderPixel | CWColormap, &win_attr);

  XAllocNamedColor(dpy, cmap, "black", &scolor, &ecolor);

  win_attr.background_pixel = scolor.pixel;

  video_win = XCreateWindow(dpy, main_win, 40, 4, 640, 480, 0, 
			    depth, InputOutput, vis, 
			    CWBackPixel | CWEventMask | CWColormap, &win_attr);
  XMapWindow(dpy, video_win);

  yy = 488;
  for (ii=0; ii<3; ii++)
    {
      xx = 16;
      for (jj=0; jj<4; jj++)
	{
	  still_wins[ii][jj] = XCreateWindow(dpy, main_win, xx, yy, 160, 160, 
					     0, depth, 
					     InputOutput, vis, 
					     CWBackPixel | CWEventMask | 
					     CWColormap, 
					     &win_attr);
	  XMapWindow(dpy, still_wins[ii][jj]);
	  xx += 160 + 16;
	}
      yy += 160 + 4;
    }

  XMapWindow(dpy, main_win);

  printf("\n  Waiting for main window to become visible...\n");
  while (1)
    {
      XNextEvent(dpy, &event);
      if (event.type == VisibilityNotify) break;
    }

  printf("\n  Selecting notification on video window.\n");

  XvSelectVideoNotify(dpy, video_win, True);

  printf("\n");
  printf("  Try pressing other mouse buttons in video window.\n");
  printf("  Hit any key to toggle between grab and ungrab port\n");
  printf("  Hit ^c to exit\n");
  printf("\n");

  gc_vals.foreground = scolor.pixel;
  gc = XCreateGC(dpy, video_win, GCForeground, &gc_vals);

  ii = 0;
  jj = 0;
  grabbed = 0;

  XvPutVideo(dpy, port, video_win, gc, 0, 0, 
	     640, 480, 0, 0, 640, 480);

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
	  if (!grabbed) 
	    {
	      status = XvGrabPort(dpy, port, event.xkey.time);
	      if (status == Success)
		{
		  printf("  Port Grabbed!\n");
		  grabbed = 1;
		}
	      else if (status == XvAlreadyGrabbed)
		printf("  Port Already Grabbed!\n");
	      else if (status == XvInvalidTime)
		printf("  Invalid Grab Time!\n");
	    }
	  else  
	    {
	      XvUngrabPort(dpy, port, event.xkey.time);
	      grabbed = 0;
	      printf("  Port Ungrabbed!\n");
	    }
	}
      else if (event.type == ButtonPress)
	{
	  if (event.xbutton.button == Button3)
	    {
	      if (jj==0)
		{
		  jj=3;
		  if (ii==0) ii=2;
		  else ii--;
		}
	      else
		jj--;
	    }
	  if (event.xbutton.button == Button2)
	    {
	      if (jj==3)
		{
		  jj=0;
		  if (ii==2) ii=0;
		  else ii++;
		}
	      else
		jj++;
	    }
	  if ((event.xbutton.window == video_win) ||
	      (event.xbutton.button != Button1))
	    {
	      if (!video)
		{
		  XvPutVideo(dpy, port, video_win, gc, 0, 0, 
			     640, 480, 0, 0, 640, 480);
		  video = True;
		}
	      XvPutStill(dpy, port, still_wins[ii][jj], gc, 0, 0, 
			 640, 480, 0, 0, 160, 160);
	    }
	  else
	    {
	      for (kk=0; kk<3; kk++)
		{
		  for (ll=0; ll<4; ll++)
		    {
		      if (still_wins[kk][ll] == event.xbutton.window)
			{
			  ii = kk;
			  jj = ll;
			  XvPutStill(dpy, port, event.xbutton.window, 
				     gc, 0, 0, 640, 480, 0, 0, 160, 160);
			  break;
			}
		    }
		  if (ll < 4) break;
		}
	    }
	}
      else if (event.type == evb)
	{
	  pe = (XvEvent *)&event;
	  if (pe->xvvideo.drawable == video_win)
	    {
	      if (pe->xvvideo.reason == XvStarted)
		{
		  printf("  Video started on port %d\n", pe->xvvideo.port_id);
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
		  if (pe->xvvideo.drawable == video_win)
		    video = False;
		}
	    }
	  else
	    {
	      printf("  Wrong video notify window!!!\n");
	    }
	}
    }

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
