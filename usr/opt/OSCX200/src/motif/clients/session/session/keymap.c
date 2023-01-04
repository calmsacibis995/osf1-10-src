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
#include	<X11/Xlib.h>
#include	<X11/Xutil.h>
#include	<X11/keysym.h>

#ifndef _BSD
#  define _BSD
#  include	<sys/dir.h>
#  undef _BSD
#else
#  include	<sys/dir.h>
#endif

#include	<stdio.h>
#include	<ctype.h>

#define KEYBOARD_DEFAULT_STRING "System Default"

#define KeymapDirectory "/usr/lib/X11/keymaps/"
#define KeymapFileType ".decw_keymap"

#define KeymapFileTypeLength 12

#define FileNameLength 256
#define MaxLineLength 160

#define AllocIncrement 1200

static int server_keymap_changed = False;   /* has the server map been     */
					    /* over-ridden in this module  */

static KeySym *server_keymap_data = NULL;   /* temp. storage for server-   */
static int server_keymap_width = 0;	    /* -default keymap		   */


/*
**++
**
**  ROUTINE:
**
**	DXListKeyboardMaps
**
**  CALLING SEQUENCE:
**
**	list = DXListKeyboardMaps(nmaps)
**	    char **list = return address of list
**	    int *nmaps = return number of keymaps in list
**          
**  IMPLICIT INPUTS:
**
**      The logical name DECW$KEYMAP is assumed to point to a directory
**	containing keyboard mapping files.
**
**  IMPLICIT OUTPUTS:
**
**      NONE
**
**  FUNCTION VALUE OR COMPLETION CODES:
**
**	NULL if no keymaps found,
**	else a list of keymap names
**
**  SIDE EFFECTS:
**
**      Allocates memory for the list of names. Use DXFreeKeyboardMapList
**	to deallocate this memory.
**
**--
**/

char **DXListKeyboardMaps (nmaps)
    int *nmaps;
{
    char **list = NULL;		/* list of char pointers to keymap names */
    char *data = NULL;		/* block of strings of keymap names */
    char *newspace;
    int reserved_space = 0;	/* amount of space reserved for list */
    DIR *dirp;
    struct direct *file;
    char filespec[FileNameLength]; /* wildcard filespec for keymaps */
    char *result;               /* returned keymap name */
    char work[FileNameLength];	/* work area for get_file_name() */
    int nbytes = 0;
    int nfiles = 0;
    int length;
    int i;
    int pos;
    char *server_default_text = KEYBOARD_DEFAULT_STRING;
/*
 * insert 'Server default keymap' string as first entry in the list.
 */

    nfiles = 1;
    length = strlen (server_default_text) + 1;
    data = (char *)XtMalloc (AllocIncrement);
    reserved_space = AllocIncrement;
    data[0] = (char) length;
    bcopy (server_default_text, data+1, length);
    nbytes = length + 1;

/* build wildcard filespec */

    if((dirp = opendir(KeymapDirectory)) != NULL) {

    strcpy (filespec, ".*\\");
    strcat (filespec, KeymapFileType);

    re_comp(filespec);

    while ((file = readdir(dirp)) != NULL)
        {
        if(!re_exec(file->d_name))
                  continue;
        nfiles++;
        result = file->d_name;
        if(index(result, '.')){
          length = ((char *)index(result, '.') - result) +1;
          *(char *)index(result,'.') = 0;
        } else
          length=strlen(result)+1;
        if (nbytes+length+1 >= reserved_space) /* allocate another block */
                {
                data=(char *)XtRealloc(data, reserved_space + AllocIncrement);
                reserved_space += AllocIncrement;
                }
        data[nbytes]=(char)length;
        bcopy (result, data+nbytes+1, length);
        for(i = 0 ; i < length; i++)
          if(data[nbytes+1+i] == '_')
             data[nbytes+1+i] = ' ';
        nbytes+= length+1;
        }
    closedir(dirp);
    }

    pos=0;
    list = (char **) XtMalloc(nfiles * sizeof (char *));
			/* A.R.  ultrix has sizeof (char **)  */
    for (i=0; i < nfiles; i++)
	{
	list[i] = data+pos+1;
	pos+= ((unsigned char)data[pos])+1;
	}
    *nmaps=nfiles;
    return (list);
}

/*
**++
**
**  ROUTINE:
**
**      DXFreeKeyboardMapList - Free the memory allocated by DXListKeyboardMaps
**
**  CALLING SEQUENCE:
**
**      DXFreeKeyboardMapList(list)
**          char **list = address of list to be freed
**
**  IMPLICIT INPUTS:
**
**      NONE
**
**  IMPLICIT OUTPUTS:
**
**      NONE
**
**  FUNCTION VALUE OR COMPLETION CODES:
**
**	Void
**
**  SIDE EFFECTS:
**
**      NONE
**
**--
**/

void DXFreeKeyboardMapList (list)
    char **list;
{
    if (list !=NULL)
	{
	XtFree(list[0]-1);
	XtFree(list);
	}
}

/*
**++
**
**  ROUTINE:
**
**	DXSetKeyboardLockMode - Set lock key in server table to CapsLock
**		or ShiftLock
**
**  CALLING SEQUENCE:
**
**	status = DXSetKeyboardLockMode(dpy,mode)
**	    int status = result of call
**	    Display *dpy = display connection
**	    KeySym mode = lock mode (XK_Caps_Lock, XK_Shift_Lock)
**          
**  IMPLICIT INPUTS:
**
**      NONE
**
**  IMPLICIT OUTPUTS:
**
**      The server's keycode to keysym mapping table is changed so
**	the lock keycode will map to new lock mode.
**
**  FUNCTION VALUE OR COMPLETION CODES:
**
**	True : lock mode was changed
**	False : illegal lock mode
**
**  SIDE EFFECTS:
**
**      NONE
**
**--
**/

int DXSetKeyboardLockMode (dpy, mode)
    Display *dpy;
    KeySym mode;
{
    KeySym *k;
    KeyCode kc;
    int min_keycode, max_keycode ;
    int keysyms_per_keycode ;
    XModifierKeymap *modifiermap = NULL;

    /* get temporary modifiermap */
    modifiermap = XGetModifierMapping(dpy) ;

    /* ensure keysym and modifier maps are loaded */
    if (modifiermap==NULL) XKeycodeToKeysym(dpy,0,0);

    /* get keycode of lock modifier key */
    kc=modifiermap->modifiermap[(LockMapIndex * modifiermap->max_keypermod)];

    /* get min and max keycodes. */
    XDisplayKeycodes(dpy, &min_keycode, &max_keycode) ;
    
    if (kc<min_keycode || kc > max_keycode)
	return (False);

    XChangeKeyboardMapping (dpy, kc, 1, &mode, 1); 
    XInsertModifiermapEntry(modifiermap, kc, 1);
    XSetModifierMapping (dpy, modifiermap);
    XFreeModifiermap(modifiermap) ;
    return (True);
}


/*
**++
**
**  ROUTINE:
**
**	DXGetKeyboardLockMode - Return the lock mode set for a display 
**	connection
**
**  CALLING SEQUENCE:
**
**	mode = DXGetKeyboardLockMode(dpy)
**	    Display *dpy = display connection
**          
**  IMPLICIT INPUTS:
**
**      NONE
**
**  IMPLICIT OUTPUTS:
**
**      NONE
**
**  FUNCTION VALUE OR COMPLETION CODES:
**
**	Lock mode = (NoSymbol, XK_Caps_Lock, XK_Shift_Lock)
**
**  SIDE EFFECTS:
**
**      NONE
**
**--
*/

KeySym DXGetKeyboardLockMode (dpy)
    Display *dpy;
{

  XModifierKeymap *modifiermap ;    /* modifierkeymap for dxkeycaps*/

  /* get temporary modifiermap */
  modifiermap = XGetModifierMapping(dpy) ;

  /* ensure keysym and modifier maps are loaded */

  if (modifiermap==NULL) XKeycodeToKeysym(dpy,0,0);

  /* get the keysym associated with the Lock modifier key */
  
  if(modifiermap->max_keypermod > 0)
    {
      KeySym tmp ;
      tmp = XKeycodeToKeysym(dpy,
	modifiermap->modifiermap[(LockMapIndex * modifiermap->max_keypermod)],
	0) ;
      XFreeModifiermap(modifiermap) ;
      return (tmp) ;
      
      
    }
  else
    {
      XFreeModifiermap(modifiermap) ;
      return(NoSymbol);
    }
}

/*
**++
**
**  ROUTINE:
**
**	DXLoadKeyboardMap - Load keymap from file, and copy to server
**
**  CALLING SEQUENCE:
**
**	status = DXLoadKeyboardMap(dpy,id)
**	    Display *dpy = display connection
**	    char *id = name of keymap to be loaded
**          
**  IMPLICIT INPUTS:
**
**      The logical name DECW$KEYMAP is assumed to point to a directory
**	containing keyboard mapping files.
**
**  IMPLICIT OUTPUTS:
**
**      NONE
**
**  FUNCTION VALUE OR COMPLETION CODES:
**
**	True = keymap was loaded into server
**	False = keymap file could not be loaded
**
**  SIDE EFFECTS:
**
**      XMappingNotify events are generated for each client connected 
**	to this server.
**
**--
*/

#define EndLine(c) (((c)=='!' || (c) =='\n' || (c) == '\0') ? True : False )


int DXLoadKeyboardMap (dpy, keymap_name)
    Display *dpy;
    char *keymap_name;
{
    int idlen;
    int kblen;
    int ks_per_kc;
    char keymap_file[FileNameLength];
    FILE *fp;
    KeySym *keymap = NULL;
    char line[MaxLineLength];
    KeyCode min_kc;
    KeyCode max_kc;
    int i;
    int col;
    int nsyms;
    int max_keycode, min_keycode;

    /* check if we are loading to a DEC server */

    if (strncmp (ServerVendor(dpy), "DECWINDOWS", 10)!=0)
      return (False);

    /* get min and max keycodes */
    XDisplayKeycodes(dpy, &min_keycode, &max_keycode) ;
    
    /* preserve server default keymap, in case user wants to restore it */

    if (strcmp (keymap_name, KEYBOARD_DEFAULT_STRING) == 0)
      {
	/* load server default */
        if (server_keymap_changed == False)
	  {
	    return (True);	/* no change, map is already present */
	  }
	else
	  {
	    /* send over the saved server default map */
	    XChangeKeyboardMapping (dpy, min_keycode, server_keymap_width,
				    server_keymap_data,
				    max_keycode-min_keycode+1);
	    server_keymap_changed = False;

	    /*
	     * may wish to free this each time, to save memory (performance hit)
	     *
	     server_keymap_width = 0;
	     XFree (server_keymap_data);   ** Not XtFree.. this is Xlib data **
	     server_keymap_data = NULL;
	     *
	     */

	    return (True);
	  }
      }
    else    /* loading a non-default keymap file */
      {
	if (server_keymap_changed == False)
	  {	/*save server map ? */
	    if (server_keymap_data == NULL) /* may already be saved */
	      server_keymap_data = XGetKeyboardMapping (dpy,min_keycode,
						      max_keycode-min_keycode+1,
						      &server_keymap_width);
	    server_keymap_changed = True;
	  }
	else {
	  /* server map is already saved, continue loading file */
	}
      }

    /* build full filespec. */

    strcpy (keymap_file, KeymapDirectory);
    strcat (keymap_file, keymap_name);
    strcat (keymap_file, KeymapFileType);

    /* replace ' ' with '_', just in case it came from DXListKeyboardMaps */

    kblen = strlen (KeymapDirectory);
    idlen = strlen (keymap_name);
    for (i=kblen; i<kblen+idlen; i++)
      {
	if (keymap_file[i]==' ')
	  keymap_file[i]='_';
      }

    min_kc = (KeyCode) min_keycode;
    max_kc = (KeyCode) max_keycode;

    /* figure out how big the key map is and create a buffer for it */
    if ((fp=fopen (keymap_file, "r")) == NULL)
      return ( NULL);

    /* figure out how wide the keymap should be */
    ks_per_kc = ScanKeyMap(fp,keymap,min_kc,max_kc);
    
    nsyms = (max_kc-min_kc+1) * ks_per_kc;
    keymap = (KeySym *) XtMalloc (nsyms * sizeof (KeySym));
    for (i=0; i<nsyms; i++)
      keymap[i] = NoSymbol;
    fclose (fp);

    if ((fp=fopen (keymap_file, "r")) == NULL)
      return ( NULL);

    while (fgets (line, MaxLineLength, fp) != NULL)
      {
	if (AddLineToKeymap (line, keymap, min_kc, max_kc, ks_per_kc) ==False)
	  {
	    fclose (fp);
	    if (keymap != NULL)
	      XtFree (keymap);
	    return ( NULL);
	  }
      }

    fclose (fp);

    /* copy keysym map to server */
    XChangeKeyboardMapping (dpy, min_kc, ks_per_kc, keymap, (max_kc-min_kc+1));
    XtFree(keymap);
    return (True);
}

/* could also send back the min/max keycodes found in the file */
static int ScanKeyMap(fp,keymap,minkc,maxkc)
	FILE *fp;
    KeySym **keymap;
    KeyCode minkc;
    KeyCode maxkc;
{
    char line[MaxLineLength];
    int pos;
    KeyCode kc;
    KeySym *offset;
    KeySym ks;
    int ncols,max_cols;
    int col;
    int i;

	max_cols = 0;
    while (fgets (line, MaxLineLength, fp) != NULL)
	{
		ncols=0;
		if (isspace(line[0]) || line[0] == '!' ||
			line[0] == '\n' || line[0] == '\0')
			continue;	 /* ignore blank lines and comments */

		pos=0;
		if ((kc=GetToken (line, &pos)) == -1)
			return (False);
		while (GetToken (line, &pos) != -1)
		    ncols++;
		if(max_cols < ncols)
			max_cols = ncols;
    }

	if(max_cols < 2)
		max_cols = 2;
    return(max_cols);
}

static int AddLineToKeymap (line, keymap, minkc, maxkc, ncols)
    char *line;
    KeySym *keymap;
    KeyCode minkc;
    KeyCode maxkc;
    int ncols;
{
    int pos;
    KeyCode kc;
    KeySym *offset;
    KeySym ks;
    int col;
    int i;

	/* there better be a keymap already */
    if (keymap ==NULL)
		return(False);

    if (isspace(line[0]) || line[0] == '!' ||
		line[0] == '\n' || line[0] == '\0')
		return (True); /* ignore blank lines and comments */

    pos = 0;
    if ((kc=GetToken (line, &pos)) == -1)
		return (False);
    if ( kc < minkc || kc > maxkc )
		 return (False);
    offset = keymap + (kc-minkc) * (ncols);
    col=0;
    while((col < ncols) && ((ks=GetToken (line, &pos)) != -1))
	{
		*(offset + col) = ks;
		col++;
    }
    return (True);
}

/*
 * return hex value of next item on line (current position held in 'pos')
 */

int GetToken (line, pos)
    char *line;
    int *pos;
{
    int start;

    if (EndLine(line[*pos]) == True) return (-1);
    while (isspace(line[*pos]) || EndLine(line[*pos])) {
	if (EndLine(line[*pos]) == True)
		return (-1);
	(*pos)++;
    }
    start = *pos;
    while (!isspace (line[*pos]) && !EndLine (line[*pos])) {
	(*pos)++;
    }
    return (StringToHex(&line[start], (*pos)-start));
}


/*
 * convert null terminated hexadecimal string to integer
 *
 * return 'value', or '-1' on error
 *
 */

int StringToHex (str,nbytes)
    char *str;
    int nbytes;
{
    int i;
    int digit;
    int scale = 1;
    long value = 0;

    for (i=nbytes-1;i>=0;i--) {
	if (!isxdigit(str[i])) return (-1);
	if (isdigit(str[i]))
		digit=str[i]-'0';
	else
		digit=toupper(str[i])-'A'+10;
	value+=(digit*scale);
	scale*=16;
    }
    return(value);
}


/* A.R.  DxSetComposeMode was in old versions */
/* A.R.  PassSymbol was in old versions */
