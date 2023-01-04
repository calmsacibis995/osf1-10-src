#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: [util.c,v 1.5 91/07/30 19:49:11 rmurphy Exp ]$";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* #module UTIL "V1-002" */
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      Notepad -- DECwindows simple out-of-the-box editor.
**
**  AUTHOR:
**
**      Joel Gringorten  - November, 1987
**
**  ABSTRACT:
**
**      This module contains miscellaneous routines.  
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**
**
**      V1-001  JMG0001         J. M. Gringorten            23-Oct-1987
**              Initial version.
**
**     [bl] 23-Nov-93   (Refer ootb_bugs 213)
**
**     connectSources(): Store the length of data before destroying
**                       the old source
**--
*/

#include "notepad.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/TextP.h>
#include <Xm/Protocols.h>		/* for XmAddWmProtocolCallback */
#include <Xm/AtomMgr.h>			/* for XmInternAtom */
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#ifndef F_SETSYN
#define F_SETSYN 10
#endif

#ifdef VMS
#include <lckdef.h>
#include <psldef.h>
#endif /* VMS */

XeditPrintf() { /* anachronism */ }

XmTextSource PseudoDiskSourceCreate(filename)
  char *filename;
{
#define chunk 2048
    static Widget dummyrText = (Widget) NULL;
    XmTextSource Source;
    XmTextBlockRec text;
    XmTextPosition pos;
    FILE *file;
    char *buf;
    int amount;
    Boolean editable;
    int max_length;

    if (dummyrText == (Widget) NULL)
    {
	dummyrText = XmCreateText(toplevel, "dummyrText", NULL, 0);
    }
    if(!strlen(filename))
    {
	Source = _XmStringSourceCreate ("", False);
	XmTextSetSource(dummyrText, Source, 0, 0);
	return(Source);
    }
    file = fopen(filename, "r");
    if(!file)
    {
	Source = _XmStringSourceCreate ("", False);
	XmTextSetSource(dummyrText, Source, 0, 0);
	return(Source);
    }
    Source = _XmStringSourceCreate ("", False);
    XmTextSetSource(dummyrText, Source, 0, 0);
    pos = 0;
    text.format = FMT8BIT;
    buf = XtMalloc(chunk);
    text.ptr = buf;
    editable = _XmStringSourceGetEditable(Source);
    max_length = _XmStringSourceGetMaxLength(Source);
    _XmStringSourceSetEditable(Source, True);
    _XmStringSourceSetMaxLength(Source, MAXINT);
    while(( amount = fread(buf, 1, chunk, file)) > 0)
    {
	text.length = amount;
	(*Source->Replace)(dummyrText, NULL, &pos, &pos, &text, True);
	pos += amount; 
    }
    _XmStringSourceSetEditable(Source, editable);
    _XmStringSourceSetMaxLength(Source, max_length);
    fclose(file);
    XtFree(buf);
    return(Source);
}

void PseudoDiskSourceDestroy(Source)
XmTextSource Source;
{
    _XmStringSourceDestroy(Source); 
}

XFontStruct *globalFontStruct;
XFontStruct *setFontStruct;
char *fontName = "";
char *setFontName;
XmFontList revertFontList;


typedef struct NamePart {
    short unsigned offset;
    short unsigned size;
} NamePart;

typedef struct _tNode {
    struct _tNode *next;
    char *fontName;
    NamePart name;
    struct _tNode *kids;
} tNode;

tNode *nameList;
tNode *SetNameNode;
tNode *SetSizeNode;

void EmptyListBox(widget)
  Widget widget;
{
    int i, pos = 1;
    Arg a[2];
    a[0].name = XmNitemCount;
    a[0].value = 0;
    XtGetValues(widget, a, 1);
    for(i=0; i < a[0].value; i++){
	XmListDeletePos(widget, 1);
    }
}

Boolean namesEqual(font1, part1, font2, part2)
  char *font1, *font2;
  NamePart *part1, *part2;
{
    if(part1->size != part2->size)
	return FALSE;
    if(strncmp(&font1[part1->offset], &font2[part2->offset], part1->size) == 0)
	return TRUE;
    else
	return FALSE;
}

Boolean parseFontname(parts, font)
  char *font;
  NamePart *parts;
{
  int i, pi, j;
    j = pi = 0;
    parts[pi].offset = 0;
    while(font[j])
    {
	if(pi == 15)
	    return FALSE;
	if(font[j] == '-')
	{
	    parts[pi].size = j - parts[pi].offset;
	    pi++;
	    parts[pi].offset = j + 1;
	}
	j++;
    }
    if(pi != 14) 
	return FALSE;
    parts[pi].size = j - parts[pi].offset;
    return TRUE;
}

tNode *putNode(list, font, name)
  tNode **list;
  char *font;
  NamePart *name;
{
    tNode *np, *result;
    for(np = *list; np; np = np->next)
    {
	if(namesEqual(font, name, np->fontName, &np->name))
	    break;
    }
    if(np)
    {
	result = np;
    }
    else
    {
	tNode *new = (tNode *)XtMalloc(sizeof(tNode));
	if(*list)
	{
	    for(np = *list; np->next; np = np->next);
	    np->next = new;
	}
	else
	{
	    *list = new;
	}
	new->next = NULL;
	new->name = *name;
	new->kids = NULL;
	new->fontName = font;
	result = new;
    }
    return result;
}

printNames(list)
  tNode *list;
{
    char buf[1024];
    tNode *np, *sp, *mp, *cp;
    for(np = list; np; np = np->next) {
	memcpy(buf, &np->fontName[np->name.offset], np->name.size);
	buf[np->name.size] = 0;
	fprintf(stderr, "%s\n", buf);
	for(sp=np->kids; sp; sp = sp->next){
	    memcpy(buf, &sp->fontName[sp->name.offset], sp->name.size);
	    buf[sp->name.size] = 0;
	    fprintf(stderr, "\t%s\n", buf);
	    for(mp=sp->kids; mp; mp = mp->next){
		memcpy(buf, &mp->fontName[mp->name.offset], mp->name.size);
		buf[mp->name.size] = 0;
	        fprintf(stderr, "\t\t%s\n", buf);
		for(cp=mp->kids; cp; cp = cp->next){
		    memcpy(buf, &cp->fontName[cp->name.offset],
					 cp->name.size);
		    buf[cp->name.size] = 0;
		    fprintf(stderr, "\t\t\t%s\n", buf);
		}
	    }
	}
    }
}

#define N_REGISTRY 0
#define N_FOUNDRY 1
#define N_FAMILY 2
#define N_WEIGHT 3
#define N_SLANT 4
#define N_SETWIDTH 5
#define N_STYLE 6
#define N_PIXELSIZE 7
#define N_POINTSIZE 8
#define N_RESX 9
#define N_RESY 10
#define N_SPACING 11
#define N_AVERAGEWIDTH  12
#define N_CHARSTSETREG  13
#define N_CHARSETENCODING 14

FillTree() {
    int num, i,j,m, pi;
    tNode *np;
    NamePart parts[15], namePart, sizePart, miscPart, charsetPart;
    char **fonts = XListFonts(CurDpy, "*", 10000, &num);
    for(i=0; i<num; i++) {
	if(parseFontname(parts, fonts[i])) {
	    namePart.offset = parts[N_FOUNDRY].offset;
	    namePart.size = parts[N_FOUNDRY].size + parts[N_FAMILY].size + 1;
	    np = putNode(&nameList, fonts[i], &namePart);
	    sizePart.offset = parts[N_PIXELSIZE].offset;
	    sizePart.size = parts[N_PIXELSIZE].size + parts[N_POINTSIZE].size + 
		    parts[N_RESX].size +  parts[N_RESY].size + 3;
	    np = putNode(&np->kids, fonts[i], &sizePart);
	    miscPart.offset = parts[N_WEIGHT].offset;
	    miscPart.size = parts[N_WEIGHT].size + parts[N_SLANT].size + 
		    parts[N_SETWIDTH].size + 2;
	    np = putNode(&np->kids, fonts[i], &miscPart);
	    charsetPart.offset = parts[N_CHARSTSETREG].offset;
	    charsetPart.size = parts[N_CHARSTSETREG].size +
			     parts[N_CHARSETENCODING].size + 1;
	    np = putNode(&np->kids, fonts[i], &charsetPart);
        }
    }
}

void DoCustomizeFont()
{
    NamePart parts[15];    
    static int GotFonts = FALSE;
    int i;
    char buf[1024];
    tNode *np;
    if(!fontDialog.popupWidget)
        makeFontDialog(workAreaPane, &fontDialog);
    if(!GotFonts){
	Arg a[1];
	BeginLongOperation();
	a[0].name = XmNwidth;
	a[0].value = 0;
	XtGetValues(Stuff.FontMiscList, a, 1);
	XtSetValues(fontDialog.stringWidget, a, 1);
	FillTree();
        for(np = nameList; np; np = np->next) {
	    buf[0] = 0;
	    parseFontname(parts, np->fontName);
	    strncat(buf, &np->fontName[parts[N_FAMILY].offset], 
				parts[N_FAMILY].size);
	    strncat(buf, " (", 2);
	    strncat(buf, &np->fontName[parts[N_FOUNDRY].offset], 
				parts[N_FOUNDRY].size);
	    strncat(buf, ")", 1);
	    XmListAddItem(Stuff.FontFamilyList, 
		XmStringLtoRCreate(buf , "ISO8859-1"), 0);
	}
	EndLongOperation();
	GotFonts = TRUE;
	
    }
    if(XtIsManaged(fontDialog.popupWidget))
	XtUnmanageChild(fontDialog.popupWidget);
    XtManageChild(fontDialog.popupWidget);
}

void DoSetFontFamily (widget, tag, reason)
  Widget widget;
  int *tag;
  XmListCallbackStruct *reason;
{
  int i;
    NamePart parts[15];    
    char buf[1024];
    tNode *np;
    EmptyListBox(Stuff.FontSizeList);
    EmptyListBox(Stuff.FontMiscList);
    np = nameList;
    for(i=1; i < reason->item_position; i++)
	np = np->next;
    SetNameNode = np;
    for(np=np->kids; np; np = np->next){
	buf[0] = 0;
	parseFontname(parts, np->fontName);
	strncat(buf, &np->fontName[parts[N_PIXELSIZE].offset], 
			    parts[N_PIXELSIZE].size);
	strncat(buf, " (", 2);
	strncat(buf, &np->fontName[parts[N_RESX].offset], 
			    parts[N_RESX].size);
	strncat(buf, ")", 1);
	XmListAddItem(Stuff.FontSizeList, 
		XmStringLtoRCreate(buf , "ISO8859-1"), 0);
    }
}

char *parseSpacing(str)
char *str;
{
    if(!strcmp(str, "m" ) || !strcmp(str, "M" ))
	return "Monospaced";
    if(!strcmp(str, "p" ) || !strcmp(str, "P" ))
	return "Proportional";
    if(!strcmp(str, "c" ) || !strcmp(str, "C" ))
	return "Character";
    return str;
}

char *parseSlant(str)
char *str;
{
    if(!strcmp(str, "r" ) || !strcmp(str, "R" ))
	return "Roman";
    if(!strcmp(str, "i" ) || !strcmp(str, "I" ))
	return "Italic";
    if(!strcmp(str, "o" ) || !strcmp(str, "O" ))
	return "Oblique";
    if(!strcmp(str, "ri" ) || !strcmp(str, "RI" ))
	return "Rev Italic";
    if(!strcmp(str, "ro" ) || !strcmp(str, "RO" ))
	return "Rev Oblique";
    return str;
}

void DoSetFontSize (widget, tag, reason)
  Widget widget;
  int *tag;
  XmListCallbackStruct *reason;
{
    int i;
    NamePart parts[15];    
    tNode *np, *npp;
    char buf[1024], wtName[32], slName[32], spName[32], swName[32], stName[32],
	 crName[32], ceName[32];
    EmptyListBox(Stuff.FontMiscList);
    np = SetNameNode->kids;
    for(i=1; i < reason->item_position; i++)
	np = np->next;
    SetSizeNode = np;
    for(np=np->kids; np; np = np->next){
	for(npp = np->kids; npp; npp = npp->next){
	    parseFontname(parts, npp->fontName);

	    memcpy(wtName, &npp->fontName[parts[N_WEIGHT].offset], 
				parts[N_WEIGHT].size);
	    wtName[parts[N_WEIGHT].size] = 0;

	    memcpy(slName, &npp->fontName[parts[N_SLANT].offset], 
				parts[N_SLANT].size);
	    slName[parts[N_SLANT].size] = 0;


	    memcpy(swName, &npp->fontName[parts[N_SETWIDTH].offset], 
				parts[N_SETWIDTH].size);
	    swName[parts[N_SETWIDTH].size] = 0;

	    memcpy(stName, &npp->fontName[parts[N_STYLE].offset], 
				parts[N_STYLE].size);
	    stName[parts[N_STYLE].size] = 0;

	    memcpy(spName, &npp->fontName[parts[N_SPACING].offset], 
				parts[N_SPACING].size);
	    spName[parts[N_SPACING].size] = 0;

	    memcpy(crName, &npp->fontName[parts[N_CHARSTSETREG].offset], 
				parts[N_CHARSTSETREG].size);
	    crName[parts[N_CHARSTSETREG].size] = 0;

	    memcpy(ceName, &npp->fontName[parts[N_CHARSETENCODING].offset], 
				parts[N_CHARSETENCODING].size);
	    ceName[parts[N_CHARSETENCODING].size] = 0;
	    sprintf
	    (
		buf,
		"%-8s%-12s%-13s%-13s%-13s%s-%s",
		wtName,
		parseSlant(slName),
		swName,
		stName,
		parseSpacing(spName),
		crName,
		ceName
	    );
	    XmListAddItem(Stuff.FontMiscList, 
		XmStringLtoRCreate(buf , "ISO8859-1"), 0);
	}
    }
}

void DoSetFontMisc (widget, tag, reason)
  Widget widget;
  int *tag;
  XmListCallbackStruct *reason;
{
  int i = 0;
    tNode *np, *npp;
    static XFontStruct *cur_font; 
    np = SetSizeNode->kids;
    while(np && i < reason->item_position) {
	npp = np->kids;
	while(npp) {
	    i++;
	    if(i == reason->item_position) {
		break;
	    }
	    npp = npp->next;
        }
        np = np->next;
    }

    if( cur_font != NULL )
        XUnloadFont( CurDpy, cur_font->fid );
    cur_font = setFontStruct = XLoadQueryFont( CurDpy, npp->fontName );
    setaValue (fontDialog.stringWidget, XmNfontList,
	XmFontListCreate(cur_font, "ISO8859-1"));
    setFontName = npp->fontName;
}

void DoRevertFont()
{
    XtUnmanageChild(fontDialog.popupWidget);
}

void DoApplyFont(){
    View *vp;
    if(setFontStruct){
	fontName = setFontName;
	globalFontStruct = setFontStruct;
	for(vp = Stuff.viewHead; vp; vp = vp->flink){
	    setaValue (vp->widget, XmNfontList,
		XmFontListCreate(globalFontStruct, "ISO8859-1"));
	}
    }
}

void modifiedHandler()
{
    char wmName[1024];
    PSsetApplicationCallback(Psource, NULL);
    if(loadedfile)
        sprintf(wmName, "%s: %s (%s)", notepadString, loadedfile,
		modifiedString);
    else
	sprintf(wmName, "%s:%s (%s)", notepadString, 
		untitledString, modifiedString);
    setaValue(toplevel, XtNtitle, wmName);
    modified = TRUE;
}

clearModified(name)
  char *name;
{
  char *iName, *parseFilename();
  char wmName[1024];
    if(read_only)
        sprintf(wmName, "%s: %s (%s)", notepadString, name, 
				readonlyString);
    else
        sprintf(wmName, "%s: %s", notepadString,  name);
    setaValue(toplevel, XtNtitle, wmName);
    iName = parseFilename(name, wmName);
    if(!iName)
	iName = name;
    setaValue(toplevel, XtNiconName, iName);
	
    modified = FALSE;
}

void ShowMessage(widget, index)
  Widget *widget;
  char *index;
{
    if(!*widget)
        MrmFetchWidget(DRM_hierarchy, index, mainWin, widget, &dummy_class);
    XtManageChild(*widget);
}

static XmTextSource connectSources(filename)
  char *filename;
{
  XmTextSource oldsource;
  XmTextSource csource;		/* Current source */
  View *vp;
  static Widget dummyaText = (Widget) NULL;
  static Widget dummycText = (Widget) NULL;

    if (dummyaText == (Widget) NULL) {
	dummyaText = XmCreateText(toplevel, "dummyaText", NULL, 0);
	dummycText = XmCreateText(toplevel, "dummycText", NULL, 0);
    }
    if(!read_only && !journalFile)/* Can't create jnl file in this directory */
    {
	ShowMessage(&Stuff._noJournalAccessMessage, "noJournalAccessMessage");
	Feep();
	read_only = TRUE;
    }
    oldsource = Psource;	/* Save pointer to previous source */
    asource = _XmStringSourceCreate ("", False);
    rsource = PseudoDiskSourceCreate(filename); /* Rsource = read source ?*/
    csource = _XmStringSourceCreate ("", False);/* Current source? */
    Psource = PSourceCreate
	(rsource, asource, csource, journalFile, changesUntilCompress); 

    for(vp = Stuff.viewHead; vp; vp = vp->flink)
    {
	XmTextWidget tw = (XmTextWidget) vp->widget;
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
 	tw->text.output->data->cursor_on = False;
#else
	tw->text.output->data->dest_visible = False;
#endif
	tw->text.output->data->has_rect = False;
	tw->text.on_or_off = off;
	XmTextSetSource(vp->widget,Psource,0,0);
	PSCountTotalLines(Psource, 0, MAXINT);
    }
    XmTextSetSource(dummyaText, asource, 0, 0);
    XmTextSetSource(dummycText, csource, 0, 0);
    PSsetApplicationCallback(Psource, modifiedHandler);
    if(oldsource)
    { 
      Psource->data->length = oldsource->data->length;

      PSourceDestroy(oldsource); 
      oldsource = 0;
    }  


    if(recover){
	PSrecover(Psource, journalFile);
	recover = FALSE;
    }
    if(read_only) 
	PSourceSetEditable(Psource, FALSE);
   return Psource;
}

void journalCallback(w, tag, reason)
  Widget   w;
  caddr_t  tag;
  int       *reason;
{
    switch(*reason){
        case XmCR_OK:
	    journalFile = fopen(jnlName, "r+");
#ifndef VMS
	    fcntl(fileno(journalFile), F_SETSYN, 0);
#endif /* VMS */
            recover = TRUE;
	    break;

        case XmCR_CANCEL:
#ifdef VMS
	    { 
 	        void unlink(); 
	    	unlink(jnlName); 
	    }
#endif /* VMS */
            journalFile = fopen(jnlName, "w+"); 
#ifndef VMS
	    if(journalFile) fcntl(fileno(journalFile), F_SETSYN, 0);
#endif /* VMS */
    }
    connectSources(tag); 
}

#ifdef VMS

void unlink(file)
  char *file;
{
struct dsc$descriptor_s fileDesc;
    fclose(journalFile);
    fileDesc.dsc$w_length = strlen(file);
    fileDesc.dsc$a_pointer = file;
    fileDesc.dsc$b_class = DSC$K_CLASS_S;
    fileDesc.dsc$b_dtype = DSC$K_DTYPE_T;
    lib$delete_file(&fileDesc) ; 
}

char *rindex(str, c)
  char *str, c;
{
  char *r;
    r = NULL;
    do {
        if (*str == c) r = str;
    } while (*str++);
    return(r);
}

#endif /* VMS */

char *mkJournalName(fullname)
  char *fullname;
{
    char *parseFilename();
    char *filename, dirname[256];
    char tempname[12];
    if(jnlName)
	XtFree(jnlName);
    jnlName = XtMalloc(1024);
    if(!strlen(fullname)){
	sprintf(tempname, "%x", getpid());
	fullname = tempname;
    }
    filename = parseFilename(fullname, dirname);
    if(filename)
	    sprintf(jnlName, "%s%s%s%s", 
		dirname, journalNamePrefix, filename, journalNameSuffix);
    else
	jnlName = NULL;
    return jnlName;
}

static FILE *connectJournal(filename)
  char *filename;
{
  XtCallbackRec cb[2];
  Arg a[8];
  int n = 0;
  struct stat stats;
    if(jnlName && strlen(jnlName)){
	unlink(jnlName);
    }
    jnlName = mkJournalName(filename);

/*     if(access(jnlName, F_OK) == 0) {
 */
    if(jnlName && (stat(jnlName, &stats) == 0) && stats.st_size){
	cb[0].callback = (XtCallbackProc)journalCallback;
	cb[0].closure = filename;
	cb[1].callback = NULL;
	if(Stuff._RecoverWarnBox)
    	    XtDestroyWidget(Stuff._RecoverWarnBox);
	XtSetArg(a[n], XmNcancelCallback, cb);		    n++;
	XtSetArg(a[n], XmNokCallback, cb);		    n++;
        MrmFetchWidgetOverride( DRM_hierarchy, "recoverWarn",
		 mainWin, NULL, a,  n, &Stuff._RecoverWarnBox, &dummy_class);	
        XtManageChild(Stuff._RecoverWarnBox);
    } else {
	if(jnlName) {
            journalFile = fopen(jnlName, "w+"); 
	} else {
	    journalFile = NULL;
        }
#ifndef VMS
 	if(journalFile) fcntl(fileno(journalFile), F_SETSYN, 0);
#endif /* VMS */
	connectSources(filename);
    }
    return journalFile;
}

XmTextSource setSources(filename)
  char *filename;
{
    if(!read_only)
	connectJournal(filename);
    else 
	connectSources(filename);
}

deleteJournal()
{
    if(journalFile && jnlName){
        fclose(journalFile);
        unlink(jnlName);
        jnlName = NULL;
    }
}

#ifdef unix

int lockFile(filename)
  char *filename;
{
  int fd;
    if (overrideLocking)
	return TRUE;
    fd = open(filename, O_WRONLY, 0);
    if(flock(fd,  LOCK_EX | LOCK_NB) == 0)
	return fd;
    else {
	close(fd);
	return 0;
    }
}

unlockFile()
{
    if(lock){
	if(overrideLocking)
	    lock = 0;
	else {
            flock(lock,  LOCK_UN);
	    close(lock);
	    lock = 0;
	}
    }
}

#endif /* unix */

#ifdef VMS

int lockFile(filename)
  char *filename;
{
  int key, status;
  if (overrideLocking)
    return TRUE;
  else {
    a_fab = cc$rms_fab;
    a_nam = cc$rms_nam;
    a_fab.fab$l_fop =  FAB$M_NAM;
    a_fab.fab$l_fna = filename;
    a_fab.fab$b_fns =strlen(filename);
    a_fab.fab$l_nam = &a_nam;
    a_nam.nam$b_ess = 255;
    a_nam.nam$l_esa = exp_str;
    status = sys$open(&a_fab /*, error_routine, success_routine*/);
    a_fib.id = a_nam.nam$w_fid[0];
    a_fib.seq = a_nam.nam$w_fid[1];
    a_fib.vol = a_nam.nam$w_fid[2];
    sprintf(lockname, "%06d%06d%06d", a_fib.id, a_fib.seq, a_fib.vol);
    foobarDesc.dsc$w_length = strlen(lockname);
    foobarDesc.dsc$a_pointer = lockname;
    foobarDesc.dsc$b_class = DSC$K_CLASS_S;
    foobarDesc.dsc$b_dtype = DSC$K_DTYPE_T;
    status = sys$enq(0, LCK$K_PWMODE, &mylsb, LCK$M_NOQUEUE | LCK$M_SYNCSTS, 
			&foobarDesc, 0, 0, 0, 0, PSL$C_USER, 0);
    if(status & 1) 
	key = (int)lockname;
    else
	key = 0;    
    sys$close(&a_fab /*, error_routine, success_routine */) ;
    return key;
  }
}

unlockFile()
{
    if(lock){
	if(overrideLocking)
	    lock = 0;
	else {
	    sys$deq(0,0,0, 0|LCK$M_DEQALL);
	    lock = 0;
	}
    }
}
#endif /* VMS */

closeInputFile()
{
    deleteJournal();
    unlockFile();
}

void setSourceEditable(Source)
  XmTextSource Source;
{
   PSourceSetEditable(Source, 1);
}

#ifdef unix

char *parseFilename(fullname, dirstring)
  char *fullname;
  char *dirstring;  /* resultant directory name written here */
{
  static char *filename;
  int dirlen;
    filename =  rindex(fullname, '/');
    if(!filename){
	*dirstring = 0;
	return fullname;
    } else {
	dirlen = strlen(fullname) - strlen(filename);
	memcpy(dirstring, fullname, dirlen + 1);
	dirstring[dirlen+1] = 0;
	return(filename+1);
    }
}
#endif /* unix */

#ifdef VMS

error_routine(/* ? */)
{    
/*    ShowMessage(&Stuff._badFilename, "badFilename"); */

}

success_routine()
{
}

char *parseFilename(fullname, dirname)
  char *fullname;
  char *dirname;  /* resultant directory name written here */
{
  struct FAB          fabbb; 
  struct NAM          nammm; 
  static char filename[128];
  char exp_string[NAM$C_MAXRSS];
  int status, namelen, dirlen;
    fabbb = cc$rms_fab;
    nammm = cc$rms_nam;
    fabbb.fab$l_fop = FAB$M_NAM;
    fabbb.fab$l_fna = fullname;
    fabbb.fab$b_fns = strlen(fullname);
    fabbb.fab$l_nam = &nammm;
    nammm.nam$b_ess = NAM$C_MAXRSS;
    nammm.nam$l_esa = exp_string;
    status = sys$parse(&fabbb, error_routine, success_routine);
    if(status & 1){
        namelen = nammm.nam$b_name + nammm.nam$b_type;
        dirlen = nammm.nam$b_node + nammm.nam$b_dev + nammm.nam$b_dir;
        memcpy(dirname, nammm.nam$l_node, dirlen);
        dirname[dirlen] = 0;
	memcpy(filename, nammm.nam$l_name, namelen);
	filename[namelen] = 0;
	return filename;
    } else {
	return 0; /* error */
    }
}

#endif /* VMS */

char *decodeCS(string)
XmString string;
{
    long byte_count, status;
    char *text;

    text = (Opaque)DXmCvtCStoFC(string, &byte_count, &status);
    return (text);
}

Cursor  workInProgress;

static Cursor MakeCursor( name)
    int name;
{
    Cursor result;
Font cfont;
static XColor fore = {0, 65535, 0, 0}; /* red */
static XColor back = {0, 65535, 65535, 65535}; /* white */

    cfont = XLoadFont (CurDpy, "decw$cursor");
    if (!cfont) return ((Cursor) 0);
    result = XCreateGlyphCursor (CurDpy,
                cfont, cfont, name, name+1, &fore, &back);
    XUnloadFont (CurDpy, cfont);
    return result;
}

static MakeWaitCursor()
{
    workInProgress = MakeCursor (decw$c_wait_cursor);
    if (workInProgress == 0)
	workInProgress= XCreateFontCursor (CurDpy, XC_watch);
}

void BeginLongOperation()
{
    if(!workInProgress)
	MakeWaitCursor();
    XDefineCursor(CurDpy, XtWindow(toplevel), workInProgress);
    XFlush (CurDpy);
}

void  EndLongOperation()
{
    XUndefineCursor(XtDisplay(mainWin), XtWindow(toplevel));
    XFlush (CurDpy);
}

#ifndef VMS
void Xfree(ptr)
char *ptr;
{
    free(ptr);
}
#include <time.h>
#include <sys/timeb.h>
void ftime(timeptr)
struct timeb *timeptr;
{
    time_t clockTime;
    struct tm *TM;

    (void) time(&clockTime);
    TM = gmtime(&clockTime);
    timeptr->time = clockTime;
    timeptr->millitm = 0;
/*    timeptr->timezone = TM->tm_gmtoff/60;*/
    timeptr->dstflag = TM->tm_isdst;
}
#endif /* VMS */

void AddProtocols (widget, delete, save)
Widget	widget;
XtCallbackProc	delete;
XtCallbackProc	save;
{
    static Atom	DwcWmDeleteWindowAtom = None;
    static Atom	DwcWmSaveYourselfAtom = None;

    /*
    ** Try to get the atoms necessary to do this job.
    */
    if (DwcWmDeleteWindowAtom == None)
    {
	DwcWmDeleteWindowAtom = XmInternAtom
	    (XtDisplay(widget), "WM_DELETE_WINDOW", False);
    }
    if (DwcWmSaveYourselfAtom == None)
    {
	DwcWmSaveYourselfAtom = XmInternAtom
	    (XtDisplay(widget), "WM_SAVE_YOURSELF", False);
    }

    /*
    ** Add the callbacks that do what's needed.
    */
    if (delete != NULL)
    {
	XmAddWMProtocolCallback
	    (widget, DwcWmDeleteWindowAtom, (XtCallbackProc)delete, NULL);
	XmActivateWMProtocol (widget, DwcWmDeleteWindowAtom);
    }

    if (save != NULL)
    {
	XmAddWMProtocolCallback
	    (widget, DwcWmSaveYourselfAtom, (XtCallbackProc)save, NULL);
	XmActivateWMProtocol (widget, DwcWmSaveYourselfAtom);
    }
}
