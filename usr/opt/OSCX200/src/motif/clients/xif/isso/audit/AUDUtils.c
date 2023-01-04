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
static char *rcsid = "@(#)$RCSfile: AUDUtils.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/12/20 21:31:51 $";
#endif

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/dir.h>

#include <Mrm/MrmAppl.h>
#include <Xm/Text.h>
#include <X11/X.h>

#include "XIsso.h"
#include "Utilities.h"
#include "AUD.h"


Boolean
AUDIsAliasEvent( event )
char	*event;
{
    int	i;

    i = 0;
    while(i < audglobal.aliasEventCount &&
	  strcmp(event,audglobal.aliasEventList[i]))
	i++;

    if( i == audglobal.aliasEventCount )
	return( False );
    else
	return( True );
}




int
AUDGetFileList( dirname, list, numEntries, lookedUp, changedList )
char	*dirname;
char 	***list;
int  	*numEntries;
int	 lookedUp, changedList;
{
    DIR			*dir;
    struct dirent	*direntry;
    struct stat		stats;
    char		currentDir[PATH_MAX];

    /* if we have previously looked up this list and it has not changed
     * then just return.
     */
    if( !lookedUp || changedList )
    {
	/* get the current directory */
	getwd( currentDir );

	/* if we are re-scanning, then free up existing list */
	if( lookedUp )
	{
	    if( *list )
	    {
		while ((*numEntries)--)
		{			
		    if( (*list)[*numEntries] )
		    {
			XtFree((*list)[*numEntries]);
			(*list)[*numEntries] = (char *) NULL;
		    }
		}
		XtFree((char *)*list);
	    }
	}

	*numEntries 	= 0;
	*list 		= (char **) NULL;
	
	/* scan the directory */
	if((dir = opendir(dirname)) != (DIR *) 0)
	{
	    /* cd to the directory */
	    chdir( dirname );
	    rewinddir(dir);
	    while(((direntry = readdir(dir)) != (struct dirent *) NULL))
	    {
		if((stat(direntry->d_name, &stats) != -1) &&
		   (S_ISREG(stats.st_mode)))
		{
		    (*numEntries)++;

		    *list = (char **) XtRealloc((char *)*list,
						sizeof(char *) * *numEntries );
		    if( *list )
		    {
			(*list)[(*numEntries)-1] = (char *) 
				XtMalloc(direntry->d_namlen + 1);
			strcpy((*list)[(*numEntries)-1], direntry->d_name);
		    }
		    else
			MemoryError();
		}
	    }
	    closedir(dir);
	    chdir( currentDir );
	}
    }

    return( AUDSuccess );

} /* end AUDGetFileList() */




int
AUDGetBaseEventList(list, numEntries, lookedUp, changedList)
char 	***list;
int  	*numEntries;
int	 lookedUp, changedList;
{
    FILE	*fp;
    char	item[AUD_MAXEVENT_LEN];
    char	item2[AUD_MAXEVENT_LEN];

    /* if we have previously looked up this list and it has not changed
     * then just return.
     */
    if( !lookedUp || changedList )
    {
	/* if we are re-scanning, then free up existing list */
	if( lookedUp )
	{
	    if( *list )
	    {
		while ((*numEntries)--)
		{			
		    if( (*list)[*numEntries] )
		    {
			XtFree((char *)(*list)[*numEntries]);
			(*list)[*numEntries] = (char *) NULL;
		    }
		}
		XtFree((char *)*list);
	    }
	}

	*numEntries 	= 0;
	*list 		= (char **) NULL;
	
	/* open the base events file */
	if((fp = fopen(AUDIT_EVENT_LIST, "r")) != (FILE *) 0)
	{
	    /* we will ignore "succeed", "fail".
	     * we will consider "old" to be a prefix to an event name.
	     */
	    while( fscanf(fp, "%s", item) != EOF )
	    {
		if(strcmp(item, AUDIT_SUCCESS) &&
		   strcmp(item, AUDIT_FAILURE))
		{
		    /* got something real */
		    if( strcmp(item, AUDIT_PREFIX) == 0 )
		    {				    
			fscanf(fp, "%s", item2);
			strcat(item, " ");
			strcat(item, item2);
		    }
		    
		    (*numEntries)++;

		    *list = (char **) XtRealloc((char *)*list,
						sizeof(char *) * *numEntries );
		    if( *list )
		    {
			(*list)[(*numEntries)-1] = (char *) 
				XtMalloc(strlen(item) + 1);
			strcpy((*list)[(*numEntries)-1], item);
		    }
		    else
			MemoryError();
		}
	    }
	    fclose( fp );
	}
    }

    return( AUDSuccess );

} /* end AUDGetBaseEventList() */




int
AUDGetSiteEventList(list, numEntries, wantSubEvents, lookedUp, changedList)
char 	***list;
int  	*numEntries;
int	wantSubEvents, lookedUp, changedList;
{
    FILE	*fp;
    Boolean	new;
    char	del[2];
    char	item[AUD_MAXEVENT_LEN], base[AUD_MAXEVENT_LEN],
                event[AUD_MAXEVENT_LEN];
    int		num;


    /* if we have previously looked up this list and it has not changed
     * then just return.
     */
    if( !lookedUp || changedList )
    {
	/* if we are re-scanning, then free up existing list */
	if( lookedUp )
	{
	    if( *list )
	    {
		while ((*numEntries)--)
		{			
		    if( (*list)[*numEntries] )
		    {
			XtFree((char *)(*list)[*numEntries]);
			(*list)[*numEntries] = (char *) NULL;
		    }
		}
		XtFree((char *)*list);
	    }
	}

	*numEntries 	= 0;
	*list 		= (char **) NULL;
	
	/* open the site events file */
	if((fp = fopen(AUDIT_SITE_EVENTS, "r")) != (FILE *) 0)
	{
	    new = True;
	    while( fscanf(fp, "%s%u%1s", item, &num, del) != EOF )
	    {
		if(wantSubEvents)
		{
		    if( !new )
		    {
			strcpy(event, base);
			strcat(event, ".");
			strcat(event, item);
		    }
		    else
		    {
			strcpy(event, item);
			strcpy(base, item);
		    }
		}
		else if( new )
		{
		    strcpy(event, item);
		}
		else
		{
		    new = (del[0] == ';');
		    continue;
		}

		new = (del[0] == ';');

		(*numEntries)++;

		*list = (char **) XtRealloc((char *) *list,
					    sizeof(char *) * *numEntries );
		if( *list )
		{
		    (*list)[(*numEntries)-1] = (char *) 
			    XtMalloc(strlen(event) + 1);
		    strcpy((*list)[(*numEntries)-1], event);
		}
		else
		    MemoryError();
	    }
	    fclose( fp );
	}
    }

    return( AUDSuccess );

} /* end AUDGetSiteEventList() */




int
AUDGetAliasEventList(list, numEntries, lookedUp, changedList)
char 	***list;
int  	*numEntries;
int	 lookedUp, changedList;
{
    int		index;
    char	alias[AUD_MAXEVENT_LEN];

    /* if we have previously looked up this list and it has not changed
     * then just return.
     */
    if( !lookedUp || changedList )
    {
	/* if we are re-scanning, then free up existing list */
	if( lookedUp )
	{
	    if( *list )
	    {
		while ((*numEntries)--)
		{			
		    if( (*list)[*numEntries] )
		    {
			XtFree((char *) (*list)[*numEntries]);
			(*list)[*numEntries] = (char *) NULL;
		    }
		}
		XtFree((char *)*list);
	    }
	}

	*numEntries 	= 0;
	*list 		= (char **) NULL;
	
	/* use libaud to retrieve the alias events */
	index = 0;
	while( aud_aliasent( index, alias, sizeof alias ) != -1 )
	{
	    (*numEntries)++;

	    *list = (char **) XtRealloc((char *)*list,
					sizeof(char *) * *numEntries );
	    if( *list )
	    {
		(*list)[(*numEntries)-1] = (char *) 
			XtMalloc(strlen(alias) + 1);
		strcpy((*list)[(*numEntries)-1], alias);
	    }
	    else
		MemoryError();

	    index++;
	}
    }

    return( AUDSuccess );

} /* end AUDGetAliasEventList() */



char *
StripWhiteSpace( string )
char *string;

{
    int begin, end, newSize;
    char *trimmed;

    /* make sure no leading or trailing white space */

    begin = 0;
    end   = strlen( string ) - 1;

    if( end < 0 )
    {
        trimmed = (char *) XtMalloc( 1 );
        trimmed[0] = '\0';

        XtFree( string );
        return( trimmed );
    }

    /* leading */
    while( (begin <= end) && isascii(string[begin]) && isspace(string[begin]) )
    {
        begin++;
    }

    if( begin > end )
    {
        trimmed = (char *) XtMalloc( 1 );
        trimmed[0] = '\0';

        XtFree( string );
        return( trimmed );
    }

    /* trailing */
    while( (end >= begin) && isascii(string[end]) && isspace(string[end]) )
    {
        end--;
    }

    newSize = (end - begin) + 1;
    trimmed = (char *) XtMalloc( newSize + 1 );

    strncpy( trimmed, &string[begin], newSize );
    trimmed[newSize] = '\0';

    XtFree( string );
    return( trimmed );

} /* end StripWhiteSpace() */


Boolean
AUDIsAuditLog(fileName)
char *fileName;
{
    char *lastPart;
    Boolean status = False;

    /* Audit Logs are recognized by their file name conventions:
     *
     * *.nnn	where nnn is any number
     * *.nnn.Z	where nnn is any number (compressed format)
     */

    if(fileName)
    {
	char *local = (char *) strdup(fileName);

	if(local != (char *) NULL)
	{
	    if((lastPart = strrchr(local, '.')) != (char *) NULL)
	    {
		if((strlen(lastPart) == 2) &&
		   (strcmp(lastPart, ".Z") == 0))
		{
		    *lastPart = '\0';
		    lastPart = strrchr(local, '.');
		}

		if((lastPart != (char *) NULL) &&
		   (strlen(lastPart) == 4) &&
		   isdigit((int) *(lastPart+1)) &&
		   isdigit((int) *(lastPart+2)) &&
		   isdigit((int) *(lastPart+3)))
			status = True;
	    }
	    free(local);
	}
    }
    
    return(status);
}
