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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: ImageCache.c,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:39:17 $"
#endif
#endif
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifdef VMS
#include <sys/ssdef.h>
#include <sys/rms.h>
#define MAXPATHLEN   NAM$C_MAXRSS
#include <descrip.h>
#endif

#include "XmI.h"
#include <Xm/XmosP.h>
#include <Xm/ScreenP.h>
#include "BitmapsI.h"

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void InitializeImageSet() ;
static void GetImageData() ;

static void ScreenDestroyCallback();
static void AddScreenDestroyCallback();

#else

static void InitializeImageSet( void ) ;
static void GetImageData( 
                        char *image_name,
                        int *hot_x,
                        int *hot_y,
                        unsigned int *width,
                        unsigned int *height) ;
static void ScreenDestroyCallback ( 
			Widget w, 
			XtPointer client_data, 
			XtPointer call_data );
static void AddScreenDestroyCallback(
			Screen *screen,
			XtPointer client_data );

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

/*  Image array handling defines, structure, and global statics  */

#ifdef DEC_MOTIF_RTOL
#define MAX_BUILTIN_IMAGES      12
#else
#define MAX_BUILTIN_IMAGES	11
#endif
#define INCREMENT_SIZE		10

typedef struct _ImageSet
{
   int       hot_x, hot_y;
   XImage  * image;
   char    * image_name;
} ImageSet;

static ImageSet * image_set = NULL;
static int image_set_size = 0;


/*  Pixmap caching structure and global statics  */

typedef struct _PixmapCache
{
   Screen * screen;
   char   * image_name;
   Pixel    foreground;
   Pixel    background;
   Pixmap   pixmap;
   int	    depth;
   int      reference_count;
   struct _PixmapCache * next;
} PixmapCache;

static PixmapCache * pixmap_cache = NULL;




/************************************************************************
 *
 *  InitializeImageSet
 *	Initialize the image set if needed.
 *
 ************************************************************************/
static void 
#ifdef _NO_PROTO
InitializeImageSet()
#else
InitializeImageSet( void )
#endif /* _NO_PROTO */
{
   register int i;

   image_set_size = MAX_BUILTIN_IMAGES + INCREMENT_SIZE;
   image_set = (ImageSet *) XtMalloc (sizeof (ImageSet) * image_set_size);

   for (i = 0; i < MAX_BUILTIN_IMAGES + INCREMENT_SIZE; i++)
   {
      image_set[i].image = NULL;
      image_set[i].hot_x = image_set[i].hot_y = 0;

      if (i < MAX_BUILTIN_IMAGES)
         image_set[i].image_name = bitmap_name_set[i];
      else
         image_set[i].image_name = NULL;
   }
}





/************************************************************************
 *
 *  _XmInstallImage
 * 
 *		Allow a hot_spot to be specified
 *
 ************************************************************************/
Boolean 
#ifdef _NO_PROTO
_XmInstallImage( image, image_name, hot_x, hot_y )
        XImage *image ;
        char *image_name ;
        int hot_x, hot_y;
#else
_XmInstallImage(
        XImage *image,
        char *image_name,
	int hot_x,
	int hot_y)
#endif /* _NO_PROTO */
{
   register int i;
   register int image_loc;


   /*  Error checking  */

   if (image == NULL || image_name == NULL) return (False);


   /*  Check for the initial allocation of the image set array  */

   if (image_set_size == 0)
      InitializeImageSet();


   /*  Verify that the image_name is not already in the image set.  */

   for (i = 0; i < image_set_size; i++)
   {
      if (image_set[i].image_name != NULL)
      {
         if (strcmp (image_name, image_set[i].image_name) == 0)
            return (False);
      }
   }


   /*  Check for an open location for the new image; there shouldn't be gaps
   **  in the set, so this will find the first empty spot following the
   **  images 
   */

   image_loc = -1;

   for (i = MAX_BUILTIN_IMAGES; i < image_set_size; i++)
   {
      if (image_set[i].image == NULL)
      {
         image_loc = i;
         break;
      }
   }


   /*  If the image loc still is -1 then more image set space  */
   /*  needs to be allocated.                                  */

   if (image_loc == -1)
   {
      image_loc = image_set_size;
      image_set_size += INCREMENT_SIZE;
      image_set = 
         (ImageSet *) XtRealloc((char *) image_set, sizeof(ImageSet) * image_set_size);

      for (i = image_set_size - INCREMENT_SIZE; i < image_set_size; i++)
      {
         image_set[i].image = NULL;
         image_set[i].image_name = NULL;
	 image_set[i].hot_x = 
	   image_set[i].hot_y = 0;
      }
   }


   /*  Initialize the image element for the new image and return True.  */

   image_set[image_loc].image = image;
   image_set[image_loc].image_name = 
      (char *) strcpy (XtMalloc (XmStrlen (image_name) + 1), image_name);
   image_set[image_loc].hot_x = hot_x;
   image_set[image_loc].hot_y = hot_y;
   return (True);
}


/************************************************************************
 *
 *  XmInstallImage
 *	Add the provided image for the image set and return an
 *	tile id to be used for further referencing.  Keep the
 *	allocation of the image_set array straight.
 *
 ************************************************************************/
Boolean 
#ifdef _NO_PROTO
XmInstallImage( image, image_name )
        XImage *image ;
        char *image_name ;
#else
XmInstallImage(
        XImage *image,
        char *image_name )
#endif /* _NO_PROTO */
{
    return _XmInstallImage(image, image_name, 0, 0);
}




/************************************************************************
 *
 *  XmUninstallImage
 *	Remove an image from the image set.
 *	Return a boolean (True) if the uninstall succeeded.  Return
 *	a boolean (False) if an error condition occurs.
 *
 ************************************************************************/
Boolean 
#ifdef _NO_PROTO
XmUninstallImage( image )
        XImage *image ;
#else
XmUninstallImage(
        XImage *image )
#endif /* _NO_PROTO */
{
   register int i;


   /*  Check for invalid conditions  */

   if (image == NULL) return (False);


   /*  Loop through the image set until the image is found.   */
   /*  Free the image, pack down the array, shrink the array  */
   /*  if needed, and return true.                            */

   /*  Start searching with the user-added images; note that image_set_size
   **  includes up to INCREMENT_SIZE allocated but unset items.
   */

   for (i = MAX_BUILTIN_IMAGES; i < image_set_size; i++)
   {
      if (image == image_set[i].image)
      {
         XtFree (image_set[i].image_name);

         /*  Pack the images down one slot  */

         for (i = i + 1; i < image_set_size; i++)
         {
            image_set[i - 1].image = image_set[i].image;
            image_set[i - 1].image_name = image_set[i].image_name;
            image_set[i - 1].hot_x = image_set[i].hot_x;
            image_set[i - 1].hot_y = image_set[i].hot_y;
	    
         }

         image_set[i].image = NULL;
         image_set[i].image_name = NULL;

         /*  Shrink the image set array if necessary  */

         for (i = image_set_size - 1; 
              i > image_set_size - INCREMENT_SIZE - 1 && 
              image_set[i].image == NULL; i--)

         if (i == image_set_size - INCREMENT_SIZE - 1)
         {
            image_set_size -= INCREMENT_SIZE;
            image_set = 
               (ImageSet *) XtRealloc ((char *) image_set, 
                                       sizeof(ImageSet) * image_set_size);
         }         

         return (True);
      }
   }

   return (False);
}


Boolean
#ifdef _NO_PROTO
_XmGetImage(screen, image_name, image)
	Screen *screen;
	char *image_name;
	XImage **image;
#else
_XmGetImage(
	Screen *screen,
	char *image_name,
	XImage **image )
#endif /* _NO_PROTO */
{
    register int i;
    register int image_loc;
    static Boolean initialized = False;
    static XImage  * built_in_image;
    register Display *display = DisplayOfScreen(screen);
    int hot_x, hot_y;

    /*  Check for the initial allocation of the image set array  */
    
    if (image_set_size == 0)
      InitializeImageSet();
    
    
    image_loc = -1;
    for (i = 0; i < image_set_size; i++)
      {
	  if (image_set[i].image_name != NULL &&
	      strcmp (image_set[i].image_name, image_name) == 0)
	    {
		image_loc = i;
		break;
	    }
      }
    
    
    /*  If no image was found, set up and go try to
	get an image from a file  */
    
    if (image_loc == -1)
      {
	  char *bmPath;
	  char *file_name;
	  SubstitutionRec	subs[1] ;
	  Boolean user_path ;
	  
	  subs[0].substitution = image_name;
	  
	  bmPath = _XmOSInitPath(image_name, "XBMLANGPATH", &user_path);
	  
          if (user_path) subs[0].match = 'B';
	  else           subs[0].match = MATCH_CHAR ;
	  
#ifdef VMS
	  if (bmPath == NULL)
	  {
	  /*
	   *  Look first in the local directory, then DECW$SYSTEM_DEFAULTS;
	   *  Use .dat as the default extension.
	   */
	  struct
	  {
	    unsigned short curlen;
	    char           body [MAXPATHLEN+1];
	  } vs;

	  unsigned long lstatus, lcontext, lflags = 0;
	  struct dsc$descriptor_s image_desc; 
	  struct dsc$descriptor_vs file_name_desc; 
	  struct dsc$descriptor_s default_file_name_desc; 

	  file_name = &vs.body;

	  image_desc.dsc$w_length  = strlen(image_name);
    	  image_desc.dsc$b_dtype   = DSC$K_DTYPE_T;     /* 8 bit chars */
     	  image_desc.dsc$b_class   = DSC$K_CLASS_S;     /* fixed length */
     	  image_desc.dsc$a_pointer = image_name;

	  file_name_desc.dsc$w_maxstrlen  = MAXPATHLEN;
    	  file_name_desc.dsc$b_dtype   = DSC$K_DTYPE_VT;    /* 8 bit chars */
     	  file_name_desc.dsc$b_class   = DSC$K_CLASS_VS;    /* Varying string */
     	  file_name_desc.dsc$a_pointer = &vs;

	  default_file_name_desc.dsc$w_length  = strlen(".DAT");
    	  default_file_name_desc.dsc$b_dtype   = DSC$K_DTYPE_T;     /* 8 bit chars */
     	  default_file_name_desc.dsc$b_class   = DSC$K_CLASS_S;     /* fixed length */
     	  default_file_name_desc.dsc$a_pointer = ".DAT";
	  
	  lcontext = 0;
	  lstatus = LIB$FIND_FILE (&image_desc, &file_name_desc,
				   &lcontext, &default_file_name_desc,
				   NULL, NULL, &lflags);
	  LIB$FIND_FILE_END (&lcontext);

	  if (!(lstatus & SS$_NORMAL)) 
	  {
	      default_file_name_desc.dsc$w_length  =
			strlen("DECW$SYSTEM_DEFAULTS:.DAT");
     	      default_file_name_desc.dsc$a_pointer = "DECW$SYSTEM_DEFAULTS:.DAT";
	      lcontext = 0;
	      lstatus = LIB$FIND_FILE (&image_desc, &file_name_desc,
				       &lcontext, &default_file_name_desc,
				       NULL, NULL, &lflags);
	      LIB$FIND_FILE_END (&lcontext);
          }
	
	  if ((lstatus & SS$_NORMAL)) 
	    {
		vs.body[vs.curlen] = '\0';
		if ((*image = (XImage *) _XmGetImageAndHotSpotFromFile (file_name, &hot_x, &hot_y)) != NULL)
		  {
		      _XmInstallImage (*image, image_name, hot_x, hot_y);
		      
		      for (image_loc = MAX_BUILTIN_IMAGES - 1; 
			   *image != image_set[image_loc].image; image_loc++);
		  }
	    }

	  } /* bmPath == NULL */
	  else
#endif	/* VMS */
	  if (file_name = XtResolvePathname(display, "bitmaps", NULL,
					    NULL, bmPath, subs, XtNumber(subs), NULL))
	    {
		if ((*image = (XImage *)
		     _XmGetImageAndHotSpotFromFile (file_name,
						    &hot_x, &hot_y))
		    != NULL)
		  {
		      _XmInstallImage (*image, image_name, hot_x, hot_y);
		      
		      for (image_loc = MAX_BUILTIN_IMAGES - 1; 
			   *image != image_set[image_loc].image; image_loc++);
		  }
		XtFree(file_name);
	    }
	  XtFree (bmPath);
      }
    
    if (image_loc == -1)
      return (FALSE);
    
        
    /*  If the image is a built in image then get it.  Otherwise  */
    /*  just use the image out of the image set array.            */
    
    if (image_loc < MAX_BUILTIN_IMAGES)
      {
	  if (!initialized)
	    {
		initialized = True;
		_XmCreateImage(built_in_image, display, NULL, 16, 16, MSBFirst);
	    }
	  
	  built_in_image->data = (char *) bitmaps [image_loc];
	  *image = built_in_image;
      }
    else
      *image = image_set[image_loc].image;
    
    return(TRUE);
}

static void 
#ifdef _NO_PROTO
GetImageData(image_name, hot_x, hot_y, width, height)
    char *image_name;
    int *hot_x, *hot_y;
    unsigned int *width, *height;
#else
GetImageData(
    char *image_name,
    int *hot_x,
    int *hot_y,
    unsigned int *width,
    unsigned int *height)
#endif /* _NO_PROTO */
{
    int i;
    for (i = 0; i < image_set_size; i++)
      {
	  if (image_set[i].image_name != NULL)
	    {
		if (strcmp (image_name, image_set[i].image_name) == 0)
		  {
		      if (i < MAX_BUILTIN_IMAGES)
			{
			    *hot_x = *hot_y = 0;
			    *width = *height = 16;
			}
		      else 
			{
			    *hot_x = image_set[i].hot_x;
			    *hot_y = image_set[i].hot_y;
			    *width = image_set[i].image->width;
			    *height = image_set[i].image->height;
			}
		      return;
		  }
	    }
      }
    *hot_x = *hot_y = 0;
    *width = *height = 0;
    return;
}

/*
 * see if this pixmap is in the cache. If it is then return all the
 * gory details about it
 */
Boolean 
#ifdef _NO_PROTO
_XmGetPixmapData(screen, pixmap, image_name, depth, foreground,
			 background, hot_x, hot_y, width, height)
    Screen *screen ;
    Pixmap pixmap;
    char  **image_name ;/* RETURN */
    int   *depth;	/* RETURN */
    Pixel *foreground ; /* RETURN */
    Pixel *background ; /* RETURN */
    int	*hot_x, *hot_y; /* RETURN */
    unsigned int *width, *height;/* RETURN */
#else
_XmGetPixmapData(
    Screen *screen,
    Pixmap pixmap,
    char **image_name,
    int *depth,
    Pixel *foreground,
    Pixel *background,
    int *hot_x,
    int *hot_y,
    unsigned int *width,
    unsigned int *height)
#endif /* _NO_PROTO */
{
   register PixmapCache * cache_ptr;

   /*  Set up a loop that goes through the cache list and  */
   /*  checks for a matching screen and pixmap.            */

   for (cache_ptr = pixmap_cache; cache_ptr; cache_ptr = cache_ptr->next)
   {
      if (cache_ptr->pixmap == pixmap && cache_ptr->screen == screen)
      {
	  *foreground = cache_ptr->foreground;
	  *background = cache_ptr->background;
	  *depth = cache_ptr->depth;
	  *image_name = cache_ptr->image_name;
	  GetImageData(*image_name, hot_x, hot_y, width, height);
	  return (True);
      }
  }
   return (False);
}


/*
 * create a pixmap from the image_name.  foreground and background
 * must be valid values. For depth 1 they should be 1 and 0
 * respectively. 
 */
Pixmap 
#ifdef _NO_PROTO
_XmGetPixmap(screen, image_name, depth, foreground, background)
    Screen *screen ;
    char *image_name ;
    int depth;
    Pixel foreground ;
    Pixel background ;
#else
_XmGetPixmap(
    Screen *screen,
    char *image_name,
    int depth,
    Pixel foreground,
    Pixel background)
#endif /* _NO_PROTO */
{    
    register PixmapCache * cache_ptr;
    register Display * display = DisplayOfScreen(screen);
    XImage * image;
    Pixmap   pixmap;
    XmScreen xmScreen;
    XGCValues gcValues;

    /*  Error checking  */
    
    if (image_name == NULL)
		return (XmUNSPECIFIED_PIXMAP);
    
    
    /*  Check for a matching pixmap  */
    
    for (cache_ptr = pixmap_cache; cache_ptr; cache_ptr = cache_ptr->next)
      {
	  if (strcmp (cache_ptr->image_name, image_name) == 0  &&
	      cache_ptr->foreground == foreground              &&
	      cache_ptr->background == background              &&
	      cache_ptr->depth == depth		 	       &&
	      cache_ptr->screen == screen)
	    {
		cache_ptr->reference_count++;
		return (cache_ptr->pixmap);
	    }
      }

    if (!_XmGetImage(screen, image_name, &image) ||
	(image->depth != depth && image->depth != 1))
      return (XmUNSPECIFIED_PIXMAP);

    /*
     * Create a pixmap to hold the image, allocate a new pixmap
     * cache entry, put the cache entry at the head of the queue,
     * initialize the cache fields and increment the image reference.
     */
    
    pixmap = XCreatePixmap (display, RootWindowOfScreen(screen), 
			    image->width, image->height,
			    depth);
    
    cache_ptr = XtNew (PixmapCache);
    AddScreenDestroyCallback(screen, (XtPointer) NULL);
    cache_ptr->next = pixmap_cache;
    pixmap_cache = cache_ptr;
    
    cache_ptr->screen = screen;
    cache_ptr->foreground = foreground;
    cache_ptr->background = background;
    cache_ptr->depth = depth;
    cache_ptr->image_name = 
      (char *) strcpy (XtMalloc (XmStrlen (image_name) + 1), image_name);
    cache_ptr->reference_count = 1;
    cache_ptr->pixmap = pixmap;
    
    
    /*  Set up a gc for the image to pixmap copy, store the image  */ 
    /*  into the pixmap and return the pixmap.                     */
    
    xmScreen = (XmScreen) XmGetXmScreen(screen);

    if (depth != xmScreen->screen.imageGCDepth || !xmScreen->screen.imageGC)
      {
	  if (xmScreen->screen.imageGC != NULL)
		XFreeGC (display, xmScreen->screen.imageGC);
	  gcValues.foreground = foreground;
	  gcValues.background = background;
	  xmScreen->screen.imageGC = XCreateGC (display, pixmap,
			  GCForeground | GCBackground, &gcValues);
	  xmScreen->screen.imageGCDepth = depth;
	  xmScreen->screen.imageForeground = foreground;
	  xmScreen->screen.imageBackground = background;
      }
    else if ((xmScreen->screen.imageForeground != foreground) ||
	     (xmScreen->screen.imageBackground != background))
      {
	  gcValues.foreground = foreground;
	  gcValues.background = background;
	  XChangeGC (display, xmScreen->screen.imageGC,
		     GCForeground | GCBackground, &gcValues);
	  xmScreen->screen.imageForeground = foreground;
	  xmScreen->screen.imageBackground = background;
      }
    
    XPutImage (display, pixmap, xmScreen->screen.imageGC, image, 
	       0, 0, 0, 0, image->width, image->height);
    return (pixmap);
}

/************************************************************************
*
*  XmGetPixmapByDepth
*	Public wrapper around _XmGetPixmap with parameter order changed.
*
************************************************************************/
Pixmap 
#ifdef _NO_PROTO
XmGetPixmapByDepth(screen, image_name, foreground, background, depth)
    Screen *screen ;
    char *image_name ;
    Pixel foreground ;
    Pixel background ;
    int depth;
#else
XmGetPixmapByDepth(
    Screen *screen,
    char *image_name,
    Pixel foreground,
    Pixel background,
    int depth)
#endif /* _NO_PROTO */
{    
    return(_XmGetPixmap(screen, image_name, depth,
		foreground, background));
}


/************************************************************************
*
*  XmGetPixmap
*	Create a pixmap of screen depth, using the image referenced
*	by the name and the foreground and background colors 
*       specified.  Ensure that multiple pixmaps of the same attributes
*	are not created by maintaining a cache of the pixmaps.
*
************************************************************************/
Pixmap 
#ifdef _NO_PROTO
XmGetPixmap( screen, image_name, foreground, background )
        Screen *screen ;
        char *image_name ;
        Pixel foreground ;
        Pixel background ;
#else
XmGetPixmap(
        Screen *screen,
        char *image_name,
        Pixel foreground,
        Pixel background )
#endif /* _NO_PROTO */
{
    return (_XmGetPixmap(screen, image_name, DefaultDepthOfScreen(screen),
			 foreground, background));
}




/************************************************************************
 *
 *  _XmInstallPixmap
 *	Install a pixmap into the pixmap cache.  This is used to add
 *	cached pixmaps which have no image associated with them.
 *
 ************************************************************************/
Boolean 
#ifdef _NO_PROTO
_XmInstallPixmap( pixmap, screen, image_name, foreground, background )
        Pixmap pixmap ;
        Screen *screen ;
        char *image_name ;
        Pixel foreground ;
        Pixel background ;
#else
_XmInstallPixmap(
        Pixmap pixmap,
        Screen *screen,
        char *image_name,
        Pixel foreground,
        Pixel background )
#endif /* _NO_PROTO */
{
   register PixmapCache * cache_ptr;

   Window root;
   int x,y;
   unsigned int width, height, border_width, depth;

   /*  Error checking  */

   if (image_name == NULL) return (False);

   /* expensive; in future permit depth to be passed in */
   depth = DefaultDepthOfScreen(screen);
   if (pixmap && (pixmap != XmUNSPECIFIED_PIXMAP))
	XGetGeometry(screen->display, pixmap, &root, &x, &y,
		&width, &height, &border_width, &depth);

   /*  See if this data has already been cached. */
 
   for (cache_ptr = pixmap_cache; cache_ptr; cache_ptr = cache_ptr->next)
   {
      if ((strcmp(cache_ptr->image_name, image_name) == 0) &&
		(cache_ptr->screen == screen) &&
	        (cache_ptr->depth == depth) &&
		(cache_ptr->foreground == foreground) &&
		(cache_ptr->background == background))
         return (False);
   }


   /*  Allocate the cache structure and put it into the list  */

   cache_ptr = XtNew (PixmapCache);
   AddScreenDestroyCallback(screen, (XtPointer) NULL);
   cache_ptr->next = pixmap_cache;
   pixmap_cache = cache_ptr;

   cache_ptr->screen = screen;
   cache_ptr->foreground = foreground;
   cache_ptr->background = background;
   cache_ptr->depth = depth;
   cache_ptr->image_name = 
      (char *) strcpy(XtMalloc(XmStrlen(image_name) + 1), image_name);
   cache_ptr->reference_count = 1;
   cache_ptr->pixmap = pixmap;

   return (True);
}



/************************************************************************
 *
 *  XmDestroyPixmap
 *	Locate a pixmap in the cache and decrement its reference count.
 *	When the reference count is at zero, free the pixmap.
 *
 ************************************************************************/
Boolean 
#ifdef _NO_PROTO
XmDestroyPixmap( screen, pixmap )
        Screen *screen ;
        Pixmap pixmap ;
#else
XmDestroyPixmap(
        Screen *screen,
        Pixmap pixmap )
#endif /* _NO_PROTO */
{
   register PixmapCache * cache_ptr;
   register PixmapCache * prev_cache_ptr = NULL;


   /*  Set up a loop that goes through the cache list and  */
   /*  checks for a matching screen and pixmap.            */

   for (cache_ptr = pixmap_cache; cache_ptr; cache_ptr = cache_ptr->next)
   {
      if (cache_ptr->pixmap == pixmap && cache_ptr->screen == screen)
      {
         cache_ptr->reference_count--;

         if (cache_ptr->reference_count == 0)
         {
            if (cache_ptr == pixmap_cache)
               pixmap_cache = cache_ptr->next;
            else
               prev_cache_ptr->next = cache_ptr->next;

            XtFree (cache_ptr->image_name);
            XFreePixmap (DisplayOfScreen(screen), cache_ptr->pixmap);
            XtFree ((char *) cache_ptr);
         }

         return (True);
      }

      prev_cache_ptr = cache_ptr;
   }

   return (False);
}

static void 
ScreenDestroyCallback
#ifdef _NO_PROTO
	( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;	/* not used */
        XtPointer call_data ;
#else
	( Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   /*  Set up a loop that goes through the cache list and  */
   /*  checks for a matching screen                        */
     register PixmapCache *cache_ptr;
     register PixmapCache *prev_cache_ptr = NULL;
     register PixmapCache *next_cache_ptr = NULL;
 
     for (cache_ptr = pixmap_cache; cache_ptr; cache_ptr = next_cache_ptr)
     {
 	if (cache_ptr->screen == XtScreen(w))
 	{
 	    if (cache_ptr == pixmap_cache)
 		pixmap_cache = cache_ptr->next;
 	    else
 		prev_cache_ptr->next = cache_ptr->next;
 	    next_cache_ptr = cache_ptr->next;
 
 	    XtFree (cache_ptr->image_name);
 	    XFreePixmap (XtDisplay(w), cache_ptr->pixmap);
 	    XtFree ((char*)cache_ptr);
 	}
 	else
 	{
 	    prev_cache_ptr = cache_ptr;
 	    next_cache_ptr = cache_ptr->next;
 	}
     }
}

static void
AddScreenDestroyCallback
#ifdef _NO_PROTO
	(screen, client_data)
	Screen *screen;
	XtPointer client_data;
#else
	(Screen *screen, 
	XtPointer client_data)
#endif /* _NO_PROTO */
{
   XmScreen scrn = (XmScreen) XmGetXmScreen(screen);
   /* could add and destroy each pixmap separately or just add the callback
   ** once and then search through the cache a single time
   */
   if ((XmScreen)NULL != scrn)
	{
	if (!((XmScreenInfo *)(scrn->screen.screenInfo))->destroyCallbackAdded)
	  {
	  XtAddCallback((Widget)scrn, XtNdestroyCallback,
		ScreenDestroyCallback, (XtPointer) client_data);
	  ((XmScreenInfo *)(scrn->screen.screenInfo))->destroyCallbackAdded = 
		True;
	  }
	}
}


#ifdef DEBUG

static void 
#ifdef _NO_PROTO
DumpPixmapCache()
#else
DumpPixmapCache(void)
#endif /* _NO_PROTO */
{
   register PixmapCache * cache_ptr;
   for (cache_ptr = pixmap_cache; cache_ptr; cache_ptr = cache_ptr->next)
   {
	printf 
("screen: 0x%x, image_name: %s, pixmap 0x%x,\n\tfg 0x%x, bg 0x%x, depth: %d, ref_count: %d\n", 
	cache_ptr->screen, cache_ptr->image_name,
	cache_ptr->pixmap,
	cache_ptr->foreground, cache_ptr->background, cache_ptr->depth,
	cache_ptr->reference_count);
   }
   printf ("\n");
}

static void 
#ifdef _NO_PROTO
DumpImageCache()
#else
DumpImageCache(void)
#endif /* _NO_PROTO */
{
    int i;
    for (i = 0; i < image_set_size; i++)
	{
	printf ("%2i image_name: %s,\thotspot (%d,%d),\timage 0x%x\n",
		i,
		image_set[i].image_name,
		image_set[i].hot_x, image_set[i].hot_y,
		image_set[i].image);
	}
	printf ("\n");
}

#endif /* DEBUG */
