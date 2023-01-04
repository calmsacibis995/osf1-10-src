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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/session/session/readfile.c,v 1.1.2.2 92/07/13 09:05:50 Leela_Obilichetti Exp $";
#endif		/* BuildSystemHeader */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <ctype.h>
#include "smdata.h"
#include "merge.h"

#define MAXHOSTNAME 255

static struct symbols {
	char *name;
	char *value;
	struct symbols *next;
} *Head;

static int lookup_value(name)
char *name;
{
	register struct symbols *ptr;

	for(ptr = Head; ptr; ptr = ptr->next)
	  if(strcmp(ptr->name, name) == 0) 
	    return((int)ptr->value);
	return(NULL);
}

static void remove_name(name)
char *name;
{
	register struct symbols *ptr,*lastptr;
	for(ptr = Head,lastptr = NULL; ptr; lastptr = ptr,ptr = ptr->next)
	  if(strcmp(ptr->name, name) == 0) {
		  if(lastptr == NULL) {
			  Head = ptr->next;
		  } else {
			  lastptr->next = ptr->next;
		  }
		  XtFree(ptr->name);
		  XtFree(ptr->value);
		  XtFree((char *)ptr);
		  return;
	  }
}

static void add_name(name, value)
char *name;
char *value;
{
	static int next_one = 0;
	register struct symbols *ptr,*lastptr = NULL;
	char buf[80];

	if(value == NULL) {
		value = buf;
		sprintf(buf,"%d",next_one++);
	}
	for(ptr = Head; ptr; lastptr = ptr,ptr = ptr->next)
	  if(strcmp(ptr->name, name) == 0) {
		  fprintf(stderr, "Redefining %s from %s to %s\n",
			  ptr->name,ptr->value, value);
		  XtFree(ptr->value);
		  strcpy(ptr->value = XtMalloc(strlen(value)+1),value);
		  return;
	  }
	ptr = (struct symbols *)XtMalloc(sizeof(*ptr));
	if(lastptr != NULL) 
	  lastptr->next = ptr;
	else
	  Head = ptr;
	ptr->next = NULL;
	strcpy(ptr->name = XtMalloc(strlen(name)+1), name);
	strcpy(ptr->value = XtMalloc(strlen(value)+1), value);
}

static void free_list() {
	register struct symbols *ptr, *next;

	for(ptr = Head; ptr;ptr = next) {
		next = ptr->next;
		XtFree(ptr->name);
		XtFree(ptr->value);
		XtFree((char *)ptr);
	}
	Head = NULL;
}

static int Resolution(pixels, mm)
    int pixels, mm;
{
    return ((pixels * 100000 / mm) + 50) / 100;
}
#define AddSimpleDef(x,y)	(add_name(y, NULL))
#define AddDef(x,y,z)		(add_name(y, z))
static void AddNum(foo,name, value)
int foo;
char * name;
int value;
{
	char buf[80];
	sprintf(buf,"%d", value);
	add_name(name, buf);
}


static init_symbols()
{
	char hostname[MAXHOSTNAME];
	XWindowAttributes   attrs;
	unsigned    int	status;
	Visual *visual;
	Screen *screen;
	int defs = 0;

	screen = DefaultScreenOfDisplay(display_id);
	visual = DefaultVisualOfScreen(screen);

	AddNum(defs, "WIDTH", screen->width);
	AddNum(defs, "HEIGHT", screen->height);
	AddNum(defs, "X_RESOLUTION", Resolution(screen->width,screen->mwidth));
	AddNum(defs, "Y_RESOLUTION", Resolution(screen->height,screen->mheight));
	AddNum(defs, "PLANES", visual->bits_per_rgb);
	switch(visual->class) {
	      case StaticGray:
		AddDef(defs, "CLASS", "StaticGray");
		break;
	      case GrayScale:
		AddDef(defs, "CLASS", "GrayScale");
		break;
	      case StaticColor:
		AddDef(defs, "CLASS", "StaticColor");
		AddSimpleDef(defs, "COLOR");
		break;
	      case PseudoColor:
		AddDef(defs, "CLASS", "PsuedoColor");
		AddSimpleDef(defs, "COLOR");
		break;
	      case TrueColor:
		AddDef(defs, "CLASS", "TrueColor");
		AddSimpleDef(defs, "COLOR");
		break;
	      case DirectColor:
		AddDef(defs, "CLASS", "DirectColor");
		AddSimpleDef(defs, "COLOR");
		break;
	}
	gethostname(hostname, MAXHOSTNAME);
	AddDef(defs, "HOST", hostname);
	AddSimpleDef(defs,hostname);
}

getline(pline, pcount, fp) 
char **pline;
int *pcount;
FILE *fp;
{
	int ch, index = 0;
	register int last_was_quote = FALSE;
	register char *line = *pline;

	while(!feof(fp)){
		switch(ch = fgetc(fp)) {
		      case EOF:
			break;
		      case '\\':
			last_was_quote = TRUE;
			if (index >= *pcount) {
				line = *pline = 
				  XtRealloc(*pline, *pcount += 256);
			}
			line[index++] = ch;
			break;
		      case '\n':
			if (index >= *pcount) {
				line = *pline = 
				  XtRealloc(*pline, *pcount += 256);
			}
			if(!last_was_quote) {
				line[index] = 0;
				return(feof(fp));
			} else {
				line[index++] = ch;
			}
			last_was_quote = FALSE;
			break;
		      default:
			if (index >= *pcount) {
				line = *pline = 
				  XtRealloc(*pline, *pcount += 256);
			}
			line[index++] = ch;
			last_was_quote = FALSE;
			break;
		}
	}
	line[index] = 0;
	return(feof(fp));
}
readfile(filename, process_line, state)
char *filename;
void (*process_line)();
int state;
{
	FILE *fp;
	static visable = TRUE;
	static int visable_depth = 0;
	static int depth = 0;
	char value[256], name[256], lineb[256], buf[30];
	char *line = XtMalloc(256);
	int line_size = 256;
	int line_no = 0;
	int start_depth;
	if((fp = fopen(filename,"r")) == NULL) {
		if((state&FIRST_DONE)) {
			perror(filename);
		}		
		XtFree(line);
		return(0);
	}
	start_depth = depth;
	if((state&FIRST_DONE) == 0)
	  init_symbols();
	while(!feof(fp)) {
		if(getline(&line, &line_size, fp)) 
		  continue;
	      line_no++;
		if(line[0] == '#') {
			if(strncmp(line, "#ifdef", 6) == 0) {
				depth ++;
				if(visable) {
					if(sscanf(line, 
						  "#ifdef %s"
						  ,name) != 1) {
						fprintf(stderr,
							"%d:?%s?\n",line_no,
							line);
						(*process_line)(line, FALSE,
								state);
						continue;
					}
					visable = (lookup_value(name) != NULL);
					if(visable) 
					  visable_depth = depth;
				}
				(*process_line)(line, FALSE, state);
				continue;
			}
			if(strncmp(line, "#ifndef", 7) == 0) {
				depth ++;
				if(visable) {
					if(sscanf(line, 
						  "#ifndef %s"
						  ,name) != 1) {
						fprintf(stderr,
							"%d:?%s?\n",line_no,
							line);
						(*process_line)(line, FALSE,
								state);
						continue;
					}
					visable = (lookup_value(name) == NULL);
					if(visable) 
					  visable_depth = depth;
				}
				(*process_line)(line, FALSE, state);
				continue;
			}
			if(strncmp(line, "#endif", 6) == 0 ) {
				depth--;
				if(depth < start_depth) {
					fprintf(stderr,
						"%d:?%s?\n",line_no, line);
					(*process_line)(line, FALSE, state);
					depth = start_depth;
					continue;
				}
				if(visable_depth >= depth) {
					visable = TRUE;
					visable_depth = depth;
				}
				(*process_line)(line, FALSE, state);
				continue;
			}
			if(strncmp(line, "#else", 5) == 0) {
				if(depth == 0) {
					fprintf(stderr,
						"%d:?%s?\n",line_no, line);
					(*process_line)(line, FALSE, state);
					continue;
				}
				if(visable) {
					visable = FALSE;
					visable_depth = depth-1;
				} else {
					if(visable_depth == depth-1) {
						visable = !visable;
						visable_depth = depth;
					}
				}
				(*process_line)(line, FALSE, state);
				continue;
			}
		        if(strncmp(line, "#define", 6)== 0) {
				switch(sscanf(line,
				 "#define %s %s",
				       name,value)) {
				      default:
				      case 0:
					fprintf(stderr,
						"%d:?%s?\n", line_no, line);
					break;
				      case 1:
					add_name(name, NULL);
					break;
				      case 2:
					add_name(name, value);
					break;
				}
				(*process_line)(line, FALSE, state);
				continue;
			}
			if(strncmp(line,"#undef", 6) == 0) {
				if(1 != sscanf(line,
				     "#undef[	 ][ 	]*%s",name)){
					fprintf(stderr,
						"%d:?%s?\n",line_no, line);
					(*process_line)(line, FALSE, state);
					continue;
				}
				remove_name(name);
			}
			if(strncmp(line, "#if", 3) == 0) {
				fprintf(stderr,"#if Not supported at %d\n",
					line_no);
				depth ++;
				(*process_line)(line, FALSE, state);
				continue;
			}
			if(strncmp(line, "#include", 8) == 0) {
				FILE *nfp;
				if(1 != sscanf(line,
				     "#include[	 ][ 	]*\"%s\"",name)){
					fprintf(stderr,
						"%d:?%s?\n",line_no, line);
					(*process_line)(line, FALSE, state);
					continue;
				}
				(void)readfile(name, process_line, state|FIRST_DONE);
				(*process_line)(line, FALSE, state);
				continue;
			}
			(*process_line)(line, FALSE, state);
			continue;
		} else 
		  (*process_line)(line,visable, state);
	}
	if((state & FIRST_DONE) == 0) 
	  free_list();
	XtFree(line);
	return(1);
}

		   
